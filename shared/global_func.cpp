// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2011 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#include "global_func.h"
#include "string_tools.h"
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/stopwatch.h>
#include "system_constants.h"

using namespace common;
using namespace zen;


size_t common::getDigitCount(size_t number) //count number of digits
{
    return number == 0 ? 1 : static_cast<size_t>(::log10(static_cast<double>(number))) + 1;
}

//############################################################################
DebugLog::DebugLog(const wxString& filePrefix) :
    prefix(filePrefix),
    lineCount(0),
    logFile(NULL)
{
    logFile = new wxFile;
    logfileName = assembleFileName();
    logFile->Open(logfileName.c_str(), wxFile::write);
}


DebugLog::~DebugLog()
{
    delete logFile; //automatically closes file handle
}


wxString DebugLog::assembleFileName()
{
    wxString tmp = wxDateTime::Now().FormatISOTime();
    tmp.Replace(wxT(":"), wxEmptyString);
    return prefix + wxString(wxT("DEBUG_")) + wxDateTime::Now().FormatISODate() + wxChar('_') + tmp + wxT(".log");
}


void DebugLog::write(const wxString& logText)
{
    ++lineCount;
    if (lineCount % 50000 == 0) //prevent logfile from becoming too big
    {
        logFile->Close();
        wxRemoveFile(logfileName);

        logfileName = assembleFileName();
        logFile->Open(logfileName.c_str(), wxFile::write);
    }

    logFile->Write(wxString(wxT("[")) + wxDateTime::Now().FormatTime() + wxT("] "));
    logFile->Write(logText + LINE_BREAK);
}

//DebugLog logDebugInfo;


wxString getCodeLocation(const wxString& file, int line)
{
    return wxString(file).AfterLast(FILE_NAME_SEPARATOR) + wxT(", LINE ") + toString<wxString>(line) + wxT(" | ");
}
