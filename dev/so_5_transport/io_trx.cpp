/*
	SObjectizer 5 Transport.
*/

#include <so_5_transport/h/io_trx.hpp>

namespace so_5_transport
{

//
// input_trx_t
//

input_trx_t::input_trx_t(
	channel_input_stream_unique_ptr_t istream )
	:
		m_istream( std::move( istream ) )
{
	m_istream->begin_transaction();
}

void
input_trx_t::checkpoint()
{
	m_istream->checkpoint();
}

void
input_trx_t::commit()
{
	m_istream->commit_transaction();
}

void
input_trx_t::rollback()
{
	m_istream->rollback_transaction();
}

//
// output_trx_t
//

output_trx_t::output_trx_t(
	channel_output_stream_unique_ptr_t ostream )
	:
		m_ostream( std::move( ostream ) )
{
	m_ostream->begin_transaction();
}

void
output_trx_t::checkpoint()
{
	m_ostream->checkpoint();
}

void
output_trx_t::commit()
{
	m_ostream->commit_transaction();
}

void
output_trx_t::rollback()
{
	m_ostream->rollback_transaction();
}

} /* namespace so_5_transport */
