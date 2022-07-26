include(${CMAKE_TOOL_DIR}/add_tool_module.cmake)
include(${CMAKE_TOOL_DIR}/module_test.cmake)

find_package(Boost 1.75	REQUIRED)

add_component(
    MODULE_NAME
        http
    NAMESPACE
        ${PROJECT_NAMESPACE}
    INTERFACE_INCLUDE_DIRS
        include
    PUBLIC_INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}
    SOURCE_FILES
        src/connect.cpp
        src/llhttp/api.c
        src/llhttp/http.c
        src/llhttp/llhttp.c
    PUBLIC_LIBRARIES
        ${Boost_LIBRARIES}
)

module_test(
    MODULE_NAME
        http
    NAMESPACE
        ${PROJECT_NAMESPACE}
    SOURCE_FILES
        test/main.cpp
)