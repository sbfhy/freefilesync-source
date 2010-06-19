// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2010 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#include "progressIndicator.h"
#include <memory>
#include "guiGenerated.h"
#include <wx/stopwatch.h>
#include "../library/resources.h"
#include "../shared/stringConv.h"
#include "../shared/util.h"
#include "../library/statistics.h"
#include <wx/wupdlock.h>
#include "../shared/globalFunctions.h"
#include "trayIcon.h"
#include <boost/shared_ptr.hpp>

#ifdef FFS_WIN
#include "../shared/taskbar.h"
#endif

using namespace FreeFileSync;


namespace
{
void setNewText(const wxString& newText, wxTextCtrl& control, bool& updateLayout)
{
    if (control.GetValue().length() != newText.length())
        updateLayout = true; //avoid screen flicker: apply only when necessary

    if (control.GetValue() != newText)
        control.ChangeValue(newText);
}

void setNewText(const wxString& newText, wxStaticText& control, bool& updateLayout)
{
    if (control.GetLabel().length() != newText.length())
        updateLayout = true; //avoid screen flicker

    if (control.GetLabel() != newText)
        control.SetLabel(newText);
}
}


class CompareStatus::CompareStatusImpl : public CompareStatusGenerated
{
public:
    CompareStatusImpl(wxTopLevelWindow& parentWindow);

    void init(); //make visible, initialize all status values
    void finalize(); //hide again

    void switchToCompareBytewise(int totalObjectsToProcess, wxLongLong totalDataToProcess);
    void incScannedObjects_NoUpdate(int number);
    void incProcessedCmpData_NoUpdate(int objectsProcessed, wxLongLong dataProcessed);
    void setStatusText_NoUpdate(const Zstring& text);
    void updateStatusPanelNow();

private:
    wxTopLevelWindow& parentWindow_;
    wxString titleTextBackup;

    //status variables
    size_t scannedObjects;
    Zstring currentStatusText;

    wxStopWatch timeElapsed;

    //gauge variables
    int        totalObjects;
    wxLongLong totalData;      //each data element represents one byte for proper progress indicator scaling
    int        currentObjects; //each object represents a file or directory processed
    wxLongLong currentData;
    double     scalingFactor;  //nr of elements has to be normalized to smaller nr. because of range of int limitation

    void showProgressExternally(const wxString& progressText, float percent = 0);

    enum CurrentStatus
    {
        SCANNING,
        COMPARING_CONTENT,
    };

    CurrentStatus status;

#ifdef FFS_WIN
    std::auto_ptr<Utility::TaskbarProgress> taskbar_;
#endif

    //remaining time
    std::auto_ptr<Statistics> statistics;
    long lastStatCallSpeed;   //used for calculating intervals between statistics update
    long lastStatCallRemTime; //
};

//redirect to implementation
CompareStatus::CompareStatus(wxTopLevelWindow& parentWindow) :
    pimpl(new CompareStatusImpl(parentWindow)) {}

CompareStatus::~CompareStatus()
{
    //DON'T delete pimpl! it relies on wxWidgets destruction (parent window destroys child windows!)
}

wxWindow* CompareStatus::getAsWindow()
{
    return pimpl;
}

void CompareStatus::init()
{
    pimpl->init();
}

void CompareStatus::finalize()
{
    pimpl->finalize();
}

void CompareStatus::switchToCompareBytewise(int totalObjectsToProcess, wxLongLong totalDataToProcess)
{
    pimpl->switchToCompareBytewise(totalObjectsToProcess, totalDataToProcess);
}

void CompareStatus::incScannedObjects_NoUpdate(int number)
{
    pimpl->incScannedObjects_NoUpdate(number);
}

void CompareStatus::incProcessedCmpData_NoUpdate(int objectsProcessed, wxLongLong dataProcessed)
{
    pimpl->incProcessedCmpData_NoUpdate(objectsProcessed, dataProcessed);
}

