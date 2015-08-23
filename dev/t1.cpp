#include <so_5/all.hpp>

#include <iostream>
#include <type_traits>

struct msg : public so_5::rt::message_t 
{
	int m_v;

	msg( int v ) : m_v(v) {}
};

struct signal : public so_5::rt::signal_t
{
};

template< typename M, typename L >
struct log_then_abort_app_indicator_t
	{
		//! Max count of waiting messages.
		const unsigned int m_limit;

		//! Logging lambda.
		const L m_lambda;

		//! Initializing constructor.
		log_then_abort_app_indicator_t(
			unsigned int limit,
			L lambda )
			:	m_limit( limit )
			,	m_lambda( std::move( lambda ) )
			{}
	};

template< typename M, typename L >
log_then_abort_app_indicator_t< M, L >
log_then_abort( unsigned int limit, L action )
{
	return log_then_abort_app_indicator_t< M, L >{ limit, std::move(action) };
}

template< bool IS_MESSAGE, typename M, typename L >
struct call_logging_action_impl {
	static void
	call(
		const so_5::rt::message_limit::overlimit_context_t & ctx,
		L action )
			{
				const M & m = dynamic_cast< const M & >( *ctx.m_message );
				action( ctx.m_receiver, m );
			}
};

template< typename M, typename L >
struct call_logging_action_impl< false, M, L > {
	static void
	call(
		const so_5::rt::message_limit::overlimit_context_t & ctx,
		L action )
		{
			action( ctx.m_receiver );
		}
};

template< typename M, typename L >
struct call_logging_action
	:	public call_logging_action_impl< so_5::rt::is_message< M >::value, M, L >
{};

template< typename M, typename L >
void
call( const log_then_abort_app_indicator_t< M, L > & indicator,
	const so_5::rt::message_limit::overlimit_context_t & ctx )
{
	call_logging_action< M, L >::call( ctx, indicator.m_lambda );
}


void test()
{
	using namespace so_5::rt;
	using namespace so_5::rt::message_limit;

	{
		overlimit_context_t ctx{
			*( static_cast< agent_t * >(nullptr) ),
			*( static_cast< control_block_t * >(nullptr) ),
			invocation_type_t::event,
			1,
			typeid(signal),
			message_ref_t{}
		};

		auto indicator = log_then_abort< signal >( 10u,
			[]( const agent_t & ) {
				std::cout << "this a signal" << std::endl;
			} );
		call( indicator, ctx );
	}
}

int main()
{
	test();
}

