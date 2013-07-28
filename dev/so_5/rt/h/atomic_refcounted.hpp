/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Базовый класс объекта с подсчетом ссылок.
*/

#if !defined( _SO_5__RT__ATOMIC_REFCOUNTED_HPP_ )
#define _SO_5__RT__ATOMIC_REFCOUNTED_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/types.hpp>

namespace so_5
{

namespace rt
{

//! Базовый класс с механизмом подсчета ссылок себя.
/*!
	Служит для организации умных ссылок на наследников.
*/
class SO_5_TYPE atomic_refcounted_t
{
		//! Запрещаем конструктор копирования.
		atomic_refcounted_t(
			const atomic_refcounted_t & );

		//! Запрещаем оператор присваивания.
		atomic_refcounted_t &
		operator = (
			const atomic_refcounted_t & );

	public:
		//! Конструктор по умолчанию.
		//! Устанавливает количество ссылок в 0.
		atomic_refcounted_t();

		virtual ~atomic_refcounted_t();

		//! Увеличить количество ссылок.
		inline void
		inc_ref_count()
		{
			++m_ref_counter;
		}

		//! Уменьшить количество ссылок.
		/*!
			количество ссылок на объект ПОСЛЕ декркмента.
		*/
		inline unsigned long
		dec_ref_count()
		{
			return --m_ref_counter;
		}

	private:
		//! Счетчик ссылок на объект.
		atomic_counter_t m_ref_counter;
};

} /* namespace rt */

} /* namespace so_5 */

#endif
