/*
	SObjectizer 5 Transport.
*/

/*!
	\file
	\brief Адаптация ACE_Hash под различные типы.
*/

#if !defined( _SO_5_TRANSPORT__ACE__ACE_HASH_ADAPTATION_HPP_ )
#define _SO_5_TRANSPORT__ACE__ACE_HASH_ADAPTATION_HPP_

#include <string>

#include <cpp_util_2/h/string_piece.hpp>

#include <ace/ACE.h>
#include <ace/Functor_T.h>

namespace so_5_transport
{

// Функция хэширования, взятая из SunriseDD.
//
// --------------------------------------------------------------------------
// Function:  dd_DJB_hash_string(string)
// --------------------------------------------------------------------------
//
// Returns the hash value of the null terminated C string <string>  using the
// DJB hash algorithm  (published by D.J.Bernstein on Usenet in comp.lang.c).
// The number  of  significant characters  for which the  hash value  will be
// calculated    is    limited   to   the   value   returned   by    function
// dd_significant_chars().
//
// Returns 0 if <string> is empty or NULL.
//
inline ACE_UINT32
hash_DJB(const char *string, size_t string_size)
{
	// seed value for DJB hashes
	const ACE_UINT32 DJB_INIT = 5381;
	const ACE_UINT32 DD_SIGNIFICANT_CHARS = 32;

	register ACE_UINT32 index = 0, hash = DJB_INIT;

	if (!string || !string_size)
		return 0;

	while ((index < string_size) && (index < DD_SIGNIFICANT_CHARS)) {
		hash = string[index] + ((hash << 5) + hash);
		index++;
	} // end while

	return (hash & 0x7fffffff);
} // end dd_DJB_hash_string

} /* namespace so_5_transport */

/*!
	\brief Хеш-функция для string.
*/
class std_string_djb_hash_t
{
	public:
		unsigned long
		operator()( const std::string & s ) const
		{
			if( s.empty() )
				return 0;
			return so_5_transport::hash_DJB( s.data(), s.size() );
		}
};

#endif

