#pragma once

#pragma warning(push)
#pragma warning(disable : 4996)
#include <cpprest/containerstream.h>
#include <cpprest/filestream.h>
#include <was/blob.h>
#include <was/core.h>
#include <was/queue.h>
#include <was/storage_account.h>
#pragma warning(pop)

#include <QObject>
Q_DECLARE_METATYPE(azure::storage::storage_uri);