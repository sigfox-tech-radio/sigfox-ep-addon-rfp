################################################################################
#
# Copyright (c) 2024, UnaBiz SAS
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1 Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  2 Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  3 Neither the name of UnaBiz SAS nor the names of its contributors may be
#    used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
# THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
################################################################################

cmake_minimum_required(VERSION 3.15)
project(sigfox_ep_addon_rfp)

#Check if Project configure by himself or by a parent project (like sigfox-ep-lib)
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR} )
    if (NOT DEFINED  SIGFOX_EP_LIB_DIR)
        message(FATAL_ERROR "${PROJECT_NAME} cannot be configured without -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib directory>")
    endif()
    set(ADDON_RFP OFF)
    add_subdirectory(${SIGFOX_EP_LIB_DIR} "${CMAKE_CURRENT_BINARY_DIR}/sigfox-ep-lib")
else()
    if (NOT DEFINED SIGFOX_EP_LIB_DIR)
        set(SIGFOX_EP_LIB_DIR ${CMAKE_SOURCE_DIR})
        message("${PROJECT_NAME} is Fetched by a parent project")
    endif()
endif()

set(ADDON_RFP_SOURCES
    src/sigfox_ep_addon_rfp_api.c
    src/test_modes_rfp/sigfox_rfp_test_mode_a.c
    src/test_modes_rfp/sigfox_rfp_test_mode_b.c
    src/test_modes_rfp/sigfox_rfp_test_mode_c.c
    src/test_modes_rfp/sigfox_rfp_test_mode_d.c
    src/test_modes_rfp/sigfox_rfp_test_mode_e.c
    src/test_modes_rfp/sigfox_rfp_test_mode_f.c
    src/test_modes_rfp/sigfox_rfp_test_mode_g.c
    src/test_modes_rfp/sigfox_rfp_test_mode_j.c
    src/test_modes_rfp/sigfox_rfp_test_mode_k.c
    src/test_modes_rfp/sigfox_rfp_test_mode_l.c
)

set(ADDON_RFP_HEADERS 
    inc/sigfox_ep_addon_rfp_api.h
    inc/sigfox_ep_addon_rfp_version.h
    inc/test_modes_rfp/sigfox_rfp_test_mode_types.h
)

set(ADDON_RFP_PUBLIC_HEADERS
    inc/sigfox_ep_addon_rfp_api.h
    inc/sigfox_ep_addon_rfp_version.h
    inc/test_modes_rfp/sigfox_rfp_test_mode_types.h
)

if(${SIGFOX_EP_CERTIFICATION} STREQUAL "OFF")
    message(FATAL_ERROR "SIGFOX_EP_CERTIFICATION Flag must be activated for this addon")
endif()
if((${SIGFOX_EP_RC3_LBT_ZONE} STREQUAL OFF) AND (${SIGFOX_EP_RC5_ZONE} STREQUAL OFF))
    list(REMOVE_ITEM ADDON_RFP_SOURCES "src/test_modes_rfp/sigfox_rfp_test_mode_g.c")
endif()
if((${SIGFOX_EP_PUBLIC_KEY_CAPABLE} STREQUAL OFF) )
    list(REMOVE_ITEM ADDON_RFP_SOURCES "src/test_modes_rfp/sigfox_rfp_test_mode_k.c")
endif()
if(${SIGFOX_EP_BIDIRECTIONAL} STREQUAL OFF)
    list(REMOVE_ITEM ADDON_RFP_SOURCES "src/test_modes_rfp/sigfox_rfp_test_mode_f.c")
endif()

#Add Cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#Precompile module
find_program(UNIFDEF unifdef)
if(NOT UNIFDEF)
    message(STATUS "unifdef - not found (precompilation target will not be available)")   
else()
    include(precompile_addon_rfp)
endif()

#Target to create object
add_library(${PROJECT_NAME}_obj OBJECT EXCLUDE_FROM_ALL ${ADDON_RFP_SOURCES})
target_include_directories( ${PROJECT_NAME}_obj PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${SIGFOX_EP_LIB_DIR}/inc
    PRIVATE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:sigfox_ep_lib_obj,INTERFACE_INCLUDE_DIRECTORIES>>
)

#Target to create library
add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:sigfox_ep_lib_obj,INTERFACE_INCLUDE_DIRECTORIES>>
    )
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "${ADDON_RFP_PUBLIC_HEADERS}"
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_LOCATION}
    LIBRARY_OUTPUT_DIRECTORY ${LIB_LOCATION}
)
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LIB_LOCATION}
    PUBLIC_HEADER DESTINATION ${API_LOCATION}  
)