void CompareStatus::setStatusText_NoUpdate(const Zstring& text)
{
    pimpl->setStatusText_NoUpdate(text);
}

void CompareStatus::updateStatusPanelNow()
{
    pimpl->updateStatusPanelNow();
}
//########################################################################################


CompareStatus::CompareStatusImpl::CompareStatusImpl(wxTopLevelWindow& parentWindow) :
    CompareStatusGenerated(&parentWindow),
    parentWindow_(parentWindow),
    scannedObjects(0),
    totalObjects(0),
    totalData(0),
    currentObjects(0),
    currentData(0),
    scalingFactor(0),
    status(SCANNING),
    lastStatCallSpeed(-1000000), //some big number
    lastStatCallRemTime(-1000000)
{
    init();
}


void CompareStatus::CompareStatusImpl::init()
{
    titleTextBackup = parentWindow_.GetTitle();

#ifdef FFS_WIN
    try //try to get access to Windows 7 Taskbar
    {
        taskbar_.reset(new Utility::TaskbarProgress(parentWindow_));
    }
    catch (const Utility::TaskbarNotAvailable&) {}
#endif

    status = SCANNING;

    //initialize gauge
    m_gauge2->SetRange(50000);
    m_gauge2->SetValue(0);

    //initially hide status that's relevant for comparing bytewise only
    bSizerFilesFound->Show(true);
    bSizerFilesRemaining->Show(false);
    sSizerSpeed->Show(false);
    sSizerTimeRemaining->Show(false);

    m_gauge2->Hide();
    bSizer42->Layout();

    scannedObjects = 0;
    currentStatusText.clear();

    totalObjects   = 0;
    totalData      = 0;
    currentObjects = 0;
    currentData    = 0;
    scalingFactor  = 0;

    statistics.reset();

    timeElapsed.Start(); //measure total time

    updateStatusPanelNow();

    Show(); //make visible
}


void CompareStatus::CompareStatusImpl::finalize() //hide again
{
#ifdef FFS_WIN
    taskbar_.reset();
#endif

    Hide();
    parentWindow_.SetTitle(titleTextBackup);
}


void CompareStatus::CompareStatusImpl::switchToCompareBytewise(int totalObjectsToProcess, wxLongLong totalDataToProcess)
{
    status = COMPARING_CONTENT;

    currentData = 0;
    totalData = totalDataToProcess;

    currentObjects = 0;
    totalObjects   = totalObjectsToProcess;

    if (totalData != 0)
        scalingFactor = 50000 / totalData.ToDouble(); //let's normalize to 50000
    else
        scalingFactor = 0;

    //set new statistics handler: 10 seconds "window" for remaining time, 5 seconds for speed
    statistics.reset(new Statistics(totalObjectsToProcess, totalDataToProcess.ToDouble(), 10000, 5000));
    lastStatCallSpeed   = -1000000; //some big number
    lastStatCallRemTime = -1000000;

    //show status for comparing bytewise
    bSizerFilesFound->Show(false);
    bSizerFilesRemaining->Show(true);
    sSizerSpeed->Show(true);
    sSizerTimeRemaining->Show(true);

    m_gauge2->Show();
    bSizer42->Layout();
}


void CompareStatus::CompareStatusImpl::incScannedObjects_NoUpdate(int number)
{
    scannedObjects += number;
}


void CompareStatus::CompareStatusImpl::incProcessedCmpData_NoUpdate(int objectsProcessed, wxLongLong dataProcessed)
{
    currentData    +=    dataProcessed;
    currentObjects += objectsProcessed;
}


void CompareStatus::CompareStatusImpl::setStatusText_NoUpdate(const Zstring& text)
{
    currentStatusText = text;
}


