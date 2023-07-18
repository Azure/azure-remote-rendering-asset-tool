#pragma once

// Version information which is going to be exposed in the output executable

#define STR2(s) #s
#define STR(s) STR2(s)

#define ARRT_VERSION_MAJOR 2
#define ARRT_VERSION_MINOR 3
#define ARRT_VERSION_PATCH 14
#define ARRT_VERSION STR(ARRT_VERSION_MAJOR) "." STR(ARRT_VERSION_MINOR) "." STR(ARRT_VERSION_PATCH)

#define VER_COMPANY "Microsoft Corporation"
#define VER_PRODUCTNAME "Azure Remote Rendering Toolkit"
#define VER_FILE_DESCRIPTION VER_PRODUCTNAME
#define VER_FILE_VERSION ARRT_VERSION
#define VER_INTERNAL_NAME VER_PRODUCTNAME
#define VER_COPYRIGHT "Copyright (C) 2023"
#define VER_ORIGINAL_FILENAME "Arrt.exe"
#define VER_PRODUCT_VERSION ARRT_VERSION
