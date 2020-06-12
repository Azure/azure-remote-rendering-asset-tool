#pragma once
#include <QtGlobal>

namespace DpiUtils
{
    // scale a size according to the DPI scaling settings in the system
    qreal size(qreal value);
} // namespace DpiUtils
