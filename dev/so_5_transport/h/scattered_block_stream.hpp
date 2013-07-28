/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Класс для оперирования потоком буфферов.
*/

#if !defined( _SO_5_TRANSPORT__SCATTERED_BLOCK_STREAM_HPP_ )
#define _SO_5_TRANSPORT__SCATTERED_BLOCK_STREAM_HPP_

#include <list>
#include <memory>

#include <ace/Time_Value.h>
#include <ace/Thread_Mutex.h>
#include <ace/Condition_Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <so_5_transport/h/declspec.hpp>
#include <so_5_transport/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/atomic_refcounted.hpp>

#include <so_5_transport/h/raw_block.hpp>

namespace so_5_transport
{

class scattered_block_stream_t;

namespace block_stream_notifiers
{

class SO_5_TRANSPORT_TYPE flush_notifier_t
{
	public:
		virtual ~flush_notifier_t();

		virtual void
		notify() = 0;

		virtual void
		reset() = 0;
};

class SO_5_TRANSPORT_TYPE free_blocks_availability_notifier_t
{
	public:
		virtual ~free_blocks_availability_notifier_t();

		virtual void
		notify_lacking_free_blocks() = 0;

		virtual void
		notify_free_blocks(
			size_t default_block_count,
			size_t available_block_count ) = 0;
};

class SO_5_TRANSPORT_TYPE stream_operation_notifier_t
{
	public:
		virtual ~stream_operation_notifier_t();

		virtual void
		notify_blocks_pushed_back() = 0;

		virtual void
		notify_blocks_pushed_front() = 0;

		virtual void
		notify_blocks_queried() = 0;
};

} /* namespace block_stream_notifiers */

//
// scattered_block_stream_params_t
//

//! Параметры блочного потока-буфера.
struct SO_5_TRANSPORT_TYPE scattered_block_stream_params_t
{
	//! Обычное количество блоков.
	/*!
		Значение по умолчанию: 16.
	*/
	size_t m_block_count;

	//! Порог до которого можно увеличивать
	//! количество блоков.
	/*!
		Значение по умолчанию: 64.
	*/
	size_t m_max_block_count;

	//! Размер единичного блока.
	/*!
		Значение по умолчанию: 32*1024.
	*/
	size_t m_single_block_size;

