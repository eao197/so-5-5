/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Interface for cooperation listener definition.
*/

#if !defined( _SO_5__RT__COOP_LISTENER_HPP_ )
#define _SO_5__RT__COOP_LISTENER_HPP_

#include <string>
#include <memory>

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

class so_environment_t;

//
// coop_listener_t
//

//! Interface for cooperation listener.
/*!
 * Cooperation listener intended for observation moments of
 * cooperation registrations and deregistrations.
 *
 * \attention SObjectizer doesn't synchronize calls to
 * on_registered() and on_deregistered(). If this is a problem
 * then programmer should take care about object's thread safety.
 */
class SO_5_TYPE coop_listener_t
{
	public:
		virtual ~coop_listener_t();

		//! Hook for cooperation registration event.
		/*!
		 * Called right after cooperation successfully registered.
		 */
		virtual void
		on_registered(
			//! SObjectizer Environment.
			so_environment_t & so_env,
			//! Cooperation which was registered.
			const std::string & coop_name ) = 0;

		//! Hook for cooperation deregistration event.
		/*!
		 * Called right after cooperation fully deregistered.
		 */
		virtual void
		on_deregistered(
			//! SObjectizer Environment.
			so_environment_t & so_env,
			//! Cooperation which was registered.
			const std::string & coop_name ) = 0;
};

//! Typedef for coop_listener autopointer.
typedef std::unique_ptr< coop_listener_t > coop_listener_unique_ptr_t;

} /* namespace rt */

} /* namespace so_5 */

#endif
