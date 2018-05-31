/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 * Copyright (C) 2005 SGI, Christoph Lameter
 * Copyright (C) 2006 Nick Piggin
 * Copyright (C) 2012 Konstantin Khlebnikov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "radix-tree.h"


#define RADIX_TREE_MAP_SHIFT	8	/* For more stressful testing */

#define RADIX_TREE_MAP_SIZE	(1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK	(RADIX_TREE_MAP_SIZE-1)

#define RADIX_TREE_TAG_LONGS	\
	((RADIX_TREE_MAP_SIZE + BITS_PER_LONG - 1) / BITS_PER_LONG)

struct radix_tree_node {
	unsigned int	height;		/* Height from the bottom */
	unsigned int	count;
	struct radix_tree_node *parent;	/* Used when ascending tree */
	void 	*slots[RADIX_TREE_MAP_SIZE];
//	unsigned long	tags[RADIX_TREE_MAX_TAGS][RADIX_TREE_TAG_LONGS];
};

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define ENOMEM 12
#define EEXIST 17
#define rcu_dereference_raw(p) p
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define RADIX_TREE_INDEX_BITS  (8 /* CHAR_BIT */ * sizeof(unsigned long))
#define RADIX_TREE_MAX_PATH (DIV_ROUND_UP(RADIX_TREE_INDEX_BITS, \
					  RADIX_TREE_MAP_SHIFT))

/*
 * The height_to_maxindex array needs to be one deeper than the maximum
 * path as height 0 holds only 1 entry.
 */
static unsigned long height_to_maxindex[RADIX_TREE_MAX_PATH + 1];

/*
 * The radix tree is variable-height, so an insert operation not only has
 * to build the branch to its corresponding item, it also has to build the
 * branch to existing items if the size has to be increased (by
 * radix_tree_extend).
 *
 * The worst case is a zero height tree with just a single item at index 0,
 * and then inserting an item at index ULONG_MAX. This requires 2 new branches
 * of RADIX_TREE_MAX_PATH size to be created, with only the root node shared.
 * Hence:
 */
#define RADIX_TREE_PRELOAD_SIZE (RADIX_TREE_MAX_PATH * 2 - 1)

/*
 * Per-cpu pool of preloaded nodes
 */
struct radix_tree_preload {
	int nr;
	struct radix_tree_node *nodes[RADIX_TREE_PRELOAD_SIZE];
};

static inline void *ptr_to_indirect(void *ptr)
{
	return (void *)((unsigned long)ptr | RADIX_TREE_INDIRECT_PTR);
}

static inline void *indirect_to_ptr(void *ptr)
{
	return (void *)((unsigned long)ptr & ~RADIX_TREE_INDIRECT_PTR);
}

/*
 * This assumes that the caller has performed appropriate preallocation, and
 * that the caller has pinned this thread of control to the current CPU.
 */
static struct radix_tree_node *
radix_tree_node_alloc(struct radix_tree_root *root)
{
	struct radix_tree_node *ret = NULL;
	ret = (struct radix_tree_node *)malloc(sizeof(struct radix_tree_node));
	return ret;
}

static inline void
radix_tree_node_free(struct radix_tree_node *node)
{
	free(node);
}

/*
 *	Return the maximum key which can be store into a
 *	radix tree with height HEIGHT.
 */
static inline unsigned long radix_tree_maxindex(unsigned int height)
{
	return height_to_maxindex[height];
}

/*
 *	Extend a radix tree so it can store key @index.
 */
static int radix_tree_extend(struct radix_tree_root *root, unsigned long index)
{
	struct radix_tree_node *node;
	struct radix_tree_node *slot;
	unsigned int height;

	/* Figure out what the height should be.  */
	height = root->height + 1;
	while (index > radix_tree_maxindex(height))
		height++;

	if (root->rnode == NULL) {
		root->height = height;
		goto out;
	}

	do {
		unsigned int newheight;
		if (!(node = radix_tree_node_alloc(root)))
			return -ENOMEM;

		/* Increase the height.  */
		newheight = root->height+1;
		node->height = newheight;
		node->count = 1;
		node->parent = NULL;
		slot = root->rnode;
		if (newheight > 1) {
			slot = (struct radix_tree_node *)indirect_to_ptr(slot);
			slot->parent = node;
		}
		node->slots[0] = slot;
		node = (struct radix_tree_node *)ptr_to_indirect(node);
		rcu_assign_pointer(root->rnode, node);
		root->height = newheight;
	} while (height > root->height);
out:
	return 0;
}

