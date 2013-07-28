/*
	Тестирование потоков данных канала.

	Суть теста:
		Создаются so_5_transport::scattered_block_stream_t
		с различными параметрами и подвергаются использоваиню
*/


#include <iostream>
#include <sstream>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <cpp_util_2/h/lexcast.hpp>
#include <cpp_util_2/h/defs.hpp>

#include <so_5/rt/h/rt.hpp>
#include <so_5_transport/h/scattered_block_stream.hpp>

#include <utest_helper_1/h/helper.hpp>

inline unsigned int
rand_num( unsigned int n )
{
	return rand() % n;
}

std::ostream &
operator << (
	std::ostream & o,
	const so_5_transport::scattered_block_stream_t & bs )
{
	return o << "[ " << "block_count = " << bs.default_block_count()
		<< ", max_block_count = " << bs.max_block_count()
		<< ", single_block_size = " << bs.single_block_size()
		<< " ]";
}

inline void
check_scattered_block_stream_params(
	size_t block_count,
	size_t max_block_count,
	size_t single_block_size,
	const so_5_transport::scattered_block_stream_t & bs )
{
	UT_CHECK_EQ( block_count, bs.default_block_count() );
	UT_CHECK_EQ( max_block_count, bs.max_block_count() );
	UT_CHECK_EQ( single_block_size, bs.single_block_size() );
}

so_5_transport::scattered_block_stream_ref_t
create_scatered_block_stream_ctor1(
	size_t block_count,
	size_t max_block_count,
	size_t single_block_size )
{
	so_5_transport::scattered_block_stream_params_t params(
		block_count,
		max_block_count,
		single_block_size );

	so_5_transport::scattered_block_stream_ref_t
		block_stream( new so_5_transport::scattered_block_stream_t(
			params ) );

	check_scattered_block_stream_params(
		block_count,
		max_block_count,
		single_block_size,
		*block_stream );

	return block_stream;
}

so_5_transport::scattered_block_stream_ref_t
create_scatered_block_stream_ctor2(
	size_t block_count,
	size_t max_block_count,
	size_t single_block_size )
{
	so_5_transport::scattered_block_stream_ref_t
		block_stream( new so_5_transport::scattered_block_stream_t(
			block_count,
			max_block_count,
			single_block_size ) );

	check_scattered_block_stream_params(
		block_count,
		max_block_count,
		single_block_size,
		*block_stream );

	return block_stream;
}


void
simple_working_circle(
	so_5_transport::scattered_block_stream_ref_t & block_stream,
	UT_CONTEXT_DECL )
{
	// Захватываем по одному блоку
	// ставим его в поток, извлекаем его из потока,
	// возвращаем в block_stream

	UT_PUSH_CONTEXT( "Simple working circle" );

	for( int i = 0; i < block_stream->max_block_count()*3; ++i )
	{
		UT_PUSH_CONTEXT( "iteration" ) << "#" << i;
		so_5_transport::raw_block_t * rb;
		rb = block_stream->occupy();

		UT_CHECK_CONDITION( 0 != rb );

		// Блок должен быть готов к записи.
		UT_CHECK_EQ( 0, rb->read_pos() );
		UT_CHECK_EQ( 0, rb->write_pos() );

		const size_t single_block_size = block_stream->single_block_size();
		// Для записи должно быть доступно
		// single_block_size байт, а для чтения
		// ничего не должно быть доступно.
		UT_CHECK_EQ( single_block_size, rb->write_size_available() );
		UT_CHECK_EQ( 0, rb->read_size_available() );

		// Как будто что-то туда записали.
		rb->total_shift_write_pos();

		// Теперь наоборот.
		UT_CHECK_EQ( 0, rb->write_size_available() );
		UT_CHECK_EQ( single_block_size, rb->read_size_available() );

		block_stream->stream_push_back( rb );

		// Теперь в потоке должны быть элементы
		UT_CHECK_CONDITION( true == block_stream->check_stream_has_blocks() );

		so_5_transport::raw_block_chain_t block_chain;

		size_t obtained_size = block_stream->stream_query_n_blocks(
			1, block_chain );

		// Должны были получить ровно 1 блок.
		UT_CHECK_EQ( 1, block_chain.size() );

		// Получено должно быть столько байт сколько в одном блоке.
		UT_CHECK_EQ( obtained_size, single_block_size );

		// Этот блок должен быть именно тем который вставлялся.
		UT_CHECK_EQ( rb, block_chain.front() );

		// В потоке не должно ничего быть.
		UT_CHECK_CONDITION( false == block_stream->check_stream_has_blocks() );

		// Как-бы  все прочитали.
		block_chain.front()->total_shift_read_pos();

		// Не должно быть ничего доступно для чтения.
		UT_CHECK_EQ( 0, block_chain.front()->read_size_available() );

		// отдаем блок обратно.
		block_stream->release( rb );

		UT_POP_CONTEXT();
	}

	UT_POP_CONTEXT();
}

