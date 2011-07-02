// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2011 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#include "check_version.h"
#include <wx/msgdlg.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include "../version/version.h"
#include <wx/utils.h>
#include <wx/timer.h>
#include <string_tools.h>
#include "msg_popup.h"
#include "../shared/standard_paths.h"
#include <wx/tokenzr.h>
#include "../shared/i18n.h"

class CloseConnectionOnExit
{
public:
    CloseConnectionOnExit(wxInputStream* httpStream, wxHTTP& webAccess) :
        m_httpStream(httpStream),
        m_webAccess(webAccess) {}

    ~CloseConnectionOnExit()
    {
        delete m_httpStream; //must be deleted BEFORE webAccess is closed
        m_webAccess.Close();
    }

private:
    wxInputStream* m_httpStream;
    wxHTTP& m_webAccess;
};


bool getOnlineVersion(wxString& version)
{
    wxHTTP webAccess;
    wxInputStream* httpStream = NULL;

    wxWindowDisabler dummy;
    CloseConnectionOnExit dummy2(httpStream, webAccess);

    webAccess.SetHeader(wxT("Content-type"), wxT("text/html; charset=utf-8"));
    webAccess.SetTimeout(5); //5 seconds of timeout instead of 10 minutes...

    if (webAccess.Connect(wxT("freefilesync.cvs.sourceforge.net"))) //only the server, no pages here yet...
    {
        //wxApp::IsMainLoopRunning(); // should return true

        httpStream = webAccess.GetInputStream(wxT("/viewvc/freefilesync/version/version.txt"));

        if (httpStream && webAccess.GetError() == wxPROTO_NOERR)
        {
            wxString newestVersion;
            wxStringOutputStream out_stream(&newestVersion);
            httpStream->Read(out_stream);
            if (!newestVersion.empty())
            {
                version = newestVersion;
                return true;
            }
        }
    }

    return false;
}


const wxChar VERSION_SEP = wxT('.');


std::vector<size_t> parseVersion(const wxString& version)
{
    std::vector<size_t> output;

    wxStringTokenizer tkz(version, VERSION_SEP, wxTOKEN_RET_EMPTY);
    while (tkz.HasMoreTokens())
    {
        const wxString& token = tkz.GetNextToken();
        output.push_back(zen::toNumber<size_t>(token));
    }
    return output;
}


bool newerVersionExists(const wxString& onlineVersion)
{
    std::vector<size_t> current = parseVersion(zen::currentVersion);
    std::vector<size_t> online  = parseVersion(onlineVersion);

    if (online.empty() || online[0] == 0) //onlineVersion may be "This website has been moved..." In this case better check for an update
        return true;

    return std::lexicographical_compare(current.begin(), current.end(),
                                        online.begin(), online.end());
}


void zen::checkForUpdateNow()
{
    wxString onlineVersion;
    if (!getOnlineVersion(onlineVersion))
    {
        wxMessageBox(_("Unable to connect to sourceforge.net!"), _("Error"), wxOK | wxICON_ERROR);
        return;
    }

    if (newerVersionExists(onlineVersion))
    {
        const int rv = wxMessageBox(wxString(_("A newer version of FreeFileSync is available:"))  + wxT(" v") + onlineVersion + wxT(". ") + _("Download now?"), _("Information"), wxYES_NO | wxICON_QUESTION);
        if (rv == wxYES)
            wxLaunchDefaultBrowser(wxT("http://sourceforge.net/projects/freefilesync/files/"));
    }
    else
        wxMessageBox(_("FreeFileSync is up to date!"), _("Information"), wxICON_INFORMATION);
}


void zen::checkForUpdatePeriodically(long& lastUpdateCheck)
{
#ifdef FFS_LINUX
    if (!zen::isPortableVersion()) //don't check for updates in installer version -> else: handled by .deb
        return;
#endif

    if (lastUpdateCheck != -1)
    {
        if (lastUpdateCheck == 0)
        {
            const bool checkRegularly = showQuestionDlg(ReturnQuestionDlg::BUTTON_YES | ReturnQuestionDlg::BUTTON_NO,
                                                        wxString(_("Do you want FreeFileSync to automatically check for updates every week?")) + wxT("\n") +
                                                        _("(Requires an Internet connection!)")) == ReturnQuestionDlg::BUTTON_YES;
            if (checkRegularly)
            {
                lastUpdateCheck = 123; //some old date (few seconds after 1970)

                checkForUpdatePeriodically(lastUpdateCheck); //check for updates now
            }
            else
                lastUpdateCheck = -1; //don't check for updates anymore
        }
        else if (wxGetLocalTime() >= lastUpdateCheck + 7 * 24 * 3600) //check weekly
        {
            wxString onlineVersion;
            if (!getOnlineVersion(onlineVersion))
                return; //do not handle error

            lastUpdateCheck = wxGetLocalTime();

            if (newerVersionExists(onlineVersion))
            {
                const int rv = wxMessageBox(wxString(_("A newer version of FreeFileSync is available:"))  + wxT(" v") + onlineVersion + wxT(". ") + _("Download now?"), _("Information"), wxYES_NO | wxICON_QUESTION);
                if (rv == wxYES)
                    wxLaunchDefaultBrowser(wxT("http://sourceforge.net/projects/freefilesync/files/"));
            }
        }
    }
}

