cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

if(NOT DEFINED PROJECT_NAME)
    set(E2D_BUILD_AS_STANDALONE ON)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14"
        CACHE STRING "Minimum OS X version to target for deployment" FORCE)
endif()

project(enduro2d)

set(E2D_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(E2D_ROOT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT E2D_BUILD_AS_STANDALONE)
    set(E2D_SYSTEM_NAME ${E2D_SYSTEM_NAME} PARENT_SCOPE)
    set(E2D_ROOT_DIRECTORY ${E2D_ROOT_DIRECTORY} PARENT_SCOPE)
endif()

set(E2D_ALL_C_CONFIGS "CMAKE_C_FLAGS")
set(E2D_ALL_CXX_CONFIGS "CMAKE_CXX_FLAGS")

if(CMAKE_BUILD_TYPE)
    string(TOUPPER ${CMAKE_BUILD_TYPE} build_type)
    set(E2D_ALL_C_CONFIGS "${E2D_ALL_C_CONFIGS}" "CMAKE_C_FLAGS_${build_type}")
    set(E2D_ALL_CXX_CONFIGS "${E2D_ALL_CXX_CONFIGS}" "CMAKE_CXX_FLAGS_${build_type}")
endif()

foreach(type ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${type} build_type)
    set(E2D_ALL_C_CONFIGS "${E2D_ALL_C_CONFIGS}" "CMAKE_C_FLAGS_${build_type}")
    set(E2D_ALL_CXX_CONFIGS "${E2D_ALL_CXX_CONFIGS}" "CMAKE_CXX_FLAGS_${build_type}")
endforeach()

#
# linking mode
#

if(MSVC)
    option(E2D_BUILD_WITH_STATIC_CRT "Use static C runtime library" ON)
    if(E2D_BUILD_WITH_STATIC_CRT)
        foreach(flag ${E2D_ALL_C_CONFIGS} ${E2D_ALL_CXX_CONFIGS})
            if(${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if(${flag} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()
        endforeach()
    endif()
endif(MSVC)

#
# compilation mode
#

set(E2D_SHARED_CXX_FLAGS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(E2D_SHARED_CXX_FLAGS -Wloop-analysis
                             -Wpointer-arith
                             -Wpessimizing-move
                             -Wuninitialized
                             -Werror=init-self
                             -Werror=self-assign
                             -Werror=self-move
                             -Werror=infinite-recursion
                             -Werror=dangling-else
                             -Werror=large-by-value-copy
                             -Werror=instantiation-after-specialization
                             -Werror=array-bounds
                             -Werror=missing-field-initializers
                             -Werror=user-defined-literals
                             -Werror=parentheses
                             -Werror=address
                             -Werror=return-stack-address)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(E2D_SHARED_CXX_FLAGS
        /we4297   # function assumed not to throw an exception but does
        /we4715   # not all control paths return a value
        /we4716   # must return a value
        /we4172   # returning address of local variable or temporary
        /we4717   # recursive on all control paths, function will cause runtime stack overflow
        /we4239   # conversion from 'type' to 'type &'
        /we4238   # class rvalue used as lvalue
        /we4700   # uninitialized local variable used
        /we4706   # assignment within conditional expression
        /we4554   # check operator precedence for possible error; use parentheses to clarify precedence
        /w14100   # unreferenced formal parameter
        /w14189   # local variable is initialized but not referenced
        /we4456)  # declaration of 'var' hides previous local declaration
endif()
string(REPLACE ";" " " E2D_SHARED_CXX_FLAGS "${E2D_SHARED_CXX_FLAGS}")

foreach(config ${E2D_ALL_CXX_CONFIGS})
    set(${config} "${${config}} ${E2D_SHARED_CXX_FLAGS}")
endforeach()

#
# coverage mode
#

option(E2D_BUILD_WITH_COVERAGE "Build with coverage" OFF)
if(E2D_BUILD_WITH_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_definitions(-DE2D_BUILD_WITH_COVERAGE)
    set(E2D_COVERAGE_FLAGS "--coverage")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${E2D_COVERAGE_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${E2D_COVERAGE_FLAGS}")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${E2D_COVERAGE_FLAGS}")
endif()

#
# sanitizer modes
#

option(E2D_BUILD_WITH_ASAN "Build with address sanitizer" OFF)
if(E2D_BUILD_WITH_ASAN AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_definitions(-DE2D_BUILD_WITH_ASAN)
    set(E2D_SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${E2D_SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${E2D_SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${E2D_SANITIZER_FLAGS}")
endif()

option(E2D_BUILD_WITH_UBSAN "Build with undefined sanitizer" OFF)
if(E2D_BUILD_WITH_UBSAN AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    add_definitions(-DE2D_BUILD_WITH_UBSAN)
    set(E2D_SANITIZER_FLAGS "-fsanitize=undefined")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${E2D_SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${E2D_SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${E2D_SANITIZER_FLAGS}")
endif()

#
# e2d sources
#

file(GLOB_RECURSE E2D_HEADERS
    headers/enduro2d/*.hpp
    headers/enduro2d/*.inl)

file(GLOB_RECURSE E2D_SOURCES
    sources/enduro2d/*.cpp
    sources/enduro2d/*.hpp
    headers/enduro2d/*.inl)

if(APPLE)
    file(GLOB_RECURSE E2D_SOURCES_MM
        sources/enduro2d/*.mm)
    list(APPEND E2D_SOURCES
        ${E2D_SOURCES_MM})
endif()

#
# e2d internal 3rd party
#

file(GLOB_RECURSE E2D_3RDPARTY
    headers/3rdparty/*.*
    sources/3rdparty/*.*)

#
# e2d external 3rd party
#

set(USE_STATIC_CRT ${E2D_BUILD_WITH_STATIC_CRT} CACHE INTERNAL "" FORCE)
set(USE_SYSTEM_CURL OFF CACHE INTERNAL "" FORCE)
set(USE_EMBEDDED_CURL ON CACHE INTERNAL "" FORCE)

add_subdirectory(modules/curly.hpp)
set_target_properties(curly.hpp libcurl PROPERTIES FOLDER modules)

add_subdirectory(modules/ecs.hpp)
add_subdirectory(modules/flat.hpp)
add_subdirectory(modules/promise.hpp)

set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)

add_subdirectory(modules/glfw)
set_target_properties(glfw PROPERTIES FOLDER modules)

set(glew-cmake_BUILD_SHARED OFF CACHE INTERNAL "" FORCE)
set(glew-cmake_BUILD_MULTI_CONTEXT OFF CACHE INTERNAL "" FORCE)

add_subdirectory(modules/glew)
set_target_properties(libglew_static PROPERTIES FOLDER modules)

add_subdirectory(modules/spine/spine-c)
set_target_properties(spine-c PROPERTIES FOLDER modules)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        add_library(bass SHARED IMPORTED)
        set_target_properties(bass PROPERTIES IMPORTED_IMPLIB
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/linux/x86/libbass.so)
        set_target_properties(bass PROPERTIES IMPORTED_LOCATION
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/linux/x86/libbass.so)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_library(bass SHARED IMPORTED)
        set_target_properties(bass PROPERTIES IMPORTED_IMPLIB
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/linux/x64/libbass.so)
        set_target_properties(bass PROPERTIES IMPORTED_LOCATION
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/linux/x64/libbass.so)
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_library(bass SHARED IMPORTED)
    set_target_properties(bass PROPERTIES IMPORTED_LOCATION
        ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/macosx/libbass.dylib)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        add_library(bass SHARED IMPORTED)
        set_target_properties(bass PROPERTIES IMPORTED_IMPLIB
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/windows/x86/bass.lib)
        set_target_properties(bass PROPERTIES IMPORTED_LOCATION
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/windows/x86/bass.dll)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_library(bass SHARED IMPORTED)
        set_target_properties(bass PROPERTIES IMPORTED_IMPLIB
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/windows/x64/bass.lib)
        set_target_properties(bass PROPERTIES IMPORTED_LOCATION
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/windows/x64/bass.dll)
    endif()
endif()

function(add_e2d_shared_libraries_to_target TO)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            add_custom_command(TARGET ${TO} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/linux/x86/libbass.so
                $<TARGET_FILE_DIR:${TO}>)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_custom_command(TARGET ${TO} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/linux/x64/libbass.so
                $<TARGET_FILE_DIR:${TO}>)
        endif()
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        add_custom_command(TARGET ${TO} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/macosx/libbass.dylib
            $<TARGET_FILE_DIR:${TO}>)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            add_custom_command(TARGET ${TO} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/windows/x86/bass.dll
                $<TARGET_FILE_DIR:${TO}>)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_custom_command(TARGET ${TO} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${E2D_ROOT_DIRECTORY}/sources/3rdparty/bass/windows/x64/bass.dll
                $<TARGET_FILE_DIR:${TO}>)
        endif()
    endif()
endfunction(add_e2d_shared_libraries_to_target)

#
# e2d library target
#

add_library(${PROJECT_NAME} STATIC
    ${E2D_HEADERS}
    ${E2D_SOURCES}
    ${E2D_3RDPARTY})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${E2D_HEADERS}
    ${E2D_SOURCES}
    ${E2D_3RDPARTY})

target_link_libraries(${PROJECT_NAME}
    PUBLIC curly.hpp
    PUBLIC ecs.hpp
    PUBLIC flat.hpp
    PUBLIC promise.hpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE bass
    PRIVATE glfw
    PRIVATE libglew_static
    PRIVATE spine-c
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:winmm.lib>)

target_include_directories(${PROJECT_NAME}
    PUBLIC headers
    PRIVATE sources)

target_compile_options(${PROJECT_NAME}
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W3 /MP>
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
        -Wall -Wextra -Wpedantic>
    PRIVATE
    $<$<PLATFORM_ID:Darwin>:
        -Wno-deprecated-declarations>)

target_compile_features(${PROJECT_NAME}
    PUBLIC cxx_std_17)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS>)

#
# subdirectories
#

option(E2D_BUILD_SAMPLES "Build samples" ON)
if(E2D_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

option(E2D_BUILD_UNTESTS "Build untests" ON)
if(E2D_BUILD_UNTESTS)
    enable_testing()
    add_subdirectory(untests)
endif()