inline void
read_block( so_5_transport::raw_block_t * rb )
{
	rb->total_shift_read_pos();
	UT_CHECK_CONDITION( 0 == rb->read_size_available() );
}

void
gready_working_circle(
	so_5_transport::scattered_block_stream_ref_t & block_stream,
	UT_CONTEXT_DECL )
{
	// Захватываем по одному блоку, пока есть свободные блоки,
	// ставим их всех в поток,
	// извлекаем все из потока.
	// возвращаем в block_stream.
	UT_PUSH_CONTEXT( "Simple working circle" );

	for( int i = 0; i < 4; ++i )
	{
		UT_PUSH_CONTEXT( "iteration" ) << "#" << i;

		so_5_transport::raw_block_chain_t initial_chain;
		for( int j = 0; j < block_stream->max_block_count(); ++j )
		{
			UT_PUSH_CONTEXT( "simulating writing to blocks" )
				<< " block #" << j;

			so_5_transport::raw_block_t * rb;
			rb = block_stream->occupy();

			UT_CHECK_CONDITION( 0 != rb );

			// Блок должен быть готов к записи.
			UT_CHECK_EQ( 0, rb->read_pos() );
			UT_CHECK_EQ( 0, rb->write_pos() );

			const size_t single_block_size = block_stream->single_block_size();
			// Для записи должно быть доступно
			// single_block_size байт, а для чтения
			// ничего не должно быть доступно.
			UT_CHECK_EQ( single_block_size, rb->write_size_available() );
			UT_CHECK_EQ( 0, rb->read_size_available() );

			// Как будто что-то туда записали.
			rb->total_shift_write_pos();

			// Теперь наоборот.
			UT_CHECK_EQ( 0, rb->write_size_available() );
			UT_CHECK_EQ( single_block_size, rb->read_size_available() );

			initial_chain.push_back( rb );

			UT_POP_CONTEXT();
		}

		block_stream->stream_push_back( initial_chain );

		// Теперь в потоке должны быть элементы.
		UT_CHECK_CONDITION( true == block_stream->check_stream_has_blocks() );

		so_5_transport::raw_block_chain_t block_chain;

		size_t obtained_size = block_stream->stream_query_all_blocks(
			block_chain );

		// Должны были получить ровно то что записывали в поток.
		UT_CHECK_EQ( initial_chain.size(), block_chain.size() );

		UT_CHECK_CONDITION( std::equal(
			initial_chain.begin(),
			initial_chain.end(),
			block_chain.begin() ) );

		initial_chain.clear();

		// Получено должно быть столько байт сколько в одном блоке
		// помноженное на максимальное количество блоков.

		UT_CHECK_EQ(
			obtained_size,
			block_stream->single_block_size() *
				block_stream->max_block_count() );

		// В потоке не должно ничего быть.
		UT_CHECK_CONDITION( false == block_stream->check_stream_has_blocks() );

		// Как-бы  все прочитали.
		std::for_each(
			block_chain.begin(),
			block_chain.end(),
			read_block );

		// Отдаем блоки обратно.
		block_stream->release( block_chain );

		UT_POP_CONTEXT();
	}
	UT_POP_CONTEXT();
}

