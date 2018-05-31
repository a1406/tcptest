#ifndef stl_relation_h_
#define stl_relation_h_

#include <utility>

/*
描述: 扩展std::pair
日期: 2014-09-02
作者: andrewliao
*/


namespace std
{
	template <typename _FIRST, typename _SECOND, typename _THREE>
	struct relation_three : public std::pair<_FIRST, _SECOND>
	{
		typedef std::pair<_FIRST, _SECOND> _BaseType;
		typedef typename _BaseType::first_type first_type;
		typedef typename _BaseType::second_type second_type;
		typedef _THREE three_type;
		typedef relation_three<_FIRST, _SECOND, _THREE>	_Myt;

		using _BaseType::first;
		using _BaseType::second;

		relation_three()
			: _BaseType()
			, three(){}

		relation_three(const first_type& _Val1, const second_type& _Val2, const three_type& _Val3 )
			: _BaseType(_Val1, _Val2)
			, three(_Val3) {}

		_Myt& operator=(const _Myt& _Right)
		{	
			first = _Right.first;
			second = _Right.second;
			three = _Right.three;

			return (*this);
		}

		void swap(_Myt& _Right)
		{
			if (this != &_Right)
			{
				_Swap_adl(first, _Right.first);
				_Swap_adl(second, _Right.second);
				_Swap_adl(three, _Right.three);
			}
		}

		three_type three;
	};

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR>
	struct relation_four : public std::relation_three<_FIRST, _SECOND, _THREE>
	{
		typedef std::relation_three<_FIRST, _SECOND, _THREE> _BaseType;
		typedef typename _BaseType::first_type first_type;
		typedef typename _BaseType::second_type second_type;
		typedef typename _BaseType::three_type three_type;

		typedef _FOUR four_type;
		typedef relation_four<_FIRST, _SECOND, _THREE, _FOUR>	_Myt;

		using _BaseType::first;
		using _BaseType::second;
		using _BaseType::three;

		relation_four()
			: _BaseType()
			, four(){}

		relation_four(const first_type& _Val1, const second_type& _Val2, const three_type& _Val3, const four_type& _Val4)
			: _BaseType(_Val1, _Val2, _Val3)
			, four(_Val4) {}

		_Myt& operator=(const _Myt& _Right)
		{	
			first = _Right.first;
			second = _Right.second;
			three = _Right.three;
			four = _Right.four;

			return (*this);
		}

		void swap(_Myt& _Right)
		{
			if (this != &_Right)
			{
				_Swap_adl(first, _Right.first);
				_Swap_adl(second, _Right.second);
				_Swap_adl(three, _Right.three);
				_Swap_adl(four, _Right.four);
			}
		}

		four_type four;
	};

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE>
	struct relation_five : public std::relation_four<_FIRST, _SECOND, _THREE, _FOUR>
	{
		typedef std::relation_four<_FIRST, _SECOND, _THREE, _FOUR> _BaseType;
		typedef typename _BaseType::first_type first_type;
		typedef typename _BaseType::second_type second_type;
		typedef typename _BaseType::three_type three_type;
		typedef typename _BaseType::four_type four_type;
		typedef _FIVE five_type;
		typedef relation_five<_FIRST, _SECOND, _THREE, _FOUR, _FIVE>	_Myt;

		using _BaseType::first;
		using _BaseType::second;
		using _BaseType::three;
		using _BaseType::four;

		relation_five()
			: _BaseType()
			, five(){}

		relation_five(const first_type& _Val1, const second_type& _Val2, const three_type& _Val3, const four_type& _Val4, const five_type& _Val5)
			: _BaseType(_Val1, _Val2, _Val3, _Val4)
			, five(_Val5) {}

		_Myt& operator=(const _Myt& _Right)
		{	
			first = _Right.first;
			second = _Right.second;
			three = _Right.three;
			four = _Right.four;
			five = _Right.five;


			return (*this);
		}

		void swap(_Myt& _Right)
		{
			if (this != &_Right)
			{
				_Swap_adl(first, _Right.first);
				_Swap_adl(second, _Right.second);
				_Swap_adl(three, _Right.three);
				_Swap_adl(four, _Right.four);
				_Swap_adl(five, _Right.five);
			}
		}

