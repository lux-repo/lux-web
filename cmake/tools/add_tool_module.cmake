# INCLUDE GUARD
if(_ADD_TOOL_MODULE_INCLUDED_)
	return()
endif()
set(_ADD_TOOL_MODULE_INCLUDED_ TRUE)

function(add_component)
	set(_options	  			SHARED)	
	set(_one_value_arguments	MODULE_NAME NAMESPACE)
	set(_multi_value_arguments	
		SOURCE_FILES 
		INTERFACE_INCLUDE_DIRS
		PUBLIC_INCLUDE_DIRS
		PRIVATE_INCLUDE_DIRS
		DEPENDENCIES 
		PUBLIC_LIBRARIES
		PRIVATE_LIBRARIES
	)

	cmake_parse_arguments(
		MODULE_ARGS
		"${_options}"
		"${_one_value_arguments}"
		"${_multi_value_arguments}"
		${ARGN}
	)

	message("---MODULE NAME:${MODULE_ARGS_MODULE_NAME}")
	if(MODULE_ARGS_DEPENDENCIES)
		message("---DEPENDEMIES:")
		foreach(dep ${MODULE_ARGS_DEPENDENCIES})
			message("------${dep}")
		endforeach()
	endif()
	
	# check module name
	if(NOT MODULE_ARGS_MODULE_NAME)
		message(FATAL_ERROR "Module name not specified.")
	endif()

	# check namespace
	if(NOT MODULE_ARGS_NAMESPACE)
		message(FATAL_ERROR "Module namespace not specified.")
	endif()

	if(MODULE_ARGS_SHARED)
		set(LIBRARY_TYPE	SHARED)
	else()
		set(LIBRARY_TYPE	STATIC)
	endif()

	set(INCLUDE_METHOD  PUBLIC)

	# is header only
	if(NOT MODULE_ARGS_SOURCE_FILES)
		if(NOT MODULE_ARGS_INTERFACE_INCLUDE_DIRS)
			message(FATAL_ERROR "Neither source file nor include include directory is set")
		endif()

		set(LIBRARY_TYPE	INTERFACE)
		set(INCLUDE_METHOD	INTERFACE)
		if(MODULE_ARGS_SHARED)
			message(WARNING "Module is a header only, but shared option has been set")
		endif()
	endif()

	add_library(
		${MODULE_ARGS_MODULE_NAME}
		${LIBRARY_TYPE}
		${MODULE_ARGS_SOURCE_FILES}
	)
	# alias
	set(ALIAS_NAME ${MODULE_ARGS_NAMESPACE}::${MODULE_ARGS_MODULE_NAME})
	message("---ALIAS NAME:${ALIAS_NAME}")
	add_library(
		${ALIAS_NAME}
		ALIAS 
		${MODULE_ARGS_MODULE_NAME}
	)

	set_target_properties(
		${MODULE_ARGS_MODULE_NAME}
		PROPERTIES 
		OUTPUT_NAME lux_web_${MODULE_ARGS_MODULE_NAME}
	)

	target_link_libraries(
		${MODULE_ARGS_MODULE_NAME}
		PUBLIC
			${MODULE_ARGS_PUBLIC_LIBRARIES}
		PRIVATE
			${MODULE_ARGS_PRIVATE_LIBRARIES}
	)
	message(${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_ARGS_INTERFACE_INCLUDE_DIRS})
	target_include_directories(
		${MODULE_ARGS_MODULE_NAME}
		${INCLUDE_METHOD}
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_ARGS_INTERFACE_INCLUDE_DIRS}>
			$<INSTALL_INTERFACE:${MODULE_ARGS_INTERFACE_INCLUDE_DIRS}>
		PUBLIC
			${MODULE_ARGS_PUBLIC_INCLUDE_DIRS}
		PRIVATE
			${MODULE_ARGS_PRIVATE_INCLUDE_DIRS}
	)

	set(lux_DEPENDENCIES)
	if(MODULE_ARGS_DEPENDENCIES)
		foreach(dep ${MODULE_ARGS_DEPENDENCIES})
			list(APPEND lux_DEPENDENCIES lux_${dep})
		endforeach()

		target_link_libraries(
			${MODULE_ARGS_MODULE_NAME}
			${INCLUDE_METHOD}
			${MODULE_ARGS_DEPENDENCIES}
		)
	endif()

	install(
		DIRECTORY	${MODULE_ARGS_INCLUDE_DIRS}
		DESTINATION ${CMAKE_INSTALL_PREFIX}
	)

	install(
		TARGETS	${MODULE_ARGS_MODULE_NAME}
		EXPORT lux::web
	)
endfunction()