void
random_working_circle(
	so_5_transport::scattered_block_stream_ref_t & block_stream,
	UT_CONTEXT_DECL )
{
	// Захватываем по неколько блоков,
	// ставим их всех в поток,
	// извлекаем все из потока.
	// возвращаем в block_stream.
	UT_PUSH_CONTEXT( "Random working circle" );

	so_5_transport::raw_block_chain_t model_stream_chain;

	for( int i = 0; i < 100; ++i)
	{
		UT_PUSH_CONTEXT( "iteration" ) << "#" << i;
		size_t how_many_to_insert =
			std::min< size_t >(
				1 + rand_num( block_stream->default_block_count() ),
				block_stream->max_block_count() - model_stream_chain.size() );

		so_5_transport::raw_block_chain_t initial_chain;
		for( int j = 0; j < how_many_to_insert; ++j )
		{
			UT_PUSH_CONTEXT( "simulating writing to blocks" )
				<< " block #" << j;

			so_5_transport::raw_block_t * rb;
			rb = block_stream->occupy();

			UT_CHECK_CONDITION( 0 != rb );

			// Блок должен быть готов к записи.
			UT_CHECK_EQ( 0, rb->read_pos() );

			UT_CHECK_EQ( 0, rb->write_pos() );

			const size_t single_block_size = block_stream->single_block_size();
			// Для записи должно быть доступно
			// single_block_size байт, а для чтения
			// ничего не должно быть доступно.
			UT_CHECK_EQ( single_block_size, rb->write_size_available() );
			UT_CHECK_EQ( 0, rb->read_size_available() );

			// Как будто что-то туда записали.
			rb->total_shift_write_pos();

			// Теперь наоборот.
			UT_CHECK_EQ( 0, rb->write_size_available() );
			UT_CHECK_EQ( single_block_size, rb->read_size_available() );

			initial_chain.push_back( rb );
			UT_POP_CONTEXT();
		}

		if( 0 == rand_num( 2 ) )
		{
			block_stream->stream_push_back( initial_chain );

			// вставляем в модельный образец.
			model_stream_chain.insert(
				model_stream_chain.end(),
				initial_chain.begin(),
				initial_chain.end() );
		}
		else
		{
			block_stream->stream_push_front( initial_chain );

			// вставляем в модельный образец.
			model_stream_chain.insert(
				model_stream_chain.begin(),
				initial_chain.begin(),
				initial_chain.end() );
		}
		initial_chain.clear();

		// Теперь в потоке должны быть элементы.
		UT_CHECK_CONDITION( true == block_stream->check_stream_has_blocks() );


		size_t how_many_to_take =
			1 + rand_num( block_stream->default_block_count() );

		so_5_transport::raw_block_chain_t block_chain;
		size_t obtained_size = block_stream->stream_query_n_blocks(
			how_many_to_take,
			block_chain );

		UT_CHECK_GE( how_many_to_take, block_chain.size() );

		// Получено должно быть столько байт сколько в одном блоке
		// помноженное на количество блоков.
		UT_CHECK_EQ(
			obtained_size,
			block_stream->single_block_size() * block_chain.size() );

		UT_CHECK_CONDITION(
			std::equal(
				block_chain.begin(),
				block_chain.end(),
				model_stream_chain.begin() ) );

		size_t pop_cnt = block_chain.size();
		while( pop_cnt-- ) model_stream_chain.pop_front();

		UT_CHECK_EQ(
			model_stream_chain.size() > 0,
			block_stream->check_stream_has_blocks() );

		// Как-бы  все прочитали.
		std::for_each(
			block_chain.begin(),
			block_chain.end(),
			read_block );

		// Отдаем блоки обратно.
		block_stream->release( block_chain );

		UT_POP_CONTEXT();
	}
	UT_POP_CONTEXT();
}

UT_UNIT_TEST( simple_params )
{
	for( size_t i = 1; i < (1<<8); i <<= 1 )
		for( size_t j = i; j < (1<<8); j <<= 1 )
		{
			so_5_transport::scattered_block_stream_ref_t
				block_stream = create_scatered_block_stream_ctor1( i, j, 64 );

			UT_PUSH_CONTEXT( "Scattered block stream ctor 1" )
				<< *block_stream;

			simple_working_circle( block_stream, UT_CONTEXT );
			gready_working_circle( block_stream, UT_CONTEXT );
			random_working_circle( block_stream, UT_CONTEXT );

			UT_POP_CONTEXT();

			block_stream = create_scatered_block_stream_ctor2( i, j, 64 );

			UT_PUSH_CONTEXT( "Scattered block stream ctor 2" )
				<< *block_stream;

			simple_working_circle( block_stream, UT_CONTEXT );
			gready_working_circle( block_stream, UT_CONTEXT );
			random_working_circle( block_stream, UT_CONTEXT );

			UT_POP_CONTEXT();
		}
};

