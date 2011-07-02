// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2011 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/choice.h>
#include "zstring.h"
#include "string_tools.h"
#include "int64.h"

namespace zen
{
wxString extractJobName(const wxString& configFilename);

wxString formatFilesizeToShortString(zen::UInt64 filesize);
wxString formatPercentage(zen::Int64 dividend, zen::Int64 divisor);

template <class NumberType>
wxString toStringSep(NumberType number); //convert number to wxString including thousands separator

void scrollToBottom(wxScrolledWindow* scrWindow);

wxString utcTimeToLocalString(zen::Int64 utcTime); //throw std::runtime_error
}


























//--------------- inline impelementation -------------------------------------------

//helper function! not to be used directly
namespace ffs_Impl
{
wxString includeNumberSeparator(const wxString& number);
}


namespace zen
{
template <class NumberType>
inline
wxString toStringSep(NumberType number)
{
    return ffs_Impl::includeNumberSeparator(zen::toString<wxString>(number));
}
}


#endif // UTIL_H_INCLUDED
