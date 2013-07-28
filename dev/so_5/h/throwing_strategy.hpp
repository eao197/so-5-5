/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Флаги, которые определяют, как реагировать на ошибки.
*/

#if !defined( _SO_5__THROWING_STRATEGY_HPP_ )
#define _SO_5__THROWING_STRATEGY_HPP_

namespace so_5
{

//
// throwing_strategy_t
//

//! Флаги, которые указывают, что делать в случае ошибок
//! в работе некоторых методов классов SObjrctizer.
enum throwing_strategy_t
{
	//! Бросать исключение в случае возникновения ошибки.
	THROW_ON_ERROR,
	//! При возникновении ошибки, сообщать о ней через код возврата.
	DO_NOT_THROW_ON_ERROR
};

} /* namespace so_5  */

#endif