		five_type five;
	};

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE, typename _SIX>
	struct relation_six : public std::relation_five<_FIRST, _SECOND, _THREE, _FOUR, _FIVE>
	{
		typedef std::relation_five<_FIRST, _SECOND, _THREE, _FOUR, _FIVE> _BaseType;
		typedef typename _BaseType::first_type first_type;
		typedef typename _BaseType::second_type second_type;
		typedef typename _BaseType::three_type three_type;
		typedef typename _BaseType::four_type four_type;
		typedef typename _BaseType::five_type five_type;
		typedef _SIX six_type;
		typedef relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX>	_Myt;

		using _BaseType::first;
		using _BaseType::second;
		using _BaseType::three;
		using _BaseType::four;
		using _BaseType::five;

		relation_six()
			: _BaseType()
			, six(){}

		relation_six(const first_type& _Val1, const second_type& _Val2, const three_type& _Val3, const four_type& _Val4, const five_type& _Val5, const six_type& _Val6)
			: _BaseType(_Val1, _Val2, _Val3, _Val4, _Val5)
			, six(_Val6) {}

		_Myt& operator=(const _Myt& _Right)
		{	
			first = _Right.first;
			second = _Right.second;
			three = _Right.three;
			four = _Right.four;
			five = _Right.five;
			six = _Right.six;

			return (*this);
		}

		void swap(_Myt& _Right)
		{
			if (this != &_Right)
			{
				_Swap_adl(first, _Right.first);
				_Swap_adl(second, _Right.second);
				_Swap_adl(three, _Right.three);
				_Swap_adl(four, _Right.four);
				_Swap_adl(five, _Right.five);
				_Swap_adl(six, _Right.six);
			}
		}

		six_type six;
	};


	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE, typename _SIX, typename _SEVEN>
	struct relation_seven : public std::relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX>
	{
		typedef std::relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX> _BaseType;
		typedef typename _BaseType::first_type first_type;
		typedef typename _BaseType::second_type second_type;
		typedef typename _BaseType::three_type three_type;
		typedef typename _BaseType::four_type four_type;
		typedef typename _BaseType::five_type five_type;
		typedef typename _BaseType::six_type six_type;
		typedef _SEVEN seven_type;
		typedef relation_seven<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX, _SEVEN>	_Myt;

		using _BaseType::first;
		using _BaseType::second;
		using _BaseType::three;
		using _BaseType::four;
		using _BaseType::five;
		using _BaseType::six;

		relation_seven()
			: _BaseType()
			, seven(){}

		relation_seven(const first_type& _Val1, const second_type& _Val2, const three_type& _Val3, const four_type& _Val4, const five_type& _Val5, const six_type& _Val6, const seven_type& _Val7)
			: _BaseType(_Val1, _Val2, _Val3, _Val4, _Val5, _Val6)
			, seven(_Val7) {}

		_Myt& operator=(const _Myt& _Right)
		{	
			first = _Right.first;
			second = _Right.second;
			three = _Right.three;
			four = _Right.four;
			five = _Right.five;
			six = _Right.six;
			seven = _Right.seven;

			return (*this);
		}

		void swap(_Myt& _Right)
		{
			if (this != &_Right)
			{
				_Swap_adl(first, _Right.first);
				_Swap_adl(second, _Right.second);
				_Swap_adl(three, _Right.three);
				_Swap_adl(four, _Right.four);
				_Swap_adl(five, _Right.five);
				_Swap_adl(six, _Right.six);
				_Swap_adl(seven, _Right.seven);
			}
		}

		seven_type seven;
	};
	
	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE, typename _SIX, typename _SEVEN, typename _EIGHT>
	struct relation_eight: public std::relation_seven<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX, _SEVEN>
	{
		typedef std::relation_seven<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX, _SEVEN> _BaseType;
		typedef typename _BaseType::first_type first_type;
		typedef typename _BaseType::second_type second_type;
		typedef typename _BaseType::three_type three_type;
		typedef typename _BaseType::four_type four_type;
		typedef typename _BaseType::five_type five_type;
		typedef typename _BaseType::six_type six_type;
		typedef typename _BaseType::seven_type seven_type;

