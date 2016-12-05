// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "status_handler.h"
#include <chrono>
#include <zen/basic_math.h>

using namespace zen;


namespace
{
std::chrono::steady_clock::time_point lastExec;
};

bool zen::updateUiIsAllowed()
{
    const auto now = std::chrono::steady_clock::now();

    if (numeric::dist(now, lastExec) > std::chrono::milliseconds(UI_UPDATE_INTERVAL_MS)) //handle potential chrono wrap-around!
    {
        lastExec = now;
        return true;
    }
    return false;
}
