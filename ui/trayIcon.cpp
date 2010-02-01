#include "trayIcon.h"
#include "../library/resources.h"
#include "smallDialogs.h"
#include <wx/taskbar.h>
#include <cmath>


enum Selection
{
    CONTEXT_RESTORE,
    CONTEXT_ABOUT
};


class MinimizeToTray::TaskBarImpl : public wxTaskBarIcon
{
public:
    TaskBarImpl(MinimizeToTray* parent) : parent_(parent) {}

    void parentHasDied()
    {
        parent_ = NULL;
    }
private:
    virtual wxMenu* CreatePopupMenu()
    {
        if (!parent_)
            return NULL;

        wxMenu* contextMenu = new wxMenu;
        contextMenu->Append(CONTEXT_ABOUT, _("&About..."));
        contextMenu->AppendSeparator();
        contextMenu->Append(CONTEXT_RESTORE, _("&Restore"));
        //event handling
        contextMenu->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MinimizeToTray::OnContextMenuSelection), NULL, parent_);

        return contextMenu; //ownership transferred to library
    }

    MinimizeToTray* parent_;
};


MinimizeToTray::MinimizeToTray(wxTopLevelWindow* callerWnd, wxWindow* secondWnd) :
    callerWnd_(callerWnd),
    secondWnd_(secondWnd),
    trayIcon(new TaskBarImpl(this))
{
    trayIcon->SetIcon(*GlobalResources::getInstance().programIcon, wxT("FreeFileSync"));
    trayIcon->Connect(wxEVT_TASKBAR_LEFT_DCLICK, wxCommandEventHandler(MinimizeToTray::OnDoubleClick), NULL, this); //register double-click

    if (callerWnd_)
        callerWnd_->Hide();
    if (secondWnd_)
        secondWnd_->Hide();
}


MinimizeToTray::~MinimizeToTray()
{
    resumeFromTray();
}


void MinimizeToTray::resumeFromTray() //remove trayIcon and restore windows:  MinimizeToTray is now a zombie object...
{
    if (trayIcon)
    {
        if (secondWnd_)
            secondWnd_->Show();

        if (callerWnd_) //usecase: avoid dialog flashing in batch silent mode
        {
            callerWnd_->Iconize(false);
            callerWnd_->Show();
            callerWnd_->Raise();
            callerWnd_->SetFocus();
        }
        trayIcon->RemoveIcon(); //hide icon until final deletion takes place
        trayIcon->Disconnect(wxEVT_TASKBAR_LEFT_DCLICK, wxCommandEventHandler(MinimizeToTray::OnDoubleClick), NULL, this);

        //use wxWidgets delayed destruction: delete during next idle loop iteration (handle late window messages, e.g. when double-clicking)
        if (!wxPendingDelete.Member(trayIcon))
            wxPendingDelete.Append(trayIcon);

        trayIcon->parentHasDied(); //TaskBarImpl (potentially) has longer lifetime than MinimizeToTray: avoid callback!
        trayIcon = NULL; //avoid reentrance
    }
}


namespace
{
inline
int roundNum(double d) //little rounding function
{
    return static_cast<int>(d < 0 ? d - .5 : d + .5);
}


wxIcon generateIcon(size_t percent) //generate icon with progress indicator
{
    percent = std::min(percent, static_cast<size_t>(100u)); //handle invalid input

#ifdef FFS_WIN
    static const wxBitmap trayIcon = GlobalResources::getInstance().getImageByName(wxT("FFS_tray_win.png"));
#elif defined FFS_LINUX
    static const wxBitmap trayIcon = GlobalResources::getInstance().getImageByName(wxT("FFS_tray_linux.png"));
#endif

    const int indicatorHeight = roundNum((trayIcon.GetHeight() * percent) / 100.0);

    //minor optimization
    static std::pair<int, wxIcon> buffer = std::make_pair(-1, wxNullIcon);
    if (buffer.first == indicatorHeight)
        return buffer.second;

    if (    trayIcon.GetWidth()  > 0 &&
            trayIcon.GetHeight() > 0)
    {
        static const int indicatorWidth = trayIcon.GetWidth() * .25;
        const int indicatorXBegin = ceil((trayIcon.GetWidth() - indicatorWidth) / 2.0);
        const int indicatorYBegin = trayIcon.GetHeight() - indicatorHeight;

        wxImage genImage(trayIcon.ConvertToImage());

        //draw progress indicator: do NOT use wxDC::DrawRectangle! Doesn't respect alpha in Windows, but does in Linux!
        //We need a simple, working solution:
        unsigned char* const data = genImage.GetData();
        for (int row = indicatorYBegin;  row < genImage.GetHeight(); ++row)
        {
            for (int col = indicatorXBegin;  col < indicatorXBegin + indicatorWidth; ++col)
            {
                unsigned char* const pixelBegin = data + (row * genImage.GetWidth() + col) * 3;
                pixelBegin[0] = 255; //red
                pixelBegin[1] = 255; //green
                pixelBegin[2] = 0;   //blue
            }
        }

        if (genImage.HasAlpha())
        {
            unsigned char* const alpha = genImage.GetAlpha();
            //make progress indicator fully opaque:
            for (int row = indicatorYBegin;  row < genImage.GetHeight(); ++row)
                ::memset(alpha + row * genImage.GetWidth() + indicatorXBegin, wxIMAGE_ALPHA_OPAQUE, indicatorWidth);
        }

        wxIcon genIcon;
        genIcon.CopyFromBitmap(wxBitmap(genImage));

        //fill buffer
        buffer.first  = indicatorHeight;
        buffer.second = genIcon;

        return genIcon;
    }

    //fallback
    wxIcon defaultIcon;
    defaultIcon.CopyFromBitmap(trayIcon);

    //fill buffer
    buffer.first  = indicatorHeight;
    buffer.second = defaultIcon;

    return defaultIcon;
}
}


void MinimizeToTray::setToolTip(const wxString& toolTipText, size_t percent)
{
    if (trayIcon)
        trayIcon->SetIcon(generateIcon(percent), toolTipText);
}


void MinimizeToTray::keepHidden()
{
    callerWnd_ = NULL;
    secondWnd_ = NULL;
}


void MinimizeToTray::OnContextMenuSelection(wxCommandEvent& event)
{
    const Selection eventId = static_cast<Selection>(event.GetId());
    switch (eventId)
    {
    case CONTEXT_ABOUT:
    {
        AboutDlg* aboutDlg = new AboutDlg(NULL);
        aboutDlg->ShowModal();
        aboutDlg->Destroy();
    }
    break;
    case CONTEXT_RESTORE:
        resumeFromTray();
    }
}


void MinimizeToTray::OnDoubleClick(wxCommandEvent& event)
{
    resumeFromTray();
}