/**
 *	radix_tree_insert    -    insert into a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *	@item:		item to insert
 *
 *	Insert an item into the radix tree at position @index.
 */
int radix_tree_insert(struct radix_tree_root *root,
			unsigned long index, void *item)
{
	struct radix_tree_node *node = NULL, *slot;
	unsigned int height, shift;
	int offset;
	int error;

	BUG_ON(radix_tree_is_indirect_ptr(item));

	/* Make sure the tree is high enough.  */
	if (index > radix_tree_maxindex(root->height)) {
		error = radix_tree_extend(root, index);
		if (error)
			return error;
	}

	slot = (struct radix_tree_node *)indirect_to_ptr(root->rnode);

	height = root->height;
	shift = (height-1) * RADIX_TREE_MAP_SHIFT;

	offset = 0;			/* uninitialised var warning */
	while (height > 0) {
		if (slot == NULL) {
			/* Have to add a child node.  */
			if (!(slot = radix_tree_node_alloc(root)))
				return -ENOMEM;
			slot->height = height;
			slot->parent = node;
			if (node) {
				rcu_assign_pointer(node->slots[offset], slot);
				node->count++;
			} else
				rcu_assign_pointer(root->rnode, (struct radix_tree_node *)ptr_to_indirect(slot));
		}

		/* Go a level down */
		offset = (index >> shift) & RADIX_TREE_MAP_MASK;
		node = slot;
		slot = (struct radix_tree_node *)node->slots[offset];
		shift -= RADIX_TREE_MAP_SHIFT;
		height--;
	}

	if (slot != NULL)
		return -EEXIST;

	if (node) {
		node->count++;
		rcu_assign_pointer(node->slots[offset], (struct radix_tree_node *)item);
	} else {
		rcu_assign_pointer(root->rnode, (struct radix_tree_node *)item);
	}

	return 0;
}

/*
 * is_slot == 1 : search for the slot.
 * is_slot == 0 : search for the node.
 */
static void *radix_tree_lookup_element(struct radix_tree_root *root,
				unsigned long index, int is_slot)
{
	unsigned int height, shift;
	struct radix_tree_node *node, **slot;

	node = rcu_dereference_raw(root->rnode);
	if (node == NULL)
		return NULL;

	if (!radix_tree_is_indirect_ptr(node)) {
		if (index > 0)
			return NULL;
		return is_slot ? (void *)&root->rnode : node;
	}
	node = (struct radix_tree_node *)indirect_to_ptr(node);

	height = node->height;
	if (index > radix_tree_maxindex(height))
		return NULL;

	shift = (height-1) * RADIX_TREE_MAP_SHIFT;

	do {
		slot = (struct radix_tree_node **)
			(node->slots + ((index>>shift) & RADIX_TREE_MAP_MASK));
		node = rcu_dereference_raw(*slot);
		if (node == NULL)
			return NULL;

		shift -= RADIX_TREE_MAP_SHIFT;
		height--;
	} while (height > 0);

	return is_slot ? (void *)slot : indirect_to_ptr(node);
}

/**
 *	radix_tree_lookup_slot    -    lookup a slot in a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Returns:  the slot corresponding to the position @index in the
 *	radix tree @root. This is useful for update-if-exists operations.
 *
 *	This function can be called under rcu_read_lock iff the slot is not
 *	modified by radix_tree_replace_slot, otherwise it must be called
 *	exclusive from other writers. Any dereference of the slot must be done
 *	using radix_tree_deref_slot.
 */
void **radix_tree_lookup_slot(struct radix_tree_root *root, unsigned long index)
{
	return (void **)radix_tree_lookup_element(root, index, 1);
}

