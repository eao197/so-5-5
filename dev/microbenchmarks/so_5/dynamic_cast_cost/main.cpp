#include <iostream>
#include <ctime>

#include "msg_consumer.hpp"

const unsigned int ITERATIONS = 50000000;

void
call_with_dynamic_cast(
	message_consumer_t & consumer,
	const msg_t * msg )
	{
		consumer.handle( *dynamic_cast< const concrete_message_t * >(msg) );
	}

void
test_with_dynamic_cast()
	{
		concrete_message_t msg;
		message_consumer_t consumer;

		for( unsigned int i = 0; i != ITERATIONS; ++i )
			call_with_dynamic_cast( consumer, &msg );
	}

void
call_without_dynamic_cast(
	message_consumer_t & consumer,
	const msg_t * msg )
	{
		consumer.handle( *reinterpret_cast< const concrete_message_t * >(msg) );
	}

void
test_without_dynamic_cast()
	{
		concrete_message_t msg;
		message_consumer_t consumer;

		for( unsigned int i = 0; i != ITERATIONS; ++i )
			call_without_dynamic_cast( consumer, &msg );
	}


template< class TEST_CASE >
void
run_and_measure(
	const char * test_name,
	TEST_CASE test_case )
	{
		const std::clock_t begin = std::clock();

		test_case();

		const std::clock_t end = std::clock();

		const double duration = std::difftime(end,begin) / CLOCKS_PER_SEC;
		std::cout << test_name << ": " << duration << "s" << std::endl;
	}

int
main()
	{
		run_and_measure( "with_dynamic_cast", test_with_dynamic_cast );
		run_and_measure( "without_dynamic_cast", test_without_dynamic_cast );

		return 0;
	}

