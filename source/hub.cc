/*
 * hub.cc
 *
 *  Created on: Jan 7, 2016
 *      Author: nsoblath
 */

#include "hub.hh"

#include "logger.hh"
#include "parsable.hh"

using scarab::parsable;

using std::string;

namespace dripline
{

    LOGGER( dlog, "hub" );


    hub::hub() :
            service(),
            f_lockout_tag(),
            f_lockout_key( generate_nil_uuid() )
    {}

    hub::hub( const string& a_address, unsigned a_port, const string& a_exchange, const string& a_queue_name, const string& a_auth_file ) :
            service( a_address, a_port, a_exchange, a_queue_name, a_auth_file),
            f_lockout_tag(),
            f_lockout_key( generate_nil_uuid() )
    {
        if( ! a_queue_name.empty() )
        {
            f_keys.insert( a_queue_name + string( ".#" ) );
        }
    }

    hub::~hub()
    {
    }

    bool hub::dripline_setup( const string& a_address, unsigned a_port, const string& a_exchange, const string& a_queue_name, const string& a_auth_file )
    {
        f_address = a_address;
        f_port = a_port;
        f_exchange = a_exchange;
        f_queue_name = a_queue_name;
        f_keys.clear();
        if( ! f_queue_name.empty() )
        {
            f_keys.insert( f_queue_name + string( ".#" ) );
        }
        if( ! a_auth_file.empty() )
        {
            if( ! use_auth_file( a_auth_file ) )
            {
                LERROR( dlog, "Unable to use authentication file <" << a_auth_file << ">" );
                return false;
            }
        }
        else
        {
            f_username = "guest";
            f_password = "guest";
        }
        return true;
    }

    bool hub::on_request_message( const request_ptr_t a_request )
    {
        reply_package t_reply_pkg( this, a_request );

        // the lockout key must be valid
        if( ! a_request->get_lockout_key_valid() )
        {
            LWARN( dlog, "Message had an invalid lockout key" );
            return t_reply_pkg.send_reply( retcode_t::message_error_invalid_key, "Lockout key could not be parsed" );;
        }
        else
        {
            switch( a_request->get_message_op() )
            {
                case op_t::run:
                {
                    return __do_run_request( a_request, t_reply_pkg );
                    break;
                }
                case op_t::get:
                {
                    return __do_get_request( a_request, t_reply_pkg );
                    break;
                } // end "get" operation
                case op_t::set:
                {
                    return __do_set_request( a_request, t_reply_pkg );
                    break;
                } // end "set" operation
                case op_t::cmd:
                {
                    return __do_cmd_request( a_request, t_reply_pkg );
                    break;
                }
                default:
                    std::stringstream t_error_stream;
                    t_error_stream << "Unrecognized message operation: <" << a_request->get_message_type() << ">";
                    string t_error_msg( t_error_stream.str() );
                    LWARN( dlog, t_error_msg );
                    return t_reply_pkg.send_reply( retcode_t::message_error_invalid_method, t_error_msg );;
                    break;
            } // end switch on message type
        }
        return false;
    }

    bool hub::do_run_request( const request_ptr_t, reply_package& )
    {
        LWARN( dlog, "This hub does not accept run requests" );
        return false;
    }

    bool hub::do_get_request( const request_ptr_t, reply_package& )
    {
        LWARN( dlog, "This hub does not accept get requests other than the basic dripline instructions" );
        return false;
    }

    bool hub::do_set_request( const request_ptr_t, reply_package& )
    {
        LWARN( dlog, "This hub does not accept set requests" );
        return false;
    }

    bool hub::do_cmd_request( const request_ptr_t, reply_package& )
    {
        LWARN( dlog, "This hub does not accept cmd requests other than the basic dripline instructions" );
        return false;
    }

    bool hub::__do_run_request( const request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        LDEBUG( dlog, "Run operation request received" );

        if( ! authenticate( a_request->lockout_key() ) )
        {
            std::stringstream t_conv;
            t_conv << a_request->lockout_key();
            string t_message( "Request denied due to lockout (key used: " + t_conv.str() + ")" );
            LINFO( dlog, t_message );
            return a_reply_pkg.send_reply( retcode_t::message_error_access_denied, t_message );;
        }

        return do_run_request( a_request, a_reply_pkg );
    }

    bool hub::__do_get_request( request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        LDEBUG( dlog, "Get operation request received" );

        string t_query_type;
        if( ! a_request->parsed_rks().empty() )
        {
            t_query_type = a_request->parsed_rks().front();
        }

        if( t_query_type == "is-locked" )
        {
            a_request->parsed_rks().pop_front();
            return handle_is_locked_request( a_request, a_reply_pkg );
        }

        return do_get_request( a_request, a_reply_pkg );
    }

    bool hub::__do_set_request( const request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        LDEBUG( dlog, "Set request received" );

        if( ! authenticate( a_request->lockout_key() ) )
        {
            std::stringstream t_conv;
            t_conv << a_request->lockout_key();
            string t_message( "Request denied due to lockout (key used: " + t_conv.str() + ")" );
            LINFO( dlog, t_message );
            return a_reply_pkg.send_reply( retcode_t::message_error_access_denied, t_message );;
        }

        return do_set_request( a_request, a_reply_pkg );
    }

