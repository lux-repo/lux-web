# INCLUDE GUARD
if(_MODULE_TEST_INCLUDED_)
	return()
endif()
set(_MODULE_TEST_INCLUDED_ TRUE)


function(module_test)
	set(_options	  			)
	set(_one_value_arguments	MODULE_NAME  NAMESPACE)
	set(_multi_value_arguments	SOURCE_FILES)
	
	cmake_parse_arguments(
		MODULE_TEST_ARGS
		"${_options}"
		"${_one_value_arguments}"
		"${_multi_value_arguments}"
		${ARGN}
	)

	# check module name
	if(NOT MODULE_TEST_ARGS_MODULE_NAME)
		message(FATAL_ERROR "Module name not specified.")
	endif()

	# check namespace
	if(NOT MODULE_TEST_ARGS_NAMESPACE)
		message(FATAL_ERROR "Module namespace not specified.")
	endif()

	set(TEST_TARGET_NAME module_test_${MODULE_TEST_ARGS_MODULE_NAME})
		
	add_executable(
		${TEST_TARGET_NAME}
		${MODULE_TEST_ARGS_SOURCE_FILES}
	)

	target_link_libraries(
		${TEST_TARGET_NAME}
		${MODULE_TEST_ARGS_NAMESPACE}::${MODULE_TEST_ARGS_MODULE_NAME}
	)
endfunction()