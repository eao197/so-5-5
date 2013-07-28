/*
	Тестирование mbapi_4::impl::network_environment_t.
*/

#include <test/mbapi_4/network_environment/h/tests.hpp>

void
check_endpoints(
	std::set< endpoint_t > & expected_endpoints,
	const endpoint_list_t & endpoint_list,
	UT_CONTEXT_DECL )
{
	for(
		endpoint_list_t::const_iterator
			it = endpoint_list.begin(),
			it_end = endpoint_list.end();
		it != it_end;
		++it )
	{
		UT_PUSH_CONTEXT( "Check endpoint " ) << "{" << it->name() << "}";
		UT_CHECK_EQ( 1, expected_endpoints.erase( *it ) );
		UT_POP_CONTEXT();
	}

	UT_CHECK_EQ( 0, expected_endpoints.size() );
}

void
check_stages(
	std::set< stagepoint_t > & expected_stages,
	const stagepoint_list_t & stagepoint_list,
	UT_CONTEXT_DECL )
{
	for(
		stagepoint_list_t::const_iterator
			it = stagepoint_list.begin(),
			it_end = stagepoint_list.end();
		it != it_end;
		++it )
	{
		UT_PUSH_CONTEXT( "Check stagepoint " ) << "{" << it->name() << ", "
			<< it->endpoint_name() << "}";
		UT_CHECK_EQ( 1, expected_stages.erase( *it ) );
		UT_POP_CONTEXT();
	}

	UT_CHECK_EQ( 0, expected_stages.size() );
}