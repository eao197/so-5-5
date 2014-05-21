/*
	SObjectizer 5.
*/

/*!
	\file
	\since v.5.3.0
	\brief Interface for the agent execution context.
*/

#if !defined( SO_5__RT__EXEC_CTX_HPP )
#define SO_5__RT__EXEC_CTX_HPP

#include <so_5/h/declspec.hpp>

namespace so_5
{

namespace rt
{

//
// exec_ctx_t
//
/*!
 * \since v.5.3.0
 * \brief An interface for the agent execution context.
 */
class SO_5_TYPE exec_ctx_t
	{
	public :
		virtual ~exec_ctx_t();

//TODO: should be uncommented!
/*
		virtual void
		push( exec_demand_t && demand ) = 0;
*/
	};

} /* namespace rt */

} /* namespace so_5 */

#endif