void CompareStatus::CompareStatusImpl::showProgressExternally(const wxString& progressText, float percent)
{
    if (parentWindow_.GetTitle() != progressText)
        parentWindow_.SetTitle(progressText);

    //show progress on Windows 7 taskbar
#ifdef FFS_WIN
    using namespace Utility;

    if (taskbar_.get())
    {
        const size_t current = 100000 * percent / 100;
        const size_t total   = 100000;
        switch (status)
        {
        case SCANNING:
            taskbar_->setStatus(TaskbarProgress::STATUS_INDETERMINATE);
            break;
        case COMPARING_CONTENT:
            taskbar_->setStatus(TaskbarProgress::STATUS_NORMAL);
            taskbar_->setProgress(current, total);
            break;
        }
    }
#endif
}


void CompareStatus::CompareStatusImpl::updateStatusPanelNow()
{
    //static RetrieveStatistics statistic;
    //statistic.writeEntry(currentData, currentObjects);
    {
        //wxWindowUpdateLocker dummy(this) -> not needed

        const float percent = totalData == 0 ? 0 : currentData.ToDouble() * 100 / totalData.ToDouble();

        //write status information to taskbar, parent title ect.
        switch (status)
        {
        case SCANNING:
            showProgressExternally(numberToStringSep(scannedObjects) + wxT(" - ") + _("Scanning..."));
            break;
        case COMPARING_CONTENT:
            showProgressExternally(formatPercentage(currentData, totalData) + wxT(" - ") + _("Comparing content..."), percent);
            break;
        }


        bool updateLayout = false; //avoid screen flicker by calling layout() only if necessary

        //remove linebreaks from currentStatusText
        wxString formattedStatusText = zToWx(currentStatusText);
        for (wxString::iterator i = formattedStatusText.begin(); i != formattedStatusText.end(); ++i)
            if (*i == wxChar('\n'))
                *i = wxChar(' ');

        //status texts
        if (m_textCtrlStatus->GetValue() != formattedStatusText) //no layout update for status texts!
            m_textCtrlStatus->ChangeValue(formattedStatusText);

        //nr of scanned objects
        setNewText(numberToStringSep(scannedObjects), *m_staticTextScanned, updateLayout);

        //progress indicator for "compare file content"
        m_gauge2->SetValue(int(currentData.ToDouble() * scalingFactor));

        //remaining files left for file comparison
        const wxString filesToCompareTmp = numberToStringSep(totalObjects - currentObjects);
        setNewText(filesToCompareTmp, *m_staticTextFilesRemaining, updateLayout);

        //remaining bytes left for file comparison
        const wxString remainingBytesTmp = FreeFileSync::formatFilesizeToShortString(totalData - currentData);
        setNewText(remainingBytesTmp, *m_staticTextDataRemaining, updateLayout);

        if (statistics.get())
        {
            if (timeElapsed.Time() - lastStatCallSpeed >= 500) //call method every 500 ms
            {
                lastStatCallSpeed = timeElapsed.Time();

                statistics->addMeasurement(currentObjects, currentData.ToDouble());

                //current speed
                setNewText(statistics->getBytesPerSecond(), *m_staticTextSpeed, updateLayout);

                if (timeElapsed.Time() - lastStatCallRemTime >= 2000) //call method every two seconds only
                {
                    lastStatCallRemTime = timeElapsed.Time();

                    //remaining time
                    setNewText(statistics->getRemainingTime(), *m_staticTextTimeRemaining, updateLayout);
                }
            }
        }

        //time elapsed
        setNewText(wxTimeSpan::Milliseconds(timeElapsed.Time()).Format(), *m_staticTextTimeElapsed, updateLayout);

        //do the ui update
        if (updateLayout)
            bSizer42->Layout();
    }
    updateUiNow();
}
//########################################################################################


class SyncStatus::SyncStatusImpl : public SyncStatusDlgGenerated
{
public:
    SyncStatusImpl(StatusHandler& updater, wxTopLevelWindow* parentWindow);
    ~SyncStatusImpl();