	scattered_block_stream_params_t();
	scattered_block_stream_params_t(
		//! количество блоков.
		size_t block_count,
		//! Порог до которого можно увеличивать
		//! количество блоков.
		size_t max_block_count,
		//! Размер единичного блока.
		size_t single_block_size );
};

//
// scattered_block_stream_t
//

//! Поток блоков-буферов.
/*!
	Входящие и исходящие данные передаются между транспортными
	агентами SObjectizer и Ace_Reactor frasmework через
	scattered_block_stream_t. Но агентов SObjectizer
	на прямую не работают с scattered_block_stream_t, а
	работают на один уровень абстракции выше с
	channel_input_stream_t и channel_input_stream_t
	при помощи механизма транзакций.

	Класс содержит пул блоков-буферов. Из этого пула можно брать блоки
	писать в них и вставлять в поток.
	Из потока блоков можно брать блоки и зачитывать из них данные,
	после чего возвращать блоки в пул, а непрочитанные блоки в начало потока.
*/
class SO_5_TRANSPORT_TYPE scattered_block_stream_t
	:
		private so_5::rt::atomic_refcounted_t
{
		friend class scattered_block_stream_ref_t;

	public:
		scattered_block_stream_t(
			//! Обычное количество блоков.
			size_t block_count,
			//! Порог до которого можно увеличивать
			//! количество блоков.
			size_t max_block_count,
			//! Размер единичного блока.
			size_t single_block_size );

		explicit scattered_block_stream_t(
			//! Параметры.
			const scattered_block_stream_params_t & scattered_block_stream_params );

		virtual ~scattered_block_stream_t();

		//! Переустановить уведомителей.
		//! \{
		void
		reset_flush_notifier(
			block_stream_notifiers::flush_notifier_t * flush_notifier = nullptr );

		void
		reset_free_blocks_availability_notifier(
			block_stream_notifiers::free_blocks_availability_notifier_t *
				free_blocks_availability_notifier = nullptr );
		void
		reset_stream_operation_notifier(
			block_stream_notifiers::stream_operation_notifier_t *
				stream_operation_notifier = nullptr );
		//! \}

		//! \name Интерфейс для получения и возврата блоков из/в пул.
		//! \{

		//! Получить блок для записи/чтения.
		raw_block_t *
		occupy(
			//! Приемник флага: было ли ожидание свободных блоков.
			bool & was_waiting_for_free_blocks,
			//! Допустимое время ожидания.
			//! * \@ timeout = 0, ожидать без ограничения.
			//! * \@ timeout = &ACE_Time_Value::zero не ждать.
			//! * \@ timeout = &some_time - ожидать какое-то время.
			const ACE_Time_Value * timeout = &ACE_Time_Value::zero );

		//! Получить блок для записи/чтения.
		raw_block_t *
		occupy(
			//! Допустимое время ожидания.
			//! * \@ timeout = 0, ожидать без ограничения.
			//! * \@ timeout = &ACE_Time_Value::zero не ждать.
			//! * \@ timeout = &some_time - ожидать какое-то время.
			const ACE_Time_Value * timeout = &ACE_Time_Value::zero );

		//! Получить несколько блоков для записи/чтения.
		/*!
			\note Если за отведенное время получить все блоки не удастся,
			то их выдиление будет отменено, и приемник не будет
			содержать результатов.
		*/
		void
		occupy(
			//! Желаемое количество блоков.
			size_t n,
			//! Приемник блоков.
			raw_block_chain_t & block_chain,
			//! Приемник флага: было ли ожидание свободных блоков.
			bool & was_waiting_for_free_blocks,
			//! Допустимое время ожидания.
			//! * \@ timeout = 0, ожидать без ограничения.
			//! * \@ timeout = &ACE_Time_Value::zero не ждать.
			//! * \@ timeout = &some_time - ожидать какое-то время.
			const ACE_Time_Value * timeout = &ACE_Time_Value::zero );

		//! Получить несколько блоков для записи/чтения.
		/*!
			\note Если за отведенное время получить все блоки не удастся,
			то их выдиление будет отменено, и приемник не будет
			содержать результатов.
		*/
		void
		occupy(
			//! Желаемое количество блоков.
			size_t n,
			//! Приемник блоков.
			raw_block_chain_t & block_chain,
			//! Допустимое время ожидания.
			//! * \@ timeout = 0, ожидать без ограничения.
			//! * \@ timeout = &ACE_Time_Value::zero не ждать.
			//! * \@ timeout = &some_time - ожидать какое-то время.
			const ACE_Time_Value * timeout = &ACE_Time_Value::zero );

		//! Вернуть блок.
		void
		release( raw_block_t * block );

		//! Вернуть цепочку блоков.
		void
		release(
			//! Цепочка блоков.
			const raw_block_chain_t & chain );
		//! \}

		//! \name Интерфейс извлечения и вставки блоков в поток.
		//! \{

		//! Поставить блок в конец потока.
		void
		stream_push_back( raw_block_t * block );

		//! Поставить блоки в конец потока.
		void
		stream_push_back( const raw_block_chain_t & chain );

		//! Поставить блок в начало потока.
		void
		stream_push_front( raw_block_t * block );

		//! Поставить блоки в начало потока.
		void
		stream_push_front( const raw_block_chain_t & chain );

		//! Оптимизированная вставка в начало.
		/*!
			В channel_input_stream_t при отмене
			транзакции и при ее подтверждении, когда не все блоки были
			до конца прочитаны, происходит вставка взятых из потока
			блоков в начало потока. Чтобы делать это быстрее
			эта функция предполагает, что пользователя
			после вызова не интересует содержимое \@ chain и
			он передает корректное значение \@ size_in_bytes,
			которое содержит размер данных в цепочке.
		*/
		void
		stream_push_front(
			//! Цепочка для вставки.
			raw_block_chain_t & chain,
			//! Размер данных в цепочке.
			size_t size_in_bytes );

		//! Извлечь все блоки из потока.
		/*!
			\return Количество байт в полученных блоках.
			\note
		*/
		size_t
		stream_query_all_blocks(
			//! Приемник результата.
			raw_block_chain_t & chain );

		//! Извлечь не более n блоков из потока.
		/*!
			\return Количество байт в полученных блоках.
		*/
		size_t
		stream_query_n_blocks(
			//! Желаемое количество блоков.
			size_t n,
			//! Приемник результата.
			raw_block_chain_t & chain );

		//! \}

		//! Сбросить поток.
		/*!
			Если в потоке есть блоки,
			то получателю через интерфейс увемителя
			отправляется уведомление.
		*/
		void
		flush();

		//! Проверить является ли поток пустым?
		/*!
			Если поток является пустым, то у уведомителя
			вызывается notify_empty_stream и взводится флаг
			о необходимости уведомления о сбросе буфера.

			\return Если в потоке есть элементы, то
			вернет true, иначе false.
		*/
		bool
		check_stream_has_blocks();

		//! Имитировать использование потока.
		void
		touch_stream();

		//! Параметры потока блоков.
		inline size_t
		default_block_count() const
		{
			return m_block_count;
		}

		//! Максимально возможноек количество блоков-буферов в обращении.
		inline size_t
		max_block_count() const
		{
			return m_max_block_count;
		}

		//! Количество блоков-буферов, которые в данный
		//! момент находятся в обращении.
		size_t
		current_block_count() const;

		//! Размер одного блока в байтах.
		inline size_t
		single_block_size() const
		{
			return m_single_block_size;
		}

	private:
		//! Псевдоним для удобного использования ACE_Guard.
		typedef ACE_Guard< ACE_Thread_Mutex > scoped_lock_t;

		//! Служебный метод для получения блока.
		raw_block_t *
		occupy_impl(
			//! Абсолютное время до которого можно ждать
			//! появления блоков, если их в данный момент не оказалось.
			//! * \@ timeout = 0, ожидать без ограничения.
			//! * \@ timeout != 0, ожидать вплоть до абсолютного времени.
			const ACE_Time_Value * abs_value,
			//! Приемник флага: было ли ожидание свободных блоков.
			bool & was_waiting_for_free_blocks );

		//! Служебный метод для возвращения блока.
		void
		release_impl(
			raw_block_t * block );

		void
		release_impl(
			const raw_block_chain_t & chain );

		//! Попытаться, если можно, выделить блоки.
		void
		allocate_blocks();

		//! Уменьшить количество блоков.
		void
		deallocate_blocks();

		//! Количество блоков по умолчанию.
		/*!
			Стратегия выделения дополнительных блоков и их освобождения
			нацелена на то чтобы держать количество свободных блоков именно
			таким.
		*/
		const size_t m_block_count;

		//! Максимально возможное количество блоков.
		const size_t m_max_block_count;

		//! Порог для удаления лишних блоков.
		/*!
			Как только образуется количество свободных блоков
			большее либо равное порогу, лишние блоки освобождаются.
		*/
		const size_t m_deallocate_block_count_threshold;

		//! Текуще количество блоков в обращении.
		size_t m_current_block_count;

		//! Размер однгого блока в байтах.
		const size_t m_single_block_size;

		//! Синфронизация работы с потоком.
		//! \{
		mutable ACE_Thread_Mutex m_lock;
		ACE_Condition_Thread_Mutex m_has_free_blocks;
		//! \}

		//! Поток блоков.
		raw_block_chain_t m_stream_chain;

		//! Имеющиеся в наличии блоки.
		raw_block_chain_t m_free_blocks;

		//! Количество байт, которое сейчас стоит в очереди потока.
		size_t m_stream_size_in_bytes;

		//! Уведомители действий.
		//! \{
		block_stream_notifiers::flush_notifier_t * m_flush_notifier;
		block_stream_notifiers::free_blocks_availability_notifier_t *
			m_free_blocks_availability_notifier;
		block_stream_notifiers::stream_operation_notifier_t *
			m_stream_operation_notifier;
		//! \}
};

//
// scattered_block_stream_ref_t
//

//! Умная ссылка не scattered_block_stream_t.
class SO_5_TRANSPORT_TYPE scattered_block_stream_ref_t
{
	public:
		scattered_block_stream_ref_t();

