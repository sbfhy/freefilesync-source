// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) Zenju (zenju AT gmx DOT de) - All Rights Reserved        *
// **************************************************************************

#ifndef PROCESSXML_H_INCLUDED
#define PROCESSXML_H_INCLUDED

#include <wx/gdicmn.h>
#include "../structures.h"
#include "xml_base.h"
#include "localization.h"
#include "../ui/column_attr.h"
//#include "ffs_paths.h"

namespace xmlAccess
{
enum XmlType
{
    XML_TYPE_GUI,
    XML_TYPE_BATCH,
    XML_TYPE_GLOBAL,
    XML_TYPE_OTHER
};

XmlType getXmlType(const Zstring& filename); //throw()


enum OnError
{
    ON_ERROR_POPUP,
    ON_ERROR_IGNORE,
    ON_ERROR_EXIT
};

enum OnGuiError
{
    ON_GUIERROR_POPUP,
    ON_GUIERROR_IGNORE
};

typedef std::wstring Description;
typedef std::wstring Commandline;
typedef std::vector<std::pair<Description, Commandline> > ExternalApps;

//---------------------------------------------------------------------
struct XmlGuiConfig
{
    XmlGuiConfig() :
        showFilteredElements(true),
        handleError(ON_GUIERROR_POPUP),
        showSyncAction(true) {} //initialize values

    zen::MainConfiguration mainCfg;

    bool showFilteredElements;
    OnGuiError handleError; //reaction on error situation during synchronization
    bool showSyncAction;
};


inline
bool operator==(const XmlGuiConfig& lhs, const XmlGuiConfig& rhs)
{
    return lhs.mainCfg              == rhs.mainCfg              &&
           lhs.showFilteredElements == rhs.showFilteredElements &&
           lhs.handleError          == rhs.handleError          &&
           lhs.showSyncAction       == rhs.showSyncAction;
}


struct XmlBatchConfig
{
    XmlBatchConfig() :
        showProgress(true),
        logfilesCountLimit(-1),
        handleError(ON_ERROR_POPUP) {}

    zen::MainConfiguration mainCfg;

    bool showProgress;
    Zstring logFileDirectory;
    int logfilesCountLimit; //max logfiles; 0 := don't save logfiles; < 0 := no limit
    OnError handleError;    //reaction on error situation during synchronization
};


struct OptionalDialogs
{
    OptionalDialogs() { resetDialogs();}

    void resetDialogs();

    bool warningDependentFolders;
    bool warningFolderPairRaceCondition;
    bool warningSignificantDifference;
    bool warningNotEnoughDiskSpace;
    bool warningUnresolvedConflicts;
    bool warningDatabaseError;
    bool warningRecyclerMissing;
    bool popupOnConfigChange;
    bool confirmSyncStart;
};


enum FileIconSize
{
    ICON_SIZE_SMALL,
    ICON_SIZE_MEDIUM,
    ICON_SIZE_LARGE
};


wxString getGlobalConfigFile();

struct XmlGlobalSettings
{
    //---------------------------------------------------------------------
    //Shared (GUI/BATCH) settings
    XmlGlobalSettings() :
        programLanguage(zen::retrieveSystemLanguage()),
        copyLockedFiles(true),
        copyFilePermissions(false),
        runWithBackgroundPriority(false),
        fileTimeTolerance(2),  //default 2s: FAT vs NTFS
        lastSyncsLogFileSizeMax(100000), //maximum size for LastSyncs.log: use a human-readable number
        verifyFileCopy(false),
        transactionalFileCopy(true),
        createLockFile(true) {}

    int programLanguage;
    bool copyLockedFiles;          //VSS usage
    bool copyFilePermissions;

    bool runWithBackgroundPriority;

    size_t fileTimeTolerance; //max. allowed file time deviation
    size_t lastSyncsLogFileSizeMax;
    bool verifyFileCopy;   //verify copied files
    bool transactionalFileCopy;
    bool createLockFile;

    OptionalDialogs optDialogs;