UT_UNIT_TEST( random_params )
{
	for( int i = 0; i < 20; ++i )
	{
		size_t block_count = 1 + rand_num( 32 );
		size_t max_block_count = block_count + rand_num(32);
		size_t single_block_size = 64;

		so_5_transport::scattered_block_stream_ref_t block_stream =
			create_scatered_block_stream_ctor1(
				block_count,
				max_block_count,
				single_block_size );

		UT_PUSH_CONTEXT( "Scattered block stream params ctor 1" )
			<< *block_stream;

		simple_working_circle( block_stream, UT_CONTEXT );
		gready_working_circle( block_stream, UT_CONTEXT );
		random_working_circle( block_stream, UT_CONTEXT );

		UT_POP_CONTEXT();

		block_stream = create_scatered_block_stream_ctor2(
			block_count,
			max_block_count,
			single_block_size );

		UT_PUSH_CONTEXT( "Scattered block stream params ctor 2" )
			<< *block_stream;

		simple_working_circle( block_stream, UT_CONTEXT );
		gready_working_circle( block_stream, UT_CONTEXT );
		random_working_circle( block_stream, UT_CONTEXT );

		UT_POP_CONTEXT();
	}
};

UT_UNIT_TEST( check_no_free_blocks )
{
	{
		so_5_transport::scattered_block_stream_ref_t block_stream =
			create_scatered_block_stream_ctor1(
				1, 1, 64 );

		UT_PUSH_CONTEXT( "Scattered block stream params ctor1" )
			<< *block_stream;

		so_5_transport::raw_block_t * rb;
		rb = block_stream->occupy();
		UT_CHECK_CONDITION( 0 != rb );

		so_5_transport::raw_block_t * rb2;
		rb2 = block_stream->occupy();
		UT_CHECK_CONDITION( 0 == rb2 );

		block_stream->release( rb );

		rb2 = block_stream->occupy();
		UT_CHECK_CONDITION( 0 != rb );
		block_stream->release( rb2 );

		UT_POP_CONTEXT();
	}
	{
		so_5_transport::scattered_block_stream_ref_t block_stream =
			create_scatered_block_stream_ctor2(
				1, 1, 64 );

		UT_PUSH_CONTEXT( "Scattered block stream params ctor1" )
			<< *block_stream;

		so_5_transport::raw_block_t * rb;
		rb = block_stream->occupy();
		UT_CHECK_CONDITION( 0 != rb );

		so_5_transport::raw_block_t * rb2;
		rb2 = block_stream->occupy();
		UT_CHECK_CONDITION( 0 == rb2 );

		block_stream->release( rb );

		rb2 = block_stream->occupy();
		UT_CHECK_CONDITION( 0 != rb );
		block_stream->release( rb2 );

		UT_POP_CONTEXT();
	}

	{
		so_5_transport::scattered_block_stream_ref_t block_stream =
			create_scatered_block_stream_ctor1(
				10, 100, 64 );

		UT_PUSH_CONTEXT( "Scattered block stream params ctor1" )
			<< *block_stream;

		so_5_transport::raw_block_chain_t initial_chain;

		for( int j = 0; j < block_stream->max_block_count(); ++j )
		{
			so_5_transport::raw_block_t * rb;
			rb = block_stream->occupy();

			UT_CHECK_CONDITION( 0 != rb );
			initial_chain.push_back( rb );
		}

		so_5_transport::raw_block_t * rb;
		rb = block_stream->occupy();
		UT_CHECK_CONDITION( 0 == rb );

		block_stream->release( initial_chain );

		UT_POP_CONTEXT();
	}
	{
		so_5_transport::scattered_block_stream_ref_t block_stream =
			create_scatered_block_stream_ctor2(
				10, 100, 64 );

		UT_PUSH_CONTEXT( "Scattered block stream params ctor2" )
			<< *block_stream;

		so_5_transport::raw_block_chain_t initial_chain;

		for( int j = 0; j < block_stream->max_block_count(); ++j )
		{
			so_5_transport::raw_block_t * rb;
			rb = block_stream->occupy();

			UT_CHECK_CONDITION( 0 != rb );
			initial_chain.push_back( rb );
		}

		so_5_transport::raw_block_t * rb;
		rb = block_stream->occupy();
		UT_CHECK_CONDITION( 0 == rb );

		block_stream->release( initial_chain );

		UT_POP_CONTEXT();
	}
}

int
main( int argc, char ** argv)
{
	srand( time( 0 ) );
	UT_RUN_UNIT_TEST( simple_params );
	UT_RUN_UNIT_TEST( random_params );
	UT_RUN_UNIT_TEST( check_no_free_blocks );

	return 0;
}