    bool hub::__do_cmd_request( const request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        LDEBUG( dlog, "Cmd request received" );

        string t_instruction;
        if( ! a_request->parsed_rks().empty() )
        {
            t_instruction = a_request->parsed_rks().front();
        }

        //LWARN( mtlog, "uuid string: " << a_request->get_payload().get_value( "key", "") << ", uuid: " << uuid_from_string( a_request->get_payload().get_value( "key", "") ) );
        // this condition includes the exception for the unlock instruction that allows us to force the unlock regardless of the key.
        // disable_key() checks the lockout key if it's not forced, so it's okay that we bypass this call to authenticate() for the unlock instruction.
        if( ! authenticate( a_request->lockout_key() ) && t_instruction != "unlock" && t_instruction != "ping" )
        {
            std::stringstream t_conv;
            t_conv << a_request->lockout_key();
            string t_message( "Request denied due to lockout (key used: " + t_conv.str() + ")" );
            LINFO( dlog, t_message );
            return a_reply_pkg.send_reply( retcode_t::message_error_access_denied, t_message );;
        }

        if( t_instruction == "lock" )
        {
            a_request->parsed_rks().pop_front();
            return handle_lock_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "unlock" )
        {
            a_request->parsed_rks().pop_front();
            return handle_unlock_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "ping" )
        {
            a_request->parsed_rks().pop_front();
            return handle_ping_request( a_request, a_reply_pkg );
        }

        return do_cmd_request( a_request, a_reply_pkg );
    }

    bool hub::handle_lock_request( const request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        uuid_t t_new_key = enable_lockout( a_request->get_sender_info(), a_request->lockout_key() );
        if( t_new_key.is_nil() )
        {
            return a_reply_pkg.send_reply( retcode_t::device_error, "Unable to lock server" );;
        }

        a_reply_pkg.f_payload.add( "key", new scarab::param_value( string_from_uuid( t_new_key ) ) );
        return a_reply_pkg.send_reply( retcode_t::success, "Server is now locked" );
    }

    bool hub::handle_unlock_request( const request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        if( ! is_locked() )
        {
            return a_reply_pkg.send_reply( retcode_t::warning_no_action_taken, "Already unlocked" );
        }

        bool t_force = a_request->get_payload().get_value( "force", false );

        if( disable_lockout( a_request->lockout_key(), t_force ) )
        {
            return a_reply_pkg.send_reply( retcode_t::success, "Server unlocked" );
        }
        return a_reply_pkg.send_reply( retcode_t::device_error, "Failed to unlock server" );;
    }

    bool hub::handle_is_locked_request( const request_ptr_t, hub::reply_package& a_reply_pkg )
    {
        bool t_is_locked = is_locked();
        a_reply_pkg.f_payload.add( "is_locked", scarab::param_value( t_is_locked ) );
        if( t_is_locked ) a_reply_pkg.f_payload.add( "tag", f_lockout_tag );
        return a_reply_pkg.send_reply( retcode_t::success, "Checked lock status" );
    }

    bool hub::handle_ping_request( const request_ptr_t a_request, hub::reply_package& a_reply_pkg )
    {
        string t_sender = a_request->sender_package();
        return a_reply_pkg.send_reply( retcode_t::success, "Hello, " + t_sender );
    }


    bool hub::reply_package::send_reply( retcode_t a_return_code, const std::string& a_return_msg ) const
    {
        if( f_service_ptr == nullptr )
        {
            LWARN( dlog, "Service pointer is null; Unable to send reply" );
            return false;
        }

        reply_ptr_t t_reply = msg_reply::create( a_return_code, a_return_msg, new scarab::param_node( f_payload ), f_reply_to, message::encoding::json );
        t_reply->correlation_id() = f_correlation_id;

        LDEBUG( dlog, "Sending reply message to <" << f_reply_to << ">:\n" <<
                 "Return code: " << t_reply->get_return_code() << '\n' <<
                 "Return message: " << t_reply->return_msg() <<
                 f_payload );

        if( ! f_service_ptr->send( t_reply ) )
        {
            LWARN( dlog, "Something went wrong while sending the reply" );
            return false;
        }

        return true;
    }

    uuid_t hub::enable_lockout( const scarab::param_node& a_tag, uuid_t a_key )
    {
        if( is_locked() ) return generate_nil_uuid();
        if( a_key.is_nil() ) f_lockout_key = generate_random_uuid();
        else f_lockout_key = a_key;
        f_lockout_tag = a_tag;
        return f_lockout_key;
    }

    bool hub::disable_lockout( const uuid_t& a_key, bool a_force )
    {
        if( ! is_locked() ) return true;
        if( ! a_force && a_key != f_lockout_key ) return false;
        f_lockout_key = generate_nil_uuid();
        f_lockout_tag.clear();
        return true;
    }

    bool hub::authenticate( const uuid_t& a_key ) const
    {
        LDEBUG( dlog, "Authenticating with key <" << a_key << ">" );
        if( is_locked() ) return check_key( a_key );
        return true;
    }


} /* namespace dripline */
