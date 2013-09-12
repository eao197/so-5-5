/*
	SObjectizer 5.
*/

#include <algorithm>

#include <ace/Guard_T.h>

#include <so_5/h/exception.hpp>

#include <so_5/rt/impl/h/local_mbox.hpp>
#include <so_5/rt/impl/h/named_local_mbox.hpp>
#include <so_5/rt/impl/h/mbox_core.hpp>

namespace so_5
{

namespace rt
{

namespace impl
{

//
// mbox_core_t
//

mbox_core_t::mbox_core_t(
	unsigned int mutex_pool_size )
	:
		m_mbox_mutex_pool( mutex_pool_size )
{
}

mbox_core_t::~mbox_core_t()
{
}


mbox_ref_t
mbox_core_t::create_local_mbox()
{
	mbox_ref_t mbox_ref( new local_mbox_t( *this ) );

	return mbox_ref;
}

mbox_ref_t
mbox_core_t::create_local_mbox(
	const nonempty_name_t & nonempty_name )
{
	const std::string name = nonempty_name.query_name();
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_dictionary_lock );

	named_mboxes_dictionary_t::iterator it =
		m_named_mboxes_dictionary.find( name );

	if( m_named_mboxes_dictionary.end() != it )
	{
		++(it->second.m_external_ref_count);
		return mbox_ref_t(
			new named_local_mbox_t(
				name,
				it->second.m_mbox,
				*this ) );
	}

	// There is no mbox with such name. New mbox should be created.
	mbox_ref_t mbox_ref( new local_mbox_t( *this ) );
	m_named_mboxes_dictionary[ name ] = named_mbox_info_t( mbox_ref );

	return mbox_ref_t( new named_local_mbox_t( name, mbox_ref, *this ) );
}

mbox_ref_t
mbox_core_t::create_local_mbox(
	std::unique_ptr< ACE_RW_Thread_Mutex >
		lock_ptr )
{
	mbox_ref_t mbox_ref(
		new local_mbox_t(
			*this,
			*lock_ptr.release() ) );

	return mbox_ref;
}

mbox_ref_t
mbox_core_t::create_local_mbox(
	const nonempty_name_t & nonempty_name,
	std::unique_ptr< ACE_RW_Thread_Mutex >
		lock_ptr )
{
	const std::string name = nonempty_name.query_name();
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_dictionary_lock );

	named_mboxes_dictionary_t::iterator it =
		m_named_mboxes_dictionary.find( name );

	if( m_named_mboxes_dictionary.end() != it )
	{
		++(it->second.m_external_ref_count);
		return mbox_ref_t(
			new named_local_mbox_t(
				name,
				it->second.m_mbox,
				*this ) );
	}

	// There is no mbox with such name. New mbox should be created.
	mbox_ref_t mbox_ref( new local_mbox_t(
		*this,
		*lock_ptr.release() ) );

	m_named_mboxes_dictionary[ name ] = named_mbox_info_t( mbox_ref );

	return mbox_ref_t( new named_local_mbox_t( name, mbox_ref, *this ) );
}

void
mbox_core_t::destroy_mbox(
	const std::string & name )
{
	ACE_Write_Guard< ACE_RW_Thread_Mutex > lock( m_dictionary_lock );

	named_mboxes_dictionary_t::iterator it =
		m_named_mboxes_dictionary.find( name );

	if( m_named_mboxes_dictionary.end() != it )
	{
		const unsigned int ref_count = --(it->second.m_external_ref_count);
		if( 0 == ref_count )
			m_named_mboxes_dictionary.erase( it );
	}
}

ACE_RW_Thread_Mutex &
mbox_core_t::allocate_mutex()
{
	return m_mbox_mutex_pool.allocate_mutex();
}

void
mbox_core_t::deallocate_mutex( ACE_RW_Thread_Mutex & m )
{
	if( !m_mbox_mutex_pool.deallocate_mutex( m ) )
	{
		// Mutex is not from pool. Assume that is was created by user
		// and should be deleted.
		delete &m;
	}
}

//
// mbox_core_ref_t
//

mbox_core_ref_t::mbox_core_ref_t()
	:
		m_mbox_core_ptr( nullptr )
{
}

mbox_core_ref_t::mbox_core_ref_t(
	mbox_core_t * mbox_core )
	:
		m_mbox_core_ptr( mbox_core )
{
	inc_mbox_core_ref_count();
}

mbox_core_ref_t::mbox_core_ref_t(
	const mbox_core_ref_t & mbox_core_ref )
	:
		m_mbox_core_ptr( mbox_core_ref.m_mbox_core_ptr )
{
	inc_mbox_core_ref_count();
}

void
mbox_core_ref_t::operator = (
	const mbox_core_ref_t & mbox_core_ref )
{
	if( &mbox_core_ref != this )
	{
		dec_mbox_core_ref_count();

		m_mbox_core_ptr = mbox_core_ref.m_mbox_core_ptr;
		inc_mbox_core_ref_count();
	}

}

mbox_core_ref_t::~mbox_core_ref_t()
{
	dec_mbox_core_ref_count();
}

inline void
mbox_core_ref_t::dec_mbox_core_ref_count()
{
	if( m_mbox_core_ptr &&
		0 == m_mbox_core_ptr->dec_ref_count() )
	{
		delete m_mbox_core_ptr;
		m_mbox_core_ptr = nullptr;
	}
}

inline void
mbox_core_ref_t::inc_mbox_core_ref_count()
{
	if( m_mbox_core_ptr )
		m_mbox_core_ptr->inc_ref_count();
}

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */
