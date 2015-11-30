// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0        *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved        *
// **************************************************************************

#include "ffs_paths.h"
#include <zen/file_access.h>
#include <wx/stdpaths.h>
#include <wx/app.h>
#include <wx+/string_conv.h>


using namespace zen;


namespace
{
inline
Zstring getExecutableDir() //directory containing executable WITH path separator at end
{
    return appendSeparator(beforeLast(utfCvrtTo<Zstring>(wxStandardPaths::Get().GetExecutablePath()), FILE_NAME_SEPARATOR, IF_MISSING_RETURN_NONE));
}



inline
bool isPortableVersion() { return !endsWith(getExecutableDir(), "/bin/"); } //this check is a bit lame...
}


bool zen::manualProgramUpdateRequired()
{
    return true;
    //return isPortableVersion(); //locally installed version is updated by Launchpad
}


Zstring zen::getResourceDir()
{
    //make independent from wxWidgets global variable "appname"; support being called by RealtimeSync
    auto appName = wxTheApp->GetAppName();
    wxTheApp->SetAppName(L"FreeFileSync");
    ZEN_ON_SCOPE_EXIT(wxTheApp->SetAppName(appName));

    if (isPortableVersion())
        return getExecutableDir();
    else //use OS' standard paths
        return appendSeparator(toZ(wxStandardPathsBase::Get().GetResourcesDir()));
}


Zstring zen::getConfigDir()
{
    //make independent from wxWidgets global variable "appname"; support being called by RealtimeSync
    auto appName = wxTheApp->GetAppName();
    wxTheApp->SetAppName(L"FreeFileSync");
    ZEN_ON_SCOPE_EXIT(wxTheApp->SetAppName(appName));

    if (isPortableVersion())
        return getExecutableDir();
    //use OS' standard paths
    Zstring configDirPath = toZ(wxStandardPathsBase::Get().GetUserDataDir());
    try
    {
        makeDirectoryRecursively(configDirPath); //throw FileError
    }
    catch (const FileError&) { assert(false); }

    return appendSeparator(configDirPath);
}


//this function is called by RealtimeSync!!!
Zstring zen::getFreeFileSyncLauncherPath()
{
    return getExecutableDir() + Zstr("FreeFileSync");

}
