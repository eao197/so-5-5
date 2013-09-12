/*
	SObjectizer 5.
*/

/*!
	\file
	\brief An addition layer for SObjectizer Environment definition.
*/

#if !defined( _SO_5__RT__SO_LAYER_HPP_ )
#define _SO_5__RT__SO_LAYER_HPP_

#include <memory>
#include <map>

#include <ace/Refcounted_Auto_Ptr.h>
#include <ace/Null_Mutex.h>

#include <so_5/h/declspec.hpp>
#include <so_5/h/ret_code.hpp>

#include <so_5/rt/h/type_wrapper.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;

namespace impl
{

class layer_core_t;

} /* namespace impl */


//
// so_layer_t
//

//! An interface of additional SObjectizer Environment layer.
/*!
*/
class SO_5_TYPE so_layer_t
{
		friend class impl::layer_core_t;
	public:
		so_layer_t();
		virtual ~so_layer_t();


		//! Start hook.
		/*!
		 * Default implementation do nothing.
		 */
		virtual ret_code_t
		start();

		//! Shutdown signal hook.
		/*!
		 * Default implementation do nothing.
		 */
		virtual void
		shutdown();

		//! Waiting for complete shutdown of layer.
		/*!
		 * Default implementation do nothing and returned immediately.
		*/
		virtual void
		wait();

	protected:
		//! Access to SObjectizer Environment.
		/*!
		 * Throws an exception if layer is not bound to
		 * SObjectizer Environment.
		 */
		so_environment_t &
		so_environment();

	private:
		//! Bind layer to SObjectizer Environment.
		void
		bind_to_environment( so_environment_t * env );

		//! SObjectizer Environment to which layer is bound.
		/*!
		 * Has actual value only after binding to environment.
		 */
		so_environment_t * m_so_environment;
};

//! Typedef for layer's autopointer.
typedef std::unique_ptr< so_layer_t > so_layer_unique_ptr_t;

//! Typedef for layer's smart pointer.
typedef std::shared_ptr< so_layer_t > so_layer_ref_t;

//! Typedef for map from layer typeid to layer.
typedef std::map< type_wrapper_t, so_layer_ref_t > so_layer_map_t;

} /* namespace rt */

} /* namespace so_5 */

#endif

