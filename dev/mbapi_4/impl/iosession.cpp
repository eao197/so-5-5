/*
	MBAPI 4.
*/

#include <mbapi_4/impl/h/iosession.hpp>

namespace mbapi_4
{

namespace impl
{

//
// isession_t
//

isession_t::isession_t(
	oess_2::io::ibinstream_t & istream,
	ACE_RW_Thread_Mutex & lock,
	const types_repositories_list_t & types_repositories )
	:
		m_lock( lock ),
		m_ient( new oess_2::stdsn::ient_std_t( istream ) )
{
	m_lock.acquire_read();

	for(
		types_repositories_list_t::const_iterator
			it = types_repositories.begin(),
			it_end = types_repositories.end();
		it != it_end;
		++it )
	{
		m_ient->add_repository( **it );
	}
}

isession_t::~isession_t()
{
	m_lock.release();
}

//
// osession_t
//

osession_t::osession_t(
	oess_2::io::obinstream_t & ostream )
	:
		m_oent( new oess_2::stdsn::oent_std_t( ostream ) )
{
}

osession_t::~osession_t()
{
}

} /* namespace impl */

} /* namespace mbapi_4 */