		typedef _EIGHT eight_type;
		typedef relation_eight<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX, _SEVEN, _EIGHT>	_Myt;

		using _BaseType::first;
		using _BaseType::second;
		using _BaseType::three;
		using _BaseType::four;
		using _BaseType::five;
		using _BaseType::six;
		using _BaseType::seven;

		relation_eight()
			: _BaseType()
			, eight(){}

		relation_eight(const first_type& _Val1, const second_type& _Val2, const three_type& _Val3, const four_type& _Val4
			, const five_type& _Val5, const six_type& _Val6, const seven_type& _Val7, const seven_type& _Val8)
			: _BaseType(_Val1, _Val2, _Val3, _Val4, _Val5, _Val6, _Val7)
			, eight(_Val8) {}

		_Myt& operator=(const _Myt& _Right)
		{	
			first = _Right.first;
			second = _Right.second;
			three = _Right.three;
			four = _Right.four;
			five = _Right.five;
			six = _Right.six;
			seven = _Right.seven;
			eight = _Right.eight;

			return (*this);
		}

		void swap(_Myt& _Right)
		{
			if (this != &_Right)
			{
				_Swap_adl(first, _Right.first);
				_Swap_adl(second, _Right.second);
				_Swap_adl(three, _Right.three);
				_Swap_adl(four, _Right.four);
				_Swap_adl(five, _Right.five);
				_Swap_adl(six, _Right.six);
				_Swap_adl(seven, _Right.seven);
				_Swap_adl(eight, _Right.eight);
			}
		}

		eight_type eight;
	};

	template <typename _FIRST, typename _SECOND, typename _THREE>
	relation_three<_FIRST, _SECOND, _THREE> make_relation(const _FIRST& _Val1, const _SECOND& _Val2, const _THREE& _Val3) {
		return relation_three<_FIRST, _SECOND, _THREE>(_Val1, _Val2, _Val3);
	}

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR>
	relation_four<_FIRST, _SECOND, _THREE, _FOUR> make_relation(const _FIRST& _Val1, const _SECOND& _Val2, const _THREE& _Val3, const _FOUR& _Val4) {
		return relation_four<_FIRST, _SECOND, _THREE, _FOUR>(_Val1, _Val2, _Val3, _Val4);
	}

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE>
	relation_five<_FIRST, _SECOND, _THREE, _FOUR, _FIVE> make_relation(const _FIRST& _Val1, const _SECOND& _Val2, const _THREE& _Val3, const _FOUR& _Val4, const _FIVE& _Val5) {
		return relation_five<_FIRST, _SECOND, _THREE, _FOUR, _FIVE>(_Val1, _Val2, _Val3, _Val4, _Val5);
	}

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE, typename _SIX>
	relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX> make_relation(const _FIRST& _Val1, const _SECOND& _Val2, const _THREE& _Val3, const _FOUR& _Val4, const _FIVE& _Val5, const _SIX& _Val6) {
		return relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX>(_Val1, _Val2, _Val3, _Val4, _Val5, _Val6);
	}

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE, typename _SIX, typename _SEVEN>
	relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX> make_relation(const _FIRST& _Val1, const _SECOND& _Val2, const _THREE& _Val3
		, const _FOUR& _Val4, const _FIVE& _Val5, const _SIX& _Val6, const _SEVEN& _Val7) {
		return relation_seven<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX, _SEVEN>(_Val1, _Val2, _Val3, _Val4, _Val5, _Val6, _Val7);
	}

	template <typename _FIRST, typename _SECOND, typename _THREE, typename _FOUR, typename _FIVE, typename _SIX, typename _SEVEN, typename _EIGHT>
	relation_six<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX> make_relation(const _FIRST& _Val1, const _SECOND& _Val2, const _THREE& _Val3
		, const _FOUR& _Val4, const _FIVE& _Val5, const _SIX& _Val6, const _SEVEN& _Val7, const _EIGHT& _Val8) {
			return relation_eight<_FIRST, _SECOND, _THREE, _FOUR, _FIVE, _SIX, _SEVEN, _EIGHT>(_Val1, _Val2, _Val3, _Val4, _Val5, _Val6, _Val7, _Val8);
	}
}

#endif//stl_relation_h_
