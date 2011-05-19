// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2011 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#ifndef TRAYICON_H_INCLUDED
#define TRAYICON_H_INCLUDED

#include <wx/event.h>
#include <wx/toplevel.h>


class MinimizeToTray : private wxEvtHandler
{
public:
    MinimizeToTray(wxTopLevelWindow* callerWnd, wxTopLevelWindow* secondWnd = NULL); //ensure both windows have longer lifetime than this instance!
    ~MinimizeToTray(); //show windows again

    void setToolTip(const wxString& toolTipText, double percent = 0); //percent (optional), number between [0, 100], for small progress indicator
    void keepHidden(); //do not show windows again: avoid window flashing shortly before it is destroyed

private:
    void OnContextMenuSelection(wxCommandEvent& event);
    void OnDoubleClick(wxCommandEvent& event);
    void resumeFromTray();

    wxTopLevelWindow* callerWnd_;
    wxTopLevelWindow* secondWnd_;
    class TaskBarImpl;
    TaskBarImpl* trayIcon; //actual tray icon (don't use inheritance to enable delayed deletion)
};

#endif // TRAYICON_H_INCLUDED
