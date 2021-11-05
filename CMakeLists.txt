cmake_minimum_required (VERSION 3.12)

# submodule update
find_package(Git QUIET)
if(GIT_FOUND)
    message(STATUS "Googletest submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}")
    endif()
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")
    message(FATAL_ERROR "Submodules update failed. Please run git submodule update --init")
endif()


# common setup for exe targets (Arrt and Tests)
function(addQtDeployment _TARGET)
    get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(
        TARGET ${_TARGET} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:${_TARGET}>" --no-translations --no-webkit2 --no-system-d3d-compiler --no-virtualkeyboard --no-angle --no-opengl-sw --no-compiler-runtime
        COMMENT "Running windeployqt..."
    )
    add_custom_command(
        TARGET ${_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_REST_NAME}.dll
            ${LIB_BLOB_NAME}.dll
            ${LIB_ARR_BIN}/RemoteRenderingClient.dll
            ${LIB_ARR_BIN}/Microsoft.Holographic.HybridRemoting.dll
            $<TARGET_FILE_DIR:${_TARGET}>
    )
endfunction()

# set variables for nuget libraries

set( CFG $<IF:$<CONFIG:Debug>,DEBUG,RELEASE> )
set( POSTFIX $<$<CONFIG:Debug>:d> )

set(LIB_BLOB_ROOT "${CMAKE_BINARY_DIR}/packages/Microsoft.Azure.Storage.CPP.v140.5.0.0")
set(LIB_BLOB_NAME "${LIB_BLOB_ROOT}/lib/native/v140/x64/${CFG}/wastorage${POSTFIX}")

set(LIB_REST_ROOT "${CMAKE_BINARY_DIR}/packages/cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn.2.9.1")
set(LIB_REST_NAME "${LIB_REST_ROOT}/lib/native/v140/windesktop/msvcstl/dyn/rt-dyn/x64/${CFG}/cpprest140${POSTFIX}_2_9")

if ("$ENV{LIB_ARR_USE_LOCAL}" STREQUAL "true")
    set(LIB_ARR_BIN $<IF:$<CONFIG:Debug>,$ENV{LIB_ARR_BIN_DEBUG},$ENV{LIB_ARR_BIN_RELEASE}>)
    set(LIB_ARR_LIB $<IF:$<CONFIG:Debug>,$ENV{LIB_ARR_LIB_DEBUG},$ENV{LIB_ARR_LIB_RELEASE}>)
    set(LIB_ARR_INCLUDE $ENV{LIB_ARR_INCLUDE})
else()
    set(LIB_ARR_ROOT "${CMAKE_BINARY_DIR}/packages/Microsoft.Azure.RemoteRendering.Cpp.1.0.19")
    set(LIB_ARR_BIN "${LIB_ARR_ROOT}/bin/win_x64/release")
    set(LIB_ARR_LIB "${LIB_ARR_ROOT}/lib/win_x64/release")
    set(LIB_ARR_INCLUDE "${LIB_ARR_ROOT}/include")
endif()