    void resetGauge(int totalObjectsToProcess, wxLongLong totalDataToProcess);
    void incProgressIndicator_NoUpdate(int objectsProcessed, wxLongLong dataProcessed);
    void incScannedObjects_NoUpdate(int number);
    void setStatusText_NoUpdate(const Zstring& text);
    void updateStatusDialogNow();

    void setCurrentStatus(SyncStatus::SyncStatusID id);
    void processHasFinished(SyncStatus::SyncStatusID id, const wxString& finalMessage);  //essential to call this in StatusUpdater derived class destructor at the LATEST(!) to prevent access to currentStatusUpdater

    void minimizeToTray();

private:
    void OnKeyPressed(wxKeyEvent& event);
    virtual void OnOkay(wxCommandEvent& event);
    virtual void OnPause(wxCommandEvent& event);
    virtual void OnAbort(wxCommandEvent& event);
    virtual void OnClose(wxCloseEvent& event);
    virtual void OnIconize(wxIconizeEvent& event);

    void resumeFromSystray();
    bool currentProcessIsRunning();
    void showProgressExternally(const wxString& progressText, float percent = 0); //percent may already be included in progressText

    wxStopWatch timeElapsed;

    StatusHandler* processStatusHandler;
    wxTopLevelWindow* mainDialog;

    //gauge variables
    int        totalObjects;
    wxLongLong totalData;
    int        currentObjects; //each object represents a file or directory processed
    wxLongLong currentData;    //each data element represents one byte for proper progress indicator scaling
    double     scalingFactor;  //nr of elements has to be normalized to smaller nr. because of range of int limitation

    //status variables
    size_t scannedObjects;
    Zstring currentStatusText;

    bool processPaused;
    SyncStatus::SyncStatusID currentStatus;

#ifdef FFS_WIN
    std::auto_ptr<Utility::TaskbarProgress> taskbar_;
#endif

    //remaining time
    std::auto_ptr<Statistics> statistics;
    long lastStatCallSpeed;   //used for calculating intervals between statistics update
    long lastStatCallRemTime; //

    wxString titelTextBackup;

    boost::shared_ptr<MinimizeToTray> minimizedToSysTray; //optional: if filled, hides all visible windows, shows again if destroyed
};


//redirect to implementation
SyncStatus::SyncStatus(StatusHandler& updater, wxTopLevelWindow* parentWindow, bool startSilent) :
    pimpl(new SyncStatusImpl(updater, parentWindow))
{
    if (startSilent)
        pimpl->minimizeToTray();
    else
    {
        pimpl->Show();
        pimpl->updateStatusDialogNow(); //update visual statistics to get rid of "dummy" texts
    }
}

SyncStatus::~SyncStatus()
{
    //DON'T delete pimpl! it will be deleted by the user clicking "OK/Cancel" -> (wxWindow::Destroy())
}

wxWindow* SyncStatus::getAsWindow()
{
    return pimpl;
}

void SyncStatus::closeWindowDirectly() //don't wait for user (silent mode)
{
    pimpl->Destroy();
}

void SyncStatus::resetGauge(int totalObjectsToProcess, wxLongLong totalDataToProcess)
{
    pimpl->resetGauge(totalObjectsToProcess, totalDataToProcess);
}

void SyncStatus::incScannedObjects_NoUpdate(int number)
{
    pimpl->incScannedObjects_NoUpdate(number);
}

void SyncStatus::incProgressIndicator_NoUpdate(int objectsProcessed, wxLongLong dataProcessed)
{
    pimpl->incProgressIndicator_NoUpdate(objectsProcessed, dataProcessed);
}

void SyncStatus::setStatusText_NoUpdate(const Zstring& text)
{
    pimpl->setStatusText_NoUpdate(text);
}

void SyncStatus::updateStatusDialogNow()
{
    pimpl->updateStatusDialogNow();
}

void SyncStatus::setCurrentStatus(SyncStatusID id)
{
    pimpl->setCurrentStatus(id);
}

void SyncStatus::processHasFinished(SyncStatusID id, const wxString& finalMessage)
{
    pimpl->processHasFinished(id, finalMessage);
}
//########################################################################################


SyncStatus::SyncStatusImpl::SyncStatusImpl(StatusHandler& updater, wxTopLevelWindow* parentWindow) :
    SyncStatusDlgGenerated(parentWindow,
                           wxID_ANY,
                           parentWindow ? wxString(wxEmptyString) : (wxString(wxT("FreeFileSync - ")) + _("Folder Comparison and Synchronization")),
                           wxDefaultPosition, wxSize(638, 376),
                           parentWindow ?
                           wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL | wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT : //wxTAB_TRAVERSAL is needed for standard button handling: wxID_OK/wxID_CANCEL
                           wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL),
    processStatusHandler(&updater),
    mainDialog(parentWindow),
    totalObjects(0),
    totalData(0),
    currentObjects(0),
    currentData(0),
    scalingFactor(0),
    scannedObjects(0),
    processPaused(false),
    currentStatus(SyncStatus::ABORTED),
    lastStatCallSpeed(-1000000), //some big number
    lastStatCallRemTime(-1000000)
{
    if (mainDialog) //save old title (will be used as progress indicator)
        titelTextBackup = mainDialog->GetTitle();

    m_animationControl1->SetAnimation(*GlobalResources::getInstance().animationSync);
    m_animationControl1->Play();

    m_staticTextSpeed->SetLabel(wxT("-"));
    m_staticTextTimeRemaining->SetLabel(wxT("-"));

    //initialize gauge
    m_gauge1->SetRange(50000);
    m_gauge1->SetValue(0);

    if (IsShown()) //don't steal focus when starting in sys-tray!
        m_buttonAbort->SetFocus();

    if (mainDialog)    //disable (main) window while this status dialog is shown
        mainDialog->Disable();

    timeElapsed.Start(); //measure total time

#ifdef FFS_WIN
    try //try to get access to Windows 7 Taskbar
    {
        taskbar_.reset(new Utility::TaskbarProgress(mainDialog != NULL ? *mainDialog : *this));
    }
    catch (const Utility::TaskbarNotAvailable&) {}
#endif

    //hide "processed" statistics until end of process
    bSizerObjectsProcessed->Show(false);
    //bSizerDataProcessed->Show(false);

    SetIcon(*GlobalResources::getInstance().programIcon); //set application icon

    //register key event
    Connect(wxEVT_CHAR_HOOK, wxKeyEventHandler(SyncStatusImpl::OnKeyPressed), NULL, this);
}


SyncStatus::SyncStatusImpl::~SyncStatusImpl()
{
    if (mainDialog)
    {
        //restore title text
        mainDialog->SetTitle(titelTextBackup);

        mainDialog->Enable();
        mainDialog->Raise();
        mainDialog->SetFocus();
    }

    if (minimizedToSysTray.get())
        minimizedToSysTray->keepHidden(); //prevent window from flashing shortly before it is destroyed
}


void SyncStatus::SyncStatusImpl::OnKeyPressed(wxKeyEvent& event)
{
    const int keyCode = event.GetKeyCode();
    if (keyCode == WXK_ESCAPE)
        Close(); //generate close event: do NOT destroy window unconditionally!

    event.Skip();
}


void SyncStatus::SyncStatusImpl::resetGauge(int totalObjectsToProcess, wxLongLong totalDataToProcess)
{
    currentData = 0;
    totalData = totalDataToProcess;

    currentObjects = 0;
    totalObjects   = totalObjectsToProcess;

    if (totalData != 0)
        scalingFactor = 50000 / totalData.ToDouble(); //let's normalize to 50000
    else
        scalingFactor = 0;

    //set new statistics handler: 10 seconds "window" for remaining time, 5 seconds for speed
    statistics.reset(new Statistics(totalObjectsToProcess, totalDataToProcess.ToDouble(), 10000, 5000));
    lastStatCallSpeed   = -1000000; //some big number
    lastStatCallRemTime = -1000000;
}


void SyncStatus::SyncStatusImpl::incProgressIndicator_NoUpdate(int objectsProcessed, wxLongLong dataProcessed)
{
    currentData    +=    dataProcessed;
    currentObjects += objectsProcessed;
}


void SyncStatus::SyncStatusImpl::incScannedObjects_NoUpdate(int number)
{
    scannedObjects += number;
}


void SyncStatus::SyncStatusImpl::setStatusText_NoUpdate(const Zstring& text)
{
    currentStatusText = text;
}


void SyncStatus::SyncStatusImpl::showProgressExternally(const wxString& progressText, float percent)
{
    //write status information to systray, if window is minimized
    if (minimizedToSysTray.get())
        minimizedToSysTray->setToolTip(progressText, percent);
    //minimizedToSysTray may be a zombie... so set title text anyway

    if (mainDialog) //show percentage in maindialog title (and thereby in taskbar)
    {
        if (mainDialog->GetTitle() != progressText)
            mainDialog->SetTitle(progressText);
    }
    else //show percentage in this dialog's title (and thereby in taskbar)
    {
        if (this->GetTitle() != progressText)
            this->SetTitle(progressText);
    }

#ifdef FFS_WIN
    using namespace Utility;

    //show progress on Windows 7 taskbar
    if (taskbar_.get())
    {
        const size_t current = 100000 * percent / 100;
        const size_t total   = 100000;

        switch (currentStatus)
        {
        case SyncStatus::SCANNING:
            taskbar_->setStatus(TaskbarProgress::STATUS_INDETERMINATE);
            break;
        case SyncStatus::FINISHED_WITH_SUCCESS:
        case SyncStatus::COMPARING_CONTENT:
        case SyncStatus::SYNCHRONIZING:
            taskbar_->setStatus(TaskbarProgress::STATUS_NORMAL);
            taskbar_->setProgress(current, total);
            break;
        case SyncStatus::PAUSE:
            taskbar_->setStatus(TaskbarProgress::STATUS_PAUSED);
            taskbar_->setProgress(current, total);
            break;
        case SyncStatus::ABORTED:
        case SyncStatus::FINISHED_WITH_ERROR:
            taskbar_->setStatus(TaskbarProgress::STATUS_ERROR);
            taskbar_->setProgress(current, total);
            break;
        }
    }
#endif
}


void SyncStatus::SyncStatusImpl::updateStatusDialogNow()
{
    //static RetrieveStatistics statistic;
    //statistic.writeEntry(currentData, currentObjects);

    const float percent = totalData == 0 ? 0 : currentData.ToDouble() * 100 / totalData.ToDouble();

    //write status information to systray, taskbar, parent title ect.
    switch (currentStatus)
    {
    case SyncStatus::SCANNING:
        showProgressExternally(numberToStringSep(scannedObjects) + wxT(" - ") + _("Scanning..."));
        break;
    case SyncStatus::COMPARING_CONTENT:
        showProgressExternally(formatPercentage(currentData, totalData) + wxT(" - ") + _("Comparing content..."), percent);
        break;
    case SyncStatus::SYNCHRONIZING:
        showProgressExternally(formatPercentage(currentData, totalData) + wxT(" - ") + _("Synchronizing..."), percent);
        break;
    case SyncStatus::PAUSE:
        showProgressExternally((totalData != 0 ? formatPercentage(currentData, totalData) + wxT(" - ") : wxString()) + _("Paused"), percent);
        break;
    case SyncStatus::ABORTED:
        showProgressExternally(_("Aborted"), percent);
        break;
    case SyncStatus::FINISHED_WITH_SUCCESS:
    case SyncStatus::FINISHED_WITH_ERROR:
        showProgressExternally(_("Completed"), percent);
        break;
    }

    //write regular status information (whether dialog is visible or not)
    {
        //wxWindowUpdateLocker dummy(this); -> not needed

        bool updateLayout = false; //avoid screen flicker by calling layout() only if necessary

        //progress indicator
        if (currentStatus == SyncStatus::SCANNING)
            m_gauge1->Pulse();
        else
            m_gauge1->SetValue(globalFunctions::round(currentData.ToDouble() * scalingFactor));

        //status text
        const wxString statusTxt = zToWx(currentStatusText);
        if (m_textCtrlInfo->GetValue() != statusTxt) //no layout update for status texts!
            m_textCtrlInfo->ChangeValue(statusTxt);

        //remaining objects
        const wxString remainingObjTmp = numberToStringSep(totalObjects - currentObjects);
        setNewText(remainingObjTmp, *m_staticTextRemainingObj, updateLayout);

        //remaining bytes left for copy
        const wxString remainingBytesTmp = FreeFileSync::formatFilesizeToShortString(totalData - currentData);
        setNewText(remainingBytesTmp, *m_staticTextDataRemaining, updateLayout);

        if (statistics.get())
        {
            if (timeElapsed.Time() - lastStatCallSpeed >= 500) //call method every 500 ms
            {
                lastStatCallSpeed = timeElapsed.Time();

                statistics->addMeasurement(currentObjects, currentData.ToDouble());

                //current speed
                setNewText(statistics->getBytesPerSecond(), *m_staticTextSpeed, updateLayout);

                if (timeElapsed.Time() - lastStatCallRemTime >= 2000) //call method every two seconds only
                {
                    lastStatCallRemTime = timeElapsed.Time();

                    //remaining time
                    setNewText(statistics->getRemainingTime(), *m_staticTextTimeRemaining, updateLayout);
                }
            }
        }

        //time elapsed
        setNewText(wxTimeSpan::Milliseconds(timeElapsed.Time()).Format(), *m_staticTextTimeElapsed, updateLayout);

        //do the ui update
        if (updateLayout)
        {
            bSizer28->Layout();
            bSizer31->Layout();
        }
    }

    //support for pause button
    while (processPaused && currentProcessIsRunning())
    {
        wxMilliSleep(UI_UPDATE_INTERVAL);
        updateUiNow();
    }

    updateUiNow();
}


bool SyncStatus::SyncStatusImpl::currentProcessIsRunning()
{
    return processStatusHandler != NULL;
}


void SyncStatus::SyncStatusImpl::setCurrentStatus(SyncStatus::SyncStatusID id)
{
    switch (id)
    {
    case SyncStatus::ABORTED:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusError")));
        m_staticTextStatus->SetLabel(_("Aborted"));
        break;

    case SyncStatus::FINISHED_WITH_SUCCESS:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusSuccess")));
        m_staticTextStatus->SetLabel(_("Completed"));
        break;

    case SyncStatus::FINISHED_WITH_ERROR:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusWarning")));
        m_staticTextStatus->SetLabel(_("Completed"));
        break;

    case SyncStatus::PAUSE:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusPause")));
        m_staticTextStatus->SetLabel(_("Paused"));
        break;

    case SyncStatus::SCANNING:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusScanning")));
        m_staticTextStatus->SetLabel(_("Scanning..."));
        break;

    case SyncStatus::COMPARING_CONTENT:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusBinaryCompare")));
        m_staticTextStatus->SetLabel(_("Comparing content..."));
        break;

    case SyncStatus::SYNCHRONIZING:
        m_bitmapStatus->SetBitmap(GlobalResources::getInstance().getImageByName(wxT("statusSyncing")));
        m_staticTextStatus->SetLabel(_("Synchronizing..."));
        break;
    }

    currentStatus = id;
    Layout();
}


