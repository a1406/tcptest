#ifndef GAME_EVENT_H__
#define GAME_EVENT_H__
#include <errno.h>
#include <stdlib.h>
#include "listen_node.h"

extern "C" {
#include "log4c.h"
}

#define UNUSED(x) (void)(x)

#ifdef LOG_FORMAT_CHECK  /*only for format check*/
#define LOG_ERR   printf
#define LOG_INFO   printf
#define LOG_DEBUG   printf
#else
#ifdef LOG_ERR
#undef LOG_ERR
#endif // LOG_ERR

#define LOG_ERR(fmt, arg...) 			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, fmt, ##arg);

#ifdef LOG_INFO
#undef LOG_INFO
#endif // LOG_INFO

#define LOG_INFO(fmt, arg...) 			log4c_category_log(mycat, LOG4C_PRIORITY_INFO, fmt, ##arg);

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif // LOG_DEBUG
#define LOG_DEBUG(fmt, arg...) 			log4c_category_log(mycat, LOG4C_PRIORITY_DEBUG, fmt, ##arg);
//这样屏蔽掉可以提高性能
//#define LOG_DEBUG(fmt, arg...) 			
#endif

extern log4c_category_t* mycat;

int game_event_init();

typedef conn_node_base * (*get_conn_node)(int fd);
typedef void (*del_conn_node)(conn_node_base *);
int game_add_listen_event(int port, get_conn_node cb1, del_conn_node cb2, const char *name);
int game_add_connect_event(struct sockaddr *sa, int socklen, conn_node_base *client);

void remove_listen_callback_event(int listen_fd, conn_node_base *client);
int create_new_socket(int set_opt);
int add_timer(struct timeval t, void *arg);

void init_mycat();
void change_mycat();

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif
