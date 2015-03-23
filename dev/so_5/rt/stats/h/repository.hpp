/*
 * SObjectizer-5
 */

/*!
 * \file
 * \since v.5.5.4
 * \brief Interfaces of data source and data sources repository.
 */

#pragma once

#include <so_5/h/declspec.hpp>

#include <so_5/rt/h/mbox.hpp>

namespace so_5
{

namespace rt
{

namespace stats
{

/*!
 * \since v.5.5.4
 * \brief An interface of data source.
 */
class SO_5_TYPE source_t
	{
		friend class repository_t;

	protected :
		source_t();
		~source_t();

	public :
		//! Send appropriate notification about the current value.
		virtual void
		distribute(
			//! Target mbox for the appropriate message.
			const mbox_t & distribution_mbox ) = 0;

	private :
		//! Previous item in the data sources list.
		source_t * m_prev;
		//! Next item in the data sources list.
		source_t * m_next;
	};

/*!
 * \since v.5.5.4
 * \brief An interface of data sources repository.
 */
class SO_5_TYPE repository_t
	{
	protected :
		~repository_t();

	public :
		//! Registration of new data source.
		/*!
		 * Caller must guarantee that the data source will live until
		 * it is registered in the repository.
		 */
		virtual void
		add( source_t & what ) = 0;

		//! Deregistration of previously registered data source.
		virtual void
		remove( source_t & what ) = 0;
	};

} /* namespace stats */

} /* namespace rt */

} /* namespace so_5 */