if (NOT "$ENV{NUGET_RESTORE}" STREQUAL "false")
    find_program(NUGET nuget PATHS "$ENV{NUGET_PATH}")
    if(NOT NUGET)
        message(FATAL "CMake could not find the nuget command line tool. Please install it!")
    else()
        # Copy the Nuget config file from source location to the CMake build directory.
        configure_file("packages.config" packages.config COPYONLY)
        # Run Nuget using the .config file to installing any missing dependencies to the build directory.
        execute_process(COMMAND
            ${NUGET} restore packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()

# retrieve version

if(DEFINED ENV{ARRT_VERSION})
    set(ARRT_VERSION $ENV{ARRT_VERSION})
else()
    message(STATUS "Retrieving version from last git tag")
    find_package (Git)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
        OUTPUT_VARIABLE ARRT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if (NOT DEFINED ARRT_VERSION)
    set (ARRT_VERSION 0)
endif()
message("Version = ${ARRT_VERSION}")

set(CMAKE_SUPPRESS_REGENERATION true)

set( CMAKE_CONFIGURATION_TYPES Debug;Release )
set (PLATFORM_SUBDIR "/x64_vs2017_win10/$<IF:$<CONFIG:Debug>,Debug,Dev>")


#################
# ARRT SOLUTION
#
project(Arrt)

# Global options, for all of the projects
if(MSVC)
    add_compile_options( /W4 /wd4505 /wd4068 /MP /bigobj /Zi)
    # Enable pdb generation in release builds.
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-extra-semi -Wno-unused-function -Wno-language-extension-token -Wno-delete-non-abstract-non-virtual-dtor)
endif()
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_EXTENSIONS OFF )
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
# No Windows.h min and max
add_definitions(-DNOMINMAX)
add_definitions(-DARRT_VERSION="${ARRT_VERSION}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#####################
# ArrtModel library 
#
set (TARGET "ArrtModel")

set (SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ArrtModel")

set(DIRS
    "Model"
    "ViewModel"
    "Utils"
)

foreach(DIR IN LISTS DIRS)
	file(
     GLOB_RECURSE NEW_FILES
     "${SOURCE_ROOT}/${DIR}/*.cpp" "${SOURCE_ROOT}/${DIR}/*.h"
    )
    set(SOURCE_LIST ${SOURCE_LIST} ${NEW_FILES})
endforeach()

source_group(TREE "${SOURCE_ROOT}" FILES ${SOURCE_LIST})

add_library(${TARGET} ${SOURCE_LIST})

target_include_directories(${TARGET} PUBLIC "${SOURCE_ROOT}")

find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
target_link_libraries(${TARGET} PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui)

target_link_libraries(${TARGET} PUBLIC "${LIB_BLOB_NAME}.lib")
target_include_directories(${TARGET} PUBLIC "${LIB_BLOB_ROOT}/build/native/include")

target_link_libraries(${TARGET} PUBLIC "${LIB_REST_NAME}.lib")
target_include_directories(${TARGET} PUBLIC "${LIB_REST_ROOT}/build/native/include")

target_link_libraries(${TARGET} PUBLIC "${LIB_ARR_LIB}/RemoteRenderingClient.lib")
target_include_directories(${TARGET} PUBLIC ${LIB_ARR_INCLUDE})

if(DEFINED ENV{EXTRA_ARR_ZONE})
    add_definitions( -DEXTRA_ARR_ZONE=$ENV{EXTRA_ARR_ZONE} )
endif()

target_link_libraries(${TARGET} PUBLIC crypt32.lib)

unset(SOURCE_LIST)

###################
# ARRT executable
#
set (TARGET "Arrt")

set (SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Arrt")
file(
     GLOB SOURCE_LIST 
	 LIST_DIRECTORIES false
     "${SOURCE_ROOT}/*.cpp" "${SOURCE_ROOT}/*.h" "${SOURCE_ROOT}/Resources/*.qrc" "${SOURCE_ROOT}/Resources/*.ico"
    )

# add the resource file only if it's visual c++, just as a temporary workaround to avoid LLVM errors with the VERSIONINFO parsing
if(MSVC)
    file( GLOB NEW_FILES "${SOURCE_ROOT}/Resources/*.rc" )
    set(SOURCE_LIST ${SOURCE_LIST} ${NEW_FILES})
endif()
    
set(DIRS
    "View"
    "ViewUtils"
    "Widgets"
)

foreach(DIR IN LISTS DIRS)
	file(
     GLOB_RECURSE NEW_FILES
     "${SOURCE_ROOT}/${DIR}/*.cpp" "${SOURCE_ROOT}/${DIR}/*.h"
    )
    set(SOURCE_LIST ${SOURCE_LIST} ${NEW_FILES})
endforeach()

source_group(TREE "${SOURCE_ROOT}" FILES ${SOURCE_LIST})

add_executable(${TARGET} WIN32 ${SOURCE_LIST})
addQtDeployment(${TARGET})

target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}")
target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ArrtModel")
target_link_libraries(${TARGET} PUBLIC ArrtModel)
target_link_libraries(${TARGET} PUBLIC d3d11.lib DXGI.lib)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Arrt)

#########
# Tests
#
set( gtest_force_shared_crt ON CACHE BOOL "Use dynamically linked crt" )
add_subdirectory(googletest)
set_target_properties(gmock gmock_main gtest gtest_main PROPERTIES FOLDER "GTest")
set_target_properties(gmock gmock_main gtest gtest_main PROPERTIES COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/tests)
enable_testing()

# find all of the *.cpp files in ./Tests and create a test exe for each one of them

#file(GLOB_RECURSE TEST_LIST Tests/Model/*.cpp)
#foreach(case_file ${TEST_LIST})
#    get_filename_component( case_name ${case_file} NAME_WE )
#    set (case_name test_${case_name})
#    add_executable(${case_name} ${case_file})
#    addQtDeployment(${case_name})
#    target_include_directories(${case_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ArrtModel")
#    target_link_libraries(${case_name} PUBLIC ArrtModel)
#    target_link_libraries(${case_name} PRIVATE gtest_main)
#    target_include_directories(${case_name} PRIVATE ${gtest_SOURCE_DIR} ${gtest_SOURCE_DIR}/include)
#    set_target_properties(${case_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/tests)
# 
#    add_test(NAME ${case_name} COMMAND ${case_name} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
#endforeach()


file(GLOB_RECURSE TEST_LIST Tests/Standalone/*.cpp)
foreach(case_file ${TEST_LIST})
    get_filename_component( case_name ${case_file} NAME_WE )
    set (case_name test_${case_name})
    add_executable(${case_name} ${case_file})
    
    addQtDeployment(${case_name})
    target_link_libraries(${case_name} PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui)
    target_link_libraries(${case_name} PUBLIC d3d11.lib DXGI.lib)

    target_link_libraries(${case_name} PRIVATE gtest_main)
    target_include_directories(${case_name} PRIVATE ${gtest_SOURCE_DIR} ${gtest_SOURCE_DIR}/include)
    set_target_properties(${case_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/tests)

    add_test(NAME ${case_name} COMMAND ${case_name} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endforeach()

