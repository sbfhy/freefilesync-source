// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2010 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#ifndef FREEFILESYNCAPP_H
#define FREEFILESYNCAPP_H

#include <wx/app.h>
#include "library/processXml.h"


class Application : public wxApp
{
public:
    bool OnInit();
    int  OnRun();
    int  OnExit();
    bool OnExceptionInMainLoop();
    void OnStartApplication(wxIdleEvent& event);

private:
    void runGuiMode(const wxString& cfgFileName, xmlAccess::XmlGlobalSettings& settings);
    void runBatchMode(const wxString& filename, xmlAccess::XmlGlobalSettings& globSettings);

    virtual wxLayoutDirection GetLayoutDirection() const //disable RTL languages for now...
    {
        return wxLayout_LeftToRight;
    }


    xmlAccess::XmlGlobalSettings globalSettings; //settings used by GUI, batch mode or both
    int returnValue;
};

#endif // FREEFILESYNCAPP_H