void SyncStatus::SyncStatusImpl::processHasFinished(SyncStatus::SyncStatusID id, const wxString& finalMessage) //essential to call this in StatusHandler derived class destructor
{
    //at the LATEST(!) to prevent access to currentStatusHandler
    //enable okay and close events; may be set in this method ONLY

    processStatusHandler = NULL; //avoid callback to (maybe) deleted parent process

    setCurrentStatus(id);

    resumeFromSystray(); //if in tray mode...

    m_buttonAbort->Disable();
    m_buttonAbort->Hide();
    m_buttonPause->Disable();
    m_buttonPause->Hide();
    m_buttonOK->Show();
    m_buttonOK->Enable();

    if (IsShown()) //don't steal focus when residing in sys-tray!
        m_buttonOK->SetFocus();

    m_animationControl1->Stop();
    m_animationControl1->Hide();

    //hide speed and remaining time
    bSizerSpeed  ->Show(false);
    bSizerRemTime->Show(false);

    //if everything was processed successfully, hide remaining statistics (is 0 anyway)
    if (    totalObjects == currentObjects &&
            totalData    == currentData)
    {
        bSizerObjectsRemaining->Show(false);

        bSizerObjectsProcessed->Show(true);

        m_staticTextProcessedObj->SetLabel(numberToStringSep(currentObjects));
        m_staticTextDataProcessed->SetLabel(FreeFileSync::formatFilesizeToShortString(currentData));
    }

    updateStatusDialogNow(); //keep this sequence to avoid display distortion, if e.g. only 1 item is sync'ed
    m_textCtrlInfo->SetValue(finalMessage); //
    Layout();                               //

    //Raise(); -> don't! user may be watching a movie in the meantime ;)
}


void SyncStatus::SyncStatusImpl::OnOkay(wxCommandEvent& event)
{
    Close(); //generate close event: do NOT destroy window unconditionally!
}


void SyncStatus::SyncStatusImpl::OnAbort(wxCommandEvent& event)
{
    Close(); //generate close event: do NOT destroy window unconditionally!
}


void SyncStatus::SyncStatusImpl::OnPause(wxCommandEvent& event)
{
    static SyncStatus::SyncStatusID previousStatus = SyncStatus::ABORTED;

    if (processPaused)
    {
        setCurrentStatus(previousStatus);
        processPaused = false;
        m_buttonPause->SetLabel(_("Pause"));
        m_animationControl1->Play();

        //resume timers
        timeElapsed.Resume();
        if (statistics.get())
            statistics->resumeTimer();
    }
    else
    {
        previousStatus = currentStatus; //save current status

        setCurrentStatus(SyncStatus::PAUSE);
        processPaused = true;
        m_buttonPause->SetLabel(_("Continue"));
        m_animationControl1->Stop();

        //pause timers
        timeElapsed.Pause();
        if (statistics.get())
            statistics->pauseTimer();
    }
}


void SyncStatus::SyncStatusImpl::OnClose(wxCloseEvent& event)
{
    processPaused = false;
    if (currentProcessIsRunning())
    {
        m_buttonAbort->Disable();
        m_buttonAbort->Hide();
        m_buttonPause->Disable();
        m_buttonPause->Hide();

        setStatusText_NoUpdate(wxToZ(_("Abort requested: Waiting for current operation to finish...")));
        //no Layout() or UI-update here to avoid cascaded Yield()-call

        processStatusHandler->requestAbortion();
    }
    else
        Destroy();
}


void SyncStatus::SyncStatusImpl::OnIconize(wxIconizeEvent& event)
{
    if (event.Iconized()) //ATTENTION: iconize event is also triggered on "Restore"! (at least under Linux)
        minimizeToTray();
}


void SyncStatus::SyncStatusImpl::minimizeToTray()
{
    minimizedToSysTray.reset(new MinimizeToTray(this, mainDialog));
}


void SyncStatus::SyncStatusImpl::resumeFromSystray()
{
    minimizedToSysTray.reset();
}
