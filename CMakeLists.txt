cmake_minimum_required(VERSION 3.19)
project(DDGI_DCCpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -m32)

include_directories(.)
add_definitions(-D_WIN32_WINNT=0x0501)

string(TIMESTAMP date V%d%m)

add_library(DDGI_DCCpp SHARED DDGI_DCCpp.cpp DDGI_Generic_def.h DDGI_Generic_lib_def.h DCCpp_commands.cpp DCCpp_commands.h DCCpp_utils.cpp DCCpp_utils.h DCCpp.cpp DCCpp.h dialogs/rsrc.rc dialogs/DCCpp_dlg_definitions.h websockets/wsclient.cpp websockets/wsclient.h)

target_link_libraries(DDGI_DCCpp wsock32 ws2_32)

if (CMAKE_BUILD_TYPE MATCHES Release)
    target_link_libraries(DDGI_DCCpp -static stdc++)
    set_target_properties(DDGI_DCCpp PROPERTIES PREFIX "" OUTPUT_NAME "DDGI_6DCCpp" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../../../Github/CDM-Rail/CDM_DGI")
else()
    set_target_properties(DDGI_DCCpp PROPERTIES PREFIX "" OUTPUT_NAME "DDGI_7DCCpp${date}" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../../../Github/CDM-Rail/CDM_DGI")
endif()