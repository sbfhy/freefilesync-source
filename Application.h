/***************************************************************
 * Name:      FreeFileSyncApp.h
 * Purpose:   Defines Application Class
 * Author:    ZenJu (zhnmju123@gmx.de)
 * Created:   2008-07-16
 * Copyright: ZenJu ()
 * License:
 **************************************************************/

#ifndef FREEFILESYNCAPP_H
#define FREEFILESYNCAPP_H

#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/ffile.h>
#include "FreeFileSync.h"
#include "ui/smallDialogs.h"
#include "library/misc.h"
#include <wx/stopwatch.h>

class Application : public wxApp
{
public:
    bool OnInit();
    int  OnRun();
    int  OnExit();

    void initialize();
    bool ProcessIdle(); //virtual method

    //methods for writing logs
    void logInit();
    void logWrite(const wxString& logText, const wxString& problemType = wxEmptyString);
    void logClose(const wxString& finalText);

private:
    void parseCommandline();

    bool applicationRunsOnCommandLineWithoutWindows;
    wxFFile logFile;
    wxStopWatch totalTime;
    CustomLocale programLanguage;

    int returnValue;
};


class CommandLineStatusUpdater : public StatusUpdater
{
public:
    CommandLineStatusUpdater(Application* application, bool continueOnError, bool silent);
    ~CommandLineStatusUpdater();

    void updateStatusText(const wxString& text);
    void initNewProcess(int objectsTotal, double dataTotal, int processID);
    void updateProcessedData(int objectsProcessed, double dataProcessed);
    int reportError(const wxString& text);
    void triggerUI_Refresh(bool asyncProcessActive);

    void noSynchronizationNeeded();

private:
    Application* app;
    SyncStatus* syncStatusFrame;
    bool continueErrors;
    bool silentMode;

    wxArrayString unhandledErrors;   //list of non-resolved errors
    int currentProcess;
    bool synchronizationNeeded;
};


#endif // FREEFILESYNCAPP_H