		explicit scattered_block_stream_ref_t(
			scattered_block_stream_t * scattered_block_stream );

		scattered_block_stream_ref_t(
			const scattered_block_stream_ref_t & scattered_block_stream_ref );

		void
		operator = ( const scattered_block_stream_ref_t & scattered_block_stream_ref );

		~scattered_block_stream_ref_t();

		inline const scattered_block_stream_t *
		get() const
		{
			return m_scattered_block_stream_ptr;
		}

		inline scattered_block_stream_t *
		get()
		{
			return m_scattered_block_stream_ptr;
		}

		inline const scattered_block_stream_t *
		operator -> () const
		{
			return m_scattered_block_stream_ptr;
		}

		inline scattered_block_stream_t *
		operator -> ()
		{
			return m_scattered_block_stream_ptr;
		}

		inline scattered_block_stream_t &
		operator * ()
		{
			return *m_scattered_block_stream_ptr;
		}


		inline const scattered_block_stream_t &
		operator * () const
		{
			return *m_scattered_block_stream_ptr;
		}

	private:
		//! Увеличить количество ссылок на scattered_block_stream
		//! и в случае необходимости удалить его.
		void
		inc_scattered_block_stream_ref_count();

		//! Уменьшить количество ссылок на scattered_block_stream
		//! и в случае необходимости удалить его.
		void
		dec_scattered_block_stream_ref_count();

		//! Хранимый объект.
		scattered_block_stream_t * m_scattered_block_stream_ptr;
};

} /* namespace so_5_transport */

#endif