/**
 *	radix_tree_lookup    -    perform lookup operation on a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Lookup the item at the position @index in the radix tree @root.
 *
 *	This function can be called under rcu_read_lock, however the caller
 *	must manage lifetimes of leaf nodes (eg. RCU may also be used to free
 *	them safely). No RCU barriers are required to access or modify the
 *	returned item, however.
 */
void *radix_tree_lookup(struct radix_tree_root *root, unsigned long index)
{
	return radix_tree_lookup_element(root, index, 0);
}

/**
 * radix_tree_next_chunk - find next chunk of slots for iteration
 *
 * @root:	radix tree root
 * @iter:	iterator state
 * @flags:	RADIX_TREE_ITER_* flags and tag index
 * Returns:	pointer to chunk first slot, or NULL if iteration is over
 */

/**
 *	radix_tree_next_hole    -    find the next hole (not-present entry)
 *	@root:		tree root
 *	@index:		index key
 *	@max_scan:	maximum range to search
 *
 *	Search the set [index, min(index+max_scan-1, MAX_INDEX)] for the lowest
 *	indexed hole.
 *
 *	Returns: the index of the hole if found, otherwise returns an index
 *	outside of the set specified (in which case 'return - index >= max_scan'
 *	will be true). In rare cases of index wrap-around, 0 will be returned.
 *
 *	radix_tree_next_hole may be called under rcu_read_lock. However, like
 *	radix_tree_gang_lookup, this will not atomically search a snapshot of
 *	the tree at a single point in time. For example, if a hole is created
 *	at index 5, then subsequently a hole is created at index 10,
 *	radix_tree_next_hole covering both indexes may return 10 if called
 *	under rcu_read_lock.
 */
unsigned long radix_tree_next_hole(struct radix_tree_root *root,
				unsigned long index, unsigned long max_scan)
{
	unsigned long i;

	for (i = 0; i < max_scan; i++) {
		if (!radix_tree_lookup(root, index))
			break;
		index++;
		if (index == 0)
			break;
	}

	return index;
}

/**
 *	radix_tree_prev_hole    -    find the prev hole (not-present entry)
 *	@root:		tree root
 *	@index:		index key
 *	@max_scan:	maximum range to search
 *
 *	Search backwards in the range [max(index-max_scan+1, 0), index]
 *	for the first hole.
 *
 *	Returns: the index of the hole if found, otherwise returns an index
 *	outside of the set specified (in which case 'index - return >= max_scan'
 *	will be true). In rare cases of wrap-around, ULONG_MAX will be returned.
 *
 *	radix_tree_next_hole may be called under rcu_read_lock. However, like
 *	radix_tree_gang_lookup, this will not atomically search a snapshot of
 *	the tree at a single point in time. For example, if a hole is created
 *	at index 10, then subsequently a hole is created at index 5,
 *	radix_tree_prev_hole covering both indexes may return 5 if called under
 *	rcu_read_lock.
 */
unsigned long radix_tree_prev_hole(struct radix_tree_root *root,
				   unsigned long index, unsigned long max_scan)
{
	unsigned long i;

	for (i = 0; i < max_scan; i++) {
		if (!radix_tree_lookup(root, index))
			break;
		index--;
		if (index == ULONG_MAX)
			break;
	}

	return index;
}

/**
 *	radix_tree_shrink    -    shrink height of a radix tree to minimal
 *	@root		radix tree root
 */
