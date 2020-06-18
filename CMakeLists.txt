cmake_minimum_required (VERSION 3.12)

# retrieve version

if(DEFINED ENV{ARRT_VERSION})
    set(ARRT_VERSION $ENV{ARRT_VERSION})
else()
    message("Retrieving version from last git tag")
    find_package (Git)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags
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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ArrtModel

project(ArrtModel)

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

add_library(${CMAKE_PROJECT_NAME} ${SOURCE_LIST})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${SOURCE_ROOT}")


find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Gui)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
# No Windows.h min and max
add_definitions(-DNOMINMAX)

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

set( CFG $<IF:$<CONFIG:Debug>,DEBUG,RELEASE> )
set( POSTFIX $<$<CONFIG:Debug>:d> )

set(LIB_BLOB_ROOT "${CMAKE_BINARY_DIR}/packages/Microsoft.Azure.Storage.CPP.v140.5.0.0")
set(LIB_BLOB_NAME "${LIB_BLOB_ROOT}/lib/native/v140/x64/${CFG}/wastorage${POSTFIX}")
target_link_libraries(${CMAKE_PROJECT_NAME} "${LIB_BLOB_NAME}.lib")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${LIB_BLOB_ROOT}/build/native/include")

set(LIB_REST_ROOT "${CMAKE_BINARY_DIR}/packages/cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn.2.9.1")
set(LIB_REST_NAME "${LIB_REST_ROOT}/lib/native/v140/windesktop/msvcstl/dyn/rt-dyn/x64/${CFG}/cpprest140${POSTFIX}_2_9")
target_link_libraries(${CMAKE_PROJECT_NAME} "${LIB_REST_NAME}.lib")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${LIB_REST_ROOT}/build/native/include")

if ("$ENV{LIB_ARR_USE_LOCAL}" STREQUAL "true")
    set(LIB_ARR_BIN $<IF:$<CONFIG:Debug>,$ENV{LIB_ARR_BIN_DEBUG},$ENV{LIB_ARR_BIN_RELEASE}>)
    set(LIB_ARR_LIB $<IF:$<CONFIG:Debug>,$ENV{LIB_ARR_LIB_DEBUG},$ENV{LIB_ARR_LIB_RELEASE}>)
    set(LIB_ARR_INCLUDE $ENV{LIB_ARR_INCLUDE})
else()
    set(LIB_ARR_ROOT "${CMAKE_BINARY_DIR}/packages/Microsoft.Azure.RemoteRendering.Cpp.0.1.32")
    set(LIB_ARR_BIN "${LIB_ARR_ROOT}/bin/win_x64/release")
    set(LIB_ARR_LIB "${LIB_ARR_ROOT}/lib/win_x64/release")
    set(LIB_ARR_INCLUDE "${LIB_ARR_ROOT}/include")
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} "${LIB_ARR_LIB}/RemoteRenderingClient.lib")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${LIB_ARR_INCLUDE})

if(DEFINED ENV{EXTRA_ARR_ZONE})
    add_definitions( -DEXTRA_ARR_ZONE=$ENV{EXTRA_ARR_ZONE} )
endif()

add_definitions( -DARRT_VERSION="${ARRT_VERSION}" )

target_link_libraries(${CMAKE_PROJECT_NAME} d3d11.lib DXGI.lib crypt32.lib)

unset(SOURCE_LIST)




# ARRT

project(Arrt)

set (SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Arrt")

file(
     GLOB SOURCE_LIST 
	 LIST_DIRECTORIES false
     "${SOURCE_ROOT}/*.cpp" "${SOURCE_ROOT}/*.h" "${SOURCE_ROOT}/Resources/*.qrc" "${SOURCE_ROOT}/Resources/*.rc" "${SOURCE_ROOT}/Resources/*.ico"
    )
    
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

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${SOURCE_LIST})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${SOURCE_ROOT}")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ArrtModel")

get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:Arrt>" --no-translations --no-webkit2 --no-system-d3d-compiler --no-virtualkeyboard --no-angle --no-opengl-sw --no-compiler-runtime
    COMMENT "Running windeployqt..."
)
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_REST_NAME}.dll
        ${LIB_BLOB_NAME}.dll
        ${LIB_ARR_BIN}/RemoteRenderingClient.dll
        ${LIB_ARR_BIN}/PerceptionDevice.dll
        ${LIB_ARR_BIN}/Microsoft.Holographic.HybridRemoting.dll
        $<TARGET_FILE_DIR:Arrt>
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
# No Windows.h min and max
add_definitions(-DNOMINMAX)
add_definitions(-DARRT_VERSION="${ARRT_VERSION}")

target_link_libraries(${CMAKE_PROJECT_NAME} ArrtModel)


# Enable all warnings and turn it into errors

if(MSVC)
    target_compile_options(Arrt PRIVATE /W4 /WX /wd4505 /wd4068 /MP /bigobj)
    target_compile_options(ArrtModel PRIVATE /W4 /WX /wd4505 /wd4068 /MP /bigobj)
else()
    target_compile_options(Arrt PRIVATE -Wall -Wextra -pedantic -Werror -Wno-extra-semi -Wno-unused-function -Wno-language-extension-token -Wno-delete-non-abstract-non-virtual-dtor)
    target_compile_options(ArrtModel PRIVATE -Wall -Wextra -pedantic -Werror -Wno-extra-semi -Wno-unused-function -Wno-language-extension-token -Wno-delete-non-abstract-non-virtual-dtor)
endif()