/*
 * uuid.hh
 *
 *  Created on: Sep 16, 2015
 *      Author: nsoblath
 */

#ifndef DRIPLINE_UUID_HH_
#define DRIPLINE_UUID_HH_

#include "dripline_api.hh"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp> // to allow streaming of uuid_t

#include <string>

// In Windows there's a preprocessor macro called uuid_t that conflicts with this typdef
#ifdef uuid_t
#undef uuid_t
#endif

namespace dripline
{
    typedef boost::uuids::uuid uuid_t;

    uuid_t DRIPLINE_API generate_random_uuid();
    uuid_t DRIPLINE_API generate_nil_uuid();

    uuid_t DRIPLINE_API uuid_from_string( const std::string& a_id_str );
    uuid_t DRIPLINE_API uuid_from_string( const char* a_id_str );

    uuid_t DRIPLINE_API uuid_from_string( const std::string& a_id_str, bool& a_valid_flag );
    uuid_t DRIPLINE_API uuid_from_string( const char* a_id_str, bool& a_valid_flag );

    std::string DRIPLINE_API string_from_uuid( const uuid_t& a_id );

} /* namespace dripline */

#endif /* MT_UUID_HH_ */