static inline void radix_tree_shrink(struct radix_tree_root *root)
{
	/* try to shrink tree height */
	while (root->height > 0) {
		struct radix_tree_node *to_free = root->rnode;
		struct radix_tree_node *slot;

		BUG_ON(!radix_tree_is_indirect_ptr(to_free));
		to_free = (struct radix_tree_node *)indirect_to_ptr(to_free);

		/*
		 * The candidate node has more than one child, or its child
		 * is not at the leftmost slot, we cannot shrink.
		 */
		if (to_free->count != 1)
			break;
		if (!to_free->slots[0])
			break;

		/*
		 * We don't need rcu_assign_pointer(), since we are simply
		 * moving the node from one part of the tree to another: if it
		 * was safe to dereference the old pointer to it
		 * (to_free->slots[0]), it will be safe to dereference the new
		 * one (root->rnode) as far as dependent read barriers go.
		 */
		slot = (struct radix_tree_node *)to_free->slots[0];
		if (root->height > 1) {
			slot->parent = NULL;
			slot = (struct radix_tree_node *)ptr_to_indirect(slot);
		}
		root->rnode = slot;
		root->height--;

		/*
		 * We have a dilemma here. The node's slot[0] must not be
		 * NULLed in case there are concurrent lookups expecting to
		 * find the item. However if this was a bottom-level node,
		 * then it may be subject to the slot pointer being visible
		 * to callers dereferencing it. If item corresponding to
		 * slot[0] is subsequently deleted, these callers would expect
		 * their slot to become empty sooner or later.
		 *
		 * For example, lockless pagecache will look up a slot, deref
		 * the page pointer, and if the page is 0 refcount it means it
		 * was concurrently deleted from pagecache so try the deref
		 * again. Fortunately there is already a requirement for logic
		 * to retry the entire slot lookup -- the indirect pointer
		 * problem (replacing direct root node with an indirect pointer
		 * also results in a stale slot). So tag the slot as indirect
		 * to force callers to retry.
		 */
		if (root->height == 0)
			*((unsigned long *)&to_free->slots[0]) |=
						RADIX_TREE_INDIRECT_PTR;

		radix_tree_node_free(to_free);
	}
}

/**
 *	radix_tree_delete    -    delete an item from a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Remove the item at @index from the radix tree rooted at @root.
 *
 *	Returns the address of the deleted item, or NULL if it was not present.
 */
void *radix_tree_delete(struct radix_tree_root *root, unsigned long index)
{
	struct radix_tree_node *node = NULL;
	struct radix_tree_node *slot = NULL;
	struct radix_tree_node *to_free;
	unsigned int height, shift;
	int offset;

	height = root->height;
	if (index > radix_tree_maxindex(height))
		goto out;

	slot = root->rnode;
	if (height == 0) {
		root->rnode = NULL;
		goto out;
	}
	slot = (struct radix_tree_node *)indirect_to_ptr(slot);
	shift = height * RADIX_TREE_MAP_SHIFT;

	do {
		if (slot == NULL)
			goto out;

		shift -= RADIX_TREE_MAP_SHIFT;
		offset = (index >> shift) & RADIX_TREE_MAP_MASK;
		node = slot;
		slot = (struct radix_tree_node *)slot->slots[offset];
	} while (shift);

	if (slot == NULL)
		goto out;

	/*
	 * Clear all tags associated with the item to be deleted.
	 * This way of doing it would be inefficient, but seldom is any set.
	 */

	to_free = NULL;
	/* Now free the nodes we do not need anymore */
	while (node) {
		node->slots[offset] = NULL;
		node->count--;
		/*
		 * Queue the node for deferred freeing after the
		 * last reference to it disappears (set NULL, above).
		 */
		if (to_free)
			radix_tree_node_free(to_free);

		if (node->count) {
			if (node == indirect_to_ptr(root->rnode))
				radix_tree_shrink(root);
			goto out;
		}

		/* Node with zero slots in use so free it */
		to_free = node;

		index >>= RADIX_TREE_MAP_SHIFT;
		offset = index & RADIX_TREE_MAP_MASK;
		node = node->parent;
	}

	root->height = 0;
	root->rnode = NULL;
	if (to_free)
		radix_tree_node_free(to_free);

out:
	return slot;
}

static unsigned long __maxindex(unsigned int height)
{
	unsigned int width = height * RADIX_TREE_MAP_SHIFT;
	int shift = RADIX_TREE_INDEX_BITS - width;

	if (shift < 0)
		return ~0UL;
	if (shift >= BITS_PER_LONG)
		return 0UL;
	return ~0UL >> shift;
}

static void radix_tree_init_maxindex(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(height_to_maxindex); i++)
		height_to_maxindex[i] = __maxindex(i);
}

void radix_tree_init(void)
{
	radix_tree_init_maxindex();
}
