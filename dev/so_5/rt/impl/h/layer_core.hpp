/*
	SObjectizer 5.
*/

/*!
	\file
	\brief A definition of utility class for work with layers.
*/

#if !defined( _SO_5__RT__IMPL__LAYER_CORE_HPP_ )
#define _SO_5__RT__IMPL__LAYER_CORE_HPP_

#include <vector>

#include <ace/RW_Thread_Mutex.h>

#include <so_5/h/ret_code.hpp>
#include <so_5/h/throwing_strategy.hpp>

#include <so_5/rt/h/type_wrapper.hpp>
#include <so_5/rt/h/so_layer.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;

namespace impl
{

//
// typed_layer_ref_t
//

/*!
 * \brief A special wrapper to store layer with its type.
 */
struct typed_layer_ref_t
{
	typed_layer_ref_t();
	typed_layer_ref_t( const so_layer_map_t::value_type & v );
	typed_layer_ref_t(
		const type_wrapper_t & type,
		const so_layer_ref_t & layer );

	bool
	operator < ( const typed_layer_ref_t & tl ) const;

	//! Type of layer.
	type_wrapper_t m_true_type;

	//! Layer itself.
	so_layer_ref_t m_layer;
};

//! Typedef for typed_layer_ret container.
typedef std::vector< typed_layer_ref_t >
	so_layer_list_t;

//
// layer_core_t
//

//! An utility class for work with layers.
/*!
 * There are two type of layers:
 * - default layers. They are known before the SObjectizer Run-Time started.
 *   Those layers should be passed to the layer_core_t constructor;
 * - extra layers. They have added when the SObjectizer Run-Time are working.
 *
 * The main difference between them is that default layers started when
 * SObjectizer Run-Time is not working. They are started during the Run-Time
 * initialization routine.
 *
 * Extra layers get started when the Run-Time is working.
 *
 * During the SObjectizer Run-Time should extra layers should be shutdowned
 * before default layers.
 */
class layer_core_t
{
	public:
		layer_core_t(
			//! Layers which are known before SObjectizer start.
			const so_layer_map_t & so_layers,
			//! SObjectizer Environment to work with.
			so_environment_t * env );
		~layer_core_t();

		//! Get a layer.
		/*!
		 * \retval nullptr if layer is not found.
		 */
		so_layer_t *
		query_layer(
			const type_wrapper_t & type ) const;

		//! Start all layers.
		ret_code_t
		start();

		//! Shutdown extra layers.
		void
		shutdown_extra_layers();

		//! Blocking wait for complete shutdown of all extra layers.
		/*!
		 * All extra layers are destroyed.
		 */
		void
		wait_extra_layers();

		//! Shutdown default layers.
		void
		shutdown_default_layers();

		//! Blocking wait for complete shutdown of all default layers.
		void
		wait_default_layers();

		//! Add an extra layer.
		ret_code_t
		add_extra_layer(
			const type_wrapper_t & type,
			const so_layer_ref_t & layer,
			throwing_strategy_t throwing_strategy );

	private:
		//! SObjectizer Environment to work with.
		so_environment_t * m_env;

		//! Default layers.
		/*!
		 * Value is set in the constructor and will never change.
		 */
		so_layer_list_t m_default_layers;

		//! Object lock for the extra layers data.
		mutable ACE_RW_Thread_Mutex m_extra_layers_lock;

		//! Extra layers.
		so_layer_list_t m_extra_layers;
};

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

#endif
