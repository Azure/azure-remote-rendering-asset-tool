#pragma once
#define _TURN_OFF_PLATFORM_STRING
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wreorder"
#pragma clang diagnostic ignored "-Wmicrosoft-extra-qualification"
#pragma clang diagnostic ignored "-Wdelete-non-abstract-non-virtual-dtor"
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#pragma warning(push)
#pragma warning(disable : 4996)
#include <cpprest/containerstream.h>
#include <cpprest/filestream.h>
#include <was/blob.h>
#include <was/core.h>
#include <was/queue.h>
#include <was/storage_account.h>
#pragma warning(pop)
#pragma clang diagnostic pop
