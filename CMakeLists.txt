cmake_minimum_required (VERSION 3.12)

set( CMAKE_CONFIGURATION_TYPES Debug;Release )

if ("$ENV{LIB_ARR_USE_LOCAL}" STREQUAL "true")
    set(LIB_ARR_BIN $<IF:$<CONFIG:Debug>,$ENV{LIB_ARR_BIN_DEBUG},$ENV{LIB_ARR_BIN_RELEASE}>)
    set(LIB_ARR_LIB $<IF:$<CONFIG:Debug>,$ENV{LIB_ARR_LIB_DEBUG},$ENV{LIB_ARR_LIB_RELEASE}>)
    set(LIB_ARR_INCLUDE $ENV{LIB_ARR_INCLUDE})
else()
    set(LIB_ARR_ROOT "${CMAKE_BINARY_DIR}/packages/Microsoft.Azure.RemoteRendering.Cpp.1.0.36")
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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QTV "Qt6")

function(setup_qt TARGET)

    set(COMPONENTS Core Widgets Gui Svg)

    find_package(${QTV} COMPONENTS ${COMPONENTS} REQUIRED)

    get_target_property(QT_CORE_DLL ${QTV}::Core IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_CORE_DLL}" DIRECTORY)
    get_filename_component(QT_DIR "${QT_BIN_DIR}" DIRECTORY)
    set(QT_DIR "${QT_DIR}/${QTV}")

    foreach(module ${COMPONENTS})

        target_link_libraries(${TARGET} PUBLIC "${QTV}::${module}")

        # copy the dll into the binary folder for each configuration using generator expressions
        # as a post-build step for every qt-enabled target:
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${QTV}::${module}>
            $<TARGET_FILE_DIR:${TARGET}>)

    endforeach()

    # Copy 'imageformats' into the binary folder.
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_DIR}/plugins/imageformats"
        "$<TARGET_FILE_DIR:${TARGET}>/imageformats")

    # Copy 'platforms' into the binary folder.
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_DIR}/plugins/platforms"
        "$<TARGET_FILE_DIR:${TARGET}>/platforms")

    # Copy 'iconengines' into the binary folder.
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_DIR}/plugins/iconengines"
        "$<TARGET_FILE_DIR:${TARGET}>/iconengines")

endfunction()

#################
# ARRT SOLUTION
#
project(Arrt)

# Global options, for all of the projects
add_compile_options( /W4 /wd4505 /wd4068 /MP /bigobj /Zi)
 
# Enable pdb generation in release builds.
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
# No Windows.h min and max
add_definitions(-DNOMINMAX)
add_definitions(-D_TURN_OFF_PLATFORM_STRING)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###################
# ARRT Executable
#
set (TARGET "Arrt")

set (SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ArrtSource")
file(GLOB SOURCE_LIST LIST_DIRECTORIES false "${SOURCE_ROOT}/*.cpp" "${SOURCE_ROOT}/*.h" "${SOURCE_ROOT}/Resources/*.qrc" "${SOURCE_ROOT}/Resources/*.ico" "${SOURCE_ROOT}/Resources/*.rc")
    
set(SRC_DIRS
	App
	Conversion
	Rendering
	Storage
	Utils
)

foreach(DIR IN LISTS SRC_DIRS)
	file(GLOB_RECURSE NEW_FILES "${SOURCE_ROOT}/${DIR}/*.cpp" "${SOURCE_ROOT}/${DIR}/*.h" "${SOURCE_ROOT}/${DIR}/*.ui")
    set(SOURCE_LIST ${SOURCE_LIST} ${NEW_FILES})
endforeach()

source_group(TREE "${SOURCE_ROOT}" FILES ${SOURCE_LIST})

add_executable(${TARGET} WIN32 ${SOURCE_LIST})

setup_qt(${TARGET})

add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LIB_ARR_BIN}/RemoteRenderingClient.dll
        ${LIB_ARR_BIN}/Microsoft.Holographic.HybridRemoting.dll
        $<TARGET_FILE_DIR:${TARGET}>
)

find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(${TARGET} PRIVATE cpprestsdk::cpprest)

target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ArrtSource")
target_link_libraries(${TARGET} PUBLIC d3d11.lib DXGI.lib)

target_link_libraries(${TARGET} PUBLIC "${LIB_ARR_LIB}/RemoteRenderingClient.lib")
target_include_directories(${TARGET} PUBLIC ${LIB_ARR_INCLUDE})

if (USE_NEW_AZURE_STORAGE_SDK)

    find_package(azure-storage-blobs-cpp CONFIG REQUIRED)
    target_link_libraries(${TARGET} PRIVATE Azure::azure-storage-blobs)

    target_compile_definitions(${TARGET} PUBLIC NEW_STORAGE_SDK=1)

else()

    set( CFG $<IF:$<CONFIG:Debug>,DEBUG,RELEASE> )
    set( POSTFIX $<$<CONFIG:Debug>:d> )

    set(LIB_BLOB_ROOT "${CMAKE_BINARY_DIR}/packages/Microsoft.Azure.Storage.CPP.v140.5.0.0")
    set(LIB_BLOB_NAME "${LIB_BLOB_ROOT}/lib/native/v140/x64/${CFG}/wastorage${POSTFIX}")

    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
             ${LIB_BLOB_NAME}.dll
            $<TARGET_FILE_DIR:${TARGET}>
    )

    target_link_libraries(${TARGET} PUBLIC "${LIB_BLOB_NAME}.lib")
    target_include_directories(${TARGET} PUBLIC "${LIB_BLOB_ROOT}/build/native/include")

endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Arrt)



