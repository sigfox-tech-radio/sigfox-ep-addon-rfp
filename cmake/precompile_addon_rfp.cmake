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

#List of precompileInc and precompileSrc files
foreach(X IN LISTS ADDON_RFP_SOURCES)
	LIST(APPEND PRECOMPIL_ADDON_RFP_SOURCES "${PRECOMPIL_DIR}/${X}")
endforeach()
foreach(X IN LISTS ADDON_RFP_HEADERS)
	LIST(APPEND PRECOMPIL_ADDON_RFP_HEADERS "${PRECOMPIL_DIR}/${X}")
endforeach()
foreach(X IN LISTS ADDON_RFP_PUBLIC_HEADERS)
	LIST(APPEND PRECOMPIL_ADDON_RFP_PUBLIC_HEADERS "${PRECOMPIL_DIR}/${X}")
endforeach()

#Custom command Loop for all Sources
foreach(X IN LISTS ADDON_RFP_SOURCES ADDON_RFP_HEADERS)
add_custom_command(
	OUTPUT "${PRECOMPIL_DIR}/${X}"
	DEPENDS ${CMAKE_BINARY_DIR}/undefs_file
	DEPENDS ${CMAKE_BINARY_DIR}/defs_file
    DEPENDS ${X}
	COMMAND	${CMAKE_COMMAND} -E make_directory ${PRECOMPIL_DIR}/src/test_modes_rfp  ${PRECOMPIL_DIR}/inc/test_modes_rfp
    COMMAND unifdef -B -k -x 2 -f ${CMAKE_BINARY_DIR}/undefs_file -f ${CMAKE_BINARY_DIR}/defs_file ${PROJECT_SOURCE_DIR}/${X} > "${PRECOMPIL_DIR}/${X}" 
	VERBATIM
)

endforeach()
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

add_custom_target(precompil_${PROJECT_NAME}
	DEPENDS precompil
    DEPENDS ${PRECOMPIL_ADDON_RFP_SOURCES}
    DEPENDS ${PRECOMPIL_ADDON_RFP_HEADERS}
  	VERBATIM
)
