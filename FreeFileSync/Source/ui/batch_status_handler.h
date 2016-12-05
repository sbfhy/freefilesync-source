// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef BATCH_STATUS_HANDLER_H_857390451451234566
#define BATCH_STATUS_HANDLER_H_857390451451234566

#include <zen/error_log.h>
#include <zen/time.h>
#include "progress_indicator.h"
#include "../lib/status_handler.h"
#include "../lib/process_xml.h"
#include "../lib/return_codes.h"


//Exception classes used to abort the "compare" and "sync" process
class BatchAbortProcess {};
class BatchRequestSwitchToMainDialog {};

//BatchStatusHandler(SyncProgressDialog) will internally process Window messages! disable GUI controls to avoid unexpected callbacks!
class BatchStatusHandler : public zen::StatusHandler //throw BatchAbortProcess, BatchRequestSwitchToMainDialog
{
public:
    BatchStatusHandler(bool showProgress, //defines: -start minimized and -quit immediately when finished
                       const std::wstring& jobName, //should not be empty for a batch job!
                       const Zstring& soundFileSyncComplete,
                       const zen::TimeComp& timeStamp,
                       const Zstring& logFolderPathPhrase,
                       int logfilesCountLimit, //0: logging inactive; < 0: no limit
                       size_t lastSyncsLogFileSizeMax,
                       const xmlAccess::OnError handleError,
                       size_t automaticRetryCount,
                       size_t automaticRetryDelay,
                       zen::FfsReturnCode& returnCode,
                       const Zstring& onCompletion,
                       std::vector<Zstring>& onCompletionHistory);
    ~BatchStatusHandler();

    void initNewPhase       (int itemsTotal, std::int64_t bytesTotal, Phase phaseID) override;
    void updateProcessedData(int itemsDelta, std::int64_t bytesDelta)                override;
    void reportInfo         (const std::wstring& text)                               override;
    void forceUiRefresh     ()                                                       override;

    void     reportWarning   (const std::wstring& warningMessage, bool& warningActive) override;
    Response reportError     (const std::wstring& errorMessage, size_t retryNumber   ) override;
    void     reportFatalError(const std::wstring& errorMessage                       ) override;

    void abortProcessNow() override final; //throw BatchAbortProcess

private:
    void onProgressDialogTerminate();

    bool showFinalResults_;
    bool switchToGuiRequested_ = false;
    const int logfilesCountLimit_;
    const size_t lastSyncsLogFileSizeMax_;
    xmlAccess::OnError handleError_;
    zen::ErrorLog errorLog_; //list of non-resolved errors and warnings
    zen::FfsReturnCode& returnCode_;

    const size_t automaticRetryCount_;
    const size_t automaticRetryDelay_;

    SyncProgressDialog* progressDlg_; //managed to have shorter lifetime than this handler!

    const std::wstring jobName_;
    const zen::TimeComp timeStamp_;
    const time_t startTime_ = std::time(nullptr); //don't use wxStopWatch: may overflow after a few days due to ::QueryPerformanceCounter()

    const Zstring logFolderPathPhrase_;
};

#endif //BATCH_STATUS_HANDLER_H_857390451451234566
