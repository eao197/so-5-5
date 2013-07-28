/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Коды ошибок транспортного слоя.
*/

#if !defined( _SO_5_TRANSPORT__RET_CODE_HPP_ )
#define _SO_5_TRANSPORT__RET_CODE_HPP_

#include <so_5/h/ret_code.hpp>

#include <iostream>

namespace so_5_transport
{

//! Вспомогательный макрос для определения кодов ошибок транспортного слоя.
#define SO_5_TRANSPORT_RET_CODE( rc ) 1000 + rc

//! \name Ошибки работы с реакторами.
//! \{

const int rc_reactor_task_activation_failed = SO_5_TRANSPORT_RET_CODE( 1 );
const int rc_default_reactor_start_failed = SO_5_TRANSPORT_RET_CODE( 2 );
const int rc_named_reactor_start_failed = SO_5_TRANSPORT_RET_CODE( 3 );
const int rc_failed_to_enforce_input_detection_no_free_blocks = SO_5_TRANSPORT_RET_CODE( 4 );

//! Слой реакторов не установлен.
const int rc_reactor_layer_is_not_set = SO_5_TRANSPORT_RET_CODE( 5 );

//! Реактор не найден.
const int rc_reactor_not_found = SO_5_TRANSPORT_RET_CODE( 6 );

//! \}


//! \name Ошибки транспортного слоя.
//! \{

//! Не удается создать acceptor.
const int rc_acceptor_creation_failed = SO_5_TRANSPORT_RET_CODE( 10 );

//! В буфере недостаточно места для чтения данных.
const int rc_not_enough_data_to_read_from_buffer = SO_5_TRANSPORT_RET_CODE( 11 );
//! В буфере нет места для записи данных.
const int rc_no_free_blocks_available_in_buffer = SO_5_TRANSPORT_RET_CODE( 12 );
//! Попытка начать запись в поток над которым не начата транзакция.
const int rc_transactiorn_not_started = SO_5_TRANSPORT_RET_CODE( 13 );

//! При указании mbox-а на который отсылать уведомления
//! о событиях с каналом, был указан пустой mbox.
const int rc_notification_mbox_zero_ptr = SO_5_TRANSPORT_RET_CODE( 20 );

//! При указании mbox-а транспортного агента, был указан пустой mbox.
const int rc_ta_mbox_zero_ptr = SO_5_TRANSPORT_RET_CODE( 21 );

//! Нельзя выполнять действий с контроллером после того как он был закрыт.
const int rc_controller_is_closed = SO_5_TRANSPORT_RET_CODE( 30 );

//! Не удается установить соединение с сервером.
const int rc_connect_failed = SO_5_TRANSPORT_RET_CODE( 40 );

//! \}


//! \name Ошибки общего плана.
//! \{

//! Маска для ошибок общего плана.
const int rc_common_error_mask = 0x100000;

//! Пустое имя ( mbox-а, кооперации ).
const int rc_empty_name = rc_common_error_mask | 1;

const int rc_unexpected_error = 0xFFFFFF;
//! \}

} /* namespace so_5_transport */

#endif
