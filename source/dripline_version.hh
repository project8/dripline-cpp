/*
 * version.hh
 *
 *  Generated by cmake from mt_version.hh.in
 *
 *  Created on: Mar 20, 2013
 *      Author: nsoblath
 */

#ifndef DRIPLINE_VERSION_HH_
#define DRIPLINE_VERSION_HH_

#include "singleton.hh"

#include "dripline_api.hh"

#include "scarab_version.hh"

#include <string>



#define set_version( version_namespace, version_class ) \
    static dripline::version_setter s_vsetter_##version_namespace##_##version_class( new version_namespace::version_class() );


namespace dripline
{
    
    class DRIPLINE_API version : public scarab::version_semantic
    {
        public:
            version();
            ~version();
    };


    class DRIPLINE_API version_wrapper : public scarab::singleton< version_wrapper >, public scarab::version_ifc
    {
        protected:
            friend class scarab::singleton< version_wrapper >;
            friend class scarab::destroyer< version_wrapper >;
            version_wrapper();
            ~version_wrapper();

        public:
            void set_imp( scarab::version_semantic* a_imp )
            {
                delete f_imp;
                f_imp = a_imp;
                return;
            }
    
        public:
            virtual unsigned major_version() const
            {
                return f_imp->major_version();
            }
            virtual unsigned minor_version() const
            {
                return f_imp->minor_version();
            }
            virtual unsigned patch_version() const
            {
                return f_imp->patch_version();
            }

            virtual const std::string& version_str() const
            {
                return f_imp->version_str();
            }

            virtual const std::string& package() const
            {
                return f_imp->package();
            }
            virtual const std::string& commit() const
            {
                return f_imp->commit();
            }

            virtual const std::string& exe_name() const
            {
                return f_imp->exe_name();
            }
            virtual const std::string& hostname() const
            {
                return f_imp->hostname();
            }
            virtual const std::string& username() const
            {
                return f_imp->username();
            }

            virtual std::string version_info_string() const
            {
                return f_imp->version_info_string();
            }

        private:
            scarab::version_semantic* f_imp;
    };


    struct DRIPLINE_API version_setter
    {
            version_setter( scarab::version_semantic* a_ver )
            {
                version_wrapper::get_instance()->set_imp( a_ver );
            }
    };

} // namespace dripline

#endif /* DRIPLINE_VERSION_HH_ */
