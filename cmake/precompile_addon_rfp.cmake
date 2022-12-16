find_program(UNIFDEF unifdef REQUIRED)
if(NOT UNIFDEF)
    message(FATAL_ERROR "unifdef not found!")
endif()
find_program(SPLINT splint REQUIRED)
if(NOT SPLINT)
    message(FATAL_ERROR "splint not found!")
endif()

find_program(UNIFDEF unifdef REQUIRED)
if(NOT UNIFDEF)
    message(FATAL_ERROR "unifdef not found!")
endif()
find_program(SPLINT splint REQUIRED)
if(NOT SPLINT)
    message(FATAL_ERROR "splint not found!")
endif()


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
	COMMAND	${CMAKE_COMMAND} -E make_directory ${PRECOMPIL_DIR}/src/tests_mode  ${PRECOMPIL_DIR}/inc/tests_mode
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