    //---------------------------------------------------------------------
    struct Gui
    {
        Gui() :
            dlgPos(wxDefaultCoord, wxDefaultCoord),
            dlgSize(wxDefaultCoord, wxDefaultCoord),
            isMaximized(false),
            sashOffset(0),
            maxFolderPairsVisible(6),
            columnAttribNavi (zen::getDefaultColumnAttributesNavi()),
            columnAttribLeft (zen::getDefaultColumnAttributesLeft()),
            columnAttribRight(zen::getDefaultColumnAttributesRight()),
            naviLastSortColumn(zen::defaultValueLastSortColumn),
            naviLastSortAscending(zen::defaultValueLastSortAscending),
            showPercentBar(zen::defaultValueShowPercentage),
            cfgFileHistMax(30),
            folderHistMax(15),
            onCompletionHistoryMax(8),
            deleteOnBothSides(false),
            useRecyclerForManualDeletion(true), //enable if OS supports it; else user will have to activate first and then get an error message
#ifdef FFS_WIN
            textSearchRespectCase(false),
#elif defined FFS_LINUX
            textSearchRespectCase(true),
#endif
            showIcons(true),
            iconSize(ICON_SIZE_SMALL),
            lastUpdateCheck(0)
        {
            //default external apps will be translated "on the fly"!!! First entry will be used for [Enter] or mouse double-click!
#ifdef FFS_WIN
            externelApplications.push_back(std::make_pair(L"Show in Explorer", //mark for extraction: _("Show in Explorer")
                                                          L"explorer /select, \"%item_path%\""));
            externelApplications.push_back(std::make_pair(L"Open with default application", //mark for extraction: _("Open with default application")
                                                          L"\"%item_path%\""));
#elif defined FFS_LINUX
            externelApplications.push_back(std::make_pair(L"Browse directory", //mark for extraction: _("Browse directory") Linux doesn't use the term "folder"
                                                          L"xdg-open \"%item_folder%\""));
            externelApplications.push_back(std::make_pair(L"Open with default application", //mark for extraction: _("Open with default application")
                                                          L"xdg-open \"%item_path%\""));
#endif
        }

        wxPoint dlgPos;
        wxSize dlgSize;
        bool isMaximized;
        int sashOffset;

        int maxFolderPairsVisible;

        std::vector<zen::ColumnAttributeNavi> columnAttribNavi; //compressed view/navigation
        std::vector<zen::ColumnAttributeRim>  columnAttribLeft;
        std::vector<zen::ColumnAttributeRim>  columnAttribRight;

        zen::ColumnTypeNavi naviLastSortColumn; //remember sort on navigation panel
        bool naviLastSortAscending; //

        bool showPercentBar; //in navigation panel

        ExternalApps externelApplications;

        std::vector<wxString> cfgFileHistory;
        size_t cfgFileHistMax;

        std::vector<wxString> lastUsedConfigFiles;

        std::vector<Zstring> folderHistoryLeft;
        std::vector<Zstring> folderHistoryRight;
        size_t folderHistMax;

        std::vector<std::wstring> onCompletionHistory;
        size_t onCompletionHistoryMax;

        bool deleteOnBothSides;
        bool useRecyclerForManualDeletion;
        bool textSearchRespectCase;

        bool showIcons;
        FileIconSize iconSize;

        long lastUpdateCheck;          //time of last update check

        wxString guiPerspectiveLast; //used by wxAuiManager
    } gui;

    //---------------------------------------------------------------------
    //struct Batch
};

void readConfig(const Zstring& filename, XmlGuiConfig&      config); //
void readConfig(const Zstring& filename, XmlBatchConfig&    config); //throw FfsXmlError
void readConfig(                         XmlGlobalSettings& config); //

void writeConfig(const XmlGuiConfig&      config, const Zstring& filename); //
void writeConfig(const XmlBatchConfig&    config, const Zstring& filename); //throw FfsXmlError
void writeConfig(const XmlGlobalSettings& config);                          //

//config conversion utilities
XmlGuiConfig   convertBatchToGui(const XmlBatchConfig& batchCfg);
XmlBatchConfig convertGuiToBatch(const XmlGuiConfig& guiCfg, const Zstring& referenceFile);


//convert (multiple) *.ffs_gui, *.ffs_batch files or combinations of both into target config structure:
enum MergeType
{
    MERGE_GUI,       //pure gui config files
    MERGE_BATCH,     //  " batch   "
    MERGE_GUI_BATCH, //gui and batch files
    MERGE_OTHER
};
MergeType getMergeType(const std::vector<Zstring>& filenames);   //throw ()

void mergeConfigs(const std::vector<Zstring>& filenames, XmlGuiConfig&   config); //throw xmlAccess::FfsXmlError
void mergeConfigs(const std::vector<Zstring>& filenames, XmlBatchConfig& config); //throw xmlAccess::FfsXmlError

std::wstring extractJobName(const Zstring& configFilename);
}


#endif // PROCESSXML_H_INCLUDED
