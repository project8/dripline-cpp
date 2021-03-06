# CMakeLists.txt for dripline/cpp/source
# Author: N. Oblath
# Created: Jan 5, 2016

##########

set( dir ${CMAKE_CURRENT_SOURCE_DIR} )

set( dripline_HEADERS
    ${dir}/agent.hh
    ${dir}/agent_config.hh
    ${dir}/amqp.hh
    ${dir}/core.hh
    ${dir}/dripline_api.hh
    ${dir}/dripline_constants.hh
    ${dir}/dripline_error.hh
    ${dir}/dripline_version.hh
    ${dir}/hub.hh
    ${dir}/message.hh
    ${dir}/relayer.hh
    ${dir}/reply_package.hh
    ${dir}/routing_key_specifier.hh
    ${dir}/service.hh
    ${dir}/uuid.hh
)

set( dripline_SOURCES
    ${dir}/agent.cc
    ${dir}/agent_config.cc
    ${dir}/core.cc
    ${dir}/dripline_constants.cc
    ${dir}/dripline_error.cc
    ${dir}/dripline_version.cc
    ${dir}/hub.cc
    ${dir}/message.cc
    ${dir}/relayer.cc
    ${dir}/reply_package.cc
    ${dir}/routing_key_specifier.cc
    ${dir}/service.cc
    ${dir}/uuid.cc
)

configure_file( ${dir}/dripline_version.cc.in ${CMAKE_CURRENT_BINARY_DIR}/dripline_version.cc )
set( dripline_SOURCES
    ${dripline_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/dripline_version.cc
)


##########

pbuilder_library( Dripline dripline_SOURCES "" )

pbuilder_install_headers( ${dripline_HEADERS} )

if( Dripline_ENABLE_EXECUTABLES )
    set( lib_dependencies 
        Dripline
    )
    set( programs
        dl_agent
    )
    pbuilder_executables( programs lib_dependencies )
endif( Dripline_ENABLE_EXECUTABLES )
