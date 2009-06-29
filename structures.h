#ifndef FREEFILESYNC_H_INCLUDED
#define FREEFILESYNC_H_INCLUDED

#include <wx/string.h>
#include <set>
#include <vector>
#include "library/zstring.h"
#include <wx/longlong.h>


namespace FreeFileSync
{
//------------------------------------------------------------------------------
//global variables
//------------------------------------------------------------------------------
    extern const wxChar FILE_NAME_SEPARATOR;

    //language dependent global variables: need to be initialized by CustomLocale on program startup and language switch
    extern const wxChar* DECIMAL_POINT;
    extern const wxChar* THOUSANDS_SEPARATOR;

    const wxString& getLastConfigFile();
    const wxString& getGlobalConfigFile();
    const wxString& getDefaultLogDirectory();
    const wxString& getLastErrorTxtFile();
    const wxString& getInstallationDir(); //FreeFileSync installation directory without path separator
//------------------------------------------------------------------------------

    enum CompareVariant
    {
        CMP_BY_CONTENT,
        CMP_BY_TIME_SIZE
    };


    enum SyncDirection
    {
        SYNC_DIR_LEFT,
        SYNC_DIR_RIGHT,
        SYNC_DIR_NONE,

        SYNC_UNRESOLVED_CONFLICT
    };


    struct SyncConfiguration
    {
        SyncConfiguration() :
                exLeftSideOnly(SYNC_DIR_RIGHT),
                exRightSideOnly(SYNC_DIR_LEFT),
                leftNewer(SYNC_DIR_RIGHT),
                rightNewer(SYNC_DIR_LEFT),
                different(SYNC_DIR_NONE) {}

        SyncDirection exLeftSideOnly;
        SyncDirection exRightSideOnly;
        SyncDirection leftNewer;
        SyncDirection rightNewer;
        SyncDirection different;

        bool operator==(const SyncConfiguration& other) const
        {
            return exLeftSideOnly  == other.exLeftSideOnly  &&
                   exRightSideOnly == other.exRightSideOnly &&
                   leftNewer       == other.leftNewer       &&
                   rightNewer      == other.rightNewer      &&
                   different       == other.different;
        }

        //get/set default configuration variants
        enum Variant
        {
            MIRROR,
            UPDATE,
            TWOWAY,
            CUSTOM
        };
        Variant getVariant();
        wxString getVariantName();
        void setVariant(const Variant var);
    };


    struct MainConfiguration
    {
        MainConfiguration();

        //Compare setting
        CompareVariant compareVar;

        //Synchronisation settings
        SyncConfiguration syncConfiguration;

        //Filter setting
        bool filterIsActive;
        wxString includeFilter;
        wxString excludeFilter;

        //misc options
        bool useRecycleBin; //use Recycle bin when deleting or overwriting files while synchronizing

        bool operator==(const MainConfiguration& other) const
        {
            return compareVar        == other.compareVar &&
                   syncConfiguration == other.syncConfiguration &&
                   filterIsActive    == other.filterIsActive &&
                   includeFilter     == other.includeFilter &&
                   excludeFilter     == other.excludeFilter;
        }
    };


    struct FolderPair
    {
        FolderPair() {}

        FolderPair(const Zstring& leftDir, const Zstring& rightDir) :
                leftDirectory(leftDir),
                rightDirectory(rightDir) {}

        Zstring leftDirectory;
        Zstring rightDirectory;

        bool operator==(const FolderPair& other) const
        {
            return leftDirectory  == other.leftDirectory &&
                   rightDirectory == other.rightDirectory;
        }
    };


    struct FileDescrLine
    {
        FileDescrLine() : objType(TYPE_NOTHING) {}

        enum ObjectType
        {
            TYPE_NOTHING,
            TYPE_DIRECTORY,
            TYPE_FILE
        };

        Zstring fullName;  // == directory + relativeName
        Zsubstr relativeName; //fullName without directory that is being synchronized
        //Note on performance: Keep redundant information "relativeName"!
        //Extracting info from "fullName" instead would result in noticeable performance loss, with only limited memory reduction (note ref. counting strings)!
        wxLongLong lastWriteTimeRaw; //number of seconds since Jan. 1st 1970 UTC, same semantics like time_t (== signed long)
        wxULongLong fileSize;
        ObjectType objType; //is it a file or directory or initial?

        bool operator < (const FileDescrLine& b) const //used by template class "set"
        {
            //quick check based on string length: we are not interested in a lexicographical order!
            const size_t aLength = relativeName.length();
            const size_t bLength = b.relativeName.length();
            if (aLength != bLength)
                return aLength < bLength;
#ifdef FFS_WIN //Windows does NOT distinguish between upper/lower-case
            return FreeFileSync::compareStringsWin32(relativeName.c_str(), b.relativeName.c_str()) < 0;  //implementing a (reverse) comparison manually is a lot slower!
#elif defined FFS_LINUX //Linux DOES distinguish between upper/lower-case
            return defaultCompare(relativeName.c_str(), b.relativeName.c_str()) < 0;
#endif
        }
    };
    typedef std::vector<FileDescrLine> DirectoryDescrType;


    enum CompareFilesResult
    {
        FILE_LEFT_SIDE_ONLY,
        FILE_RIGHT_SIDE_ONLY,
        FILE_LEFT_NEWER,
        FILE_RIGHT_NEWER,
        FILE_DIFFERENT,
        FILE_EQUAL,

        FILE_CONFLICT,
    };


    struct FileCompareLine
    {
        FileCompareLine(const CompareFilesResult defaultCmpResult,
                        const SyncDirection defaultDirection,
                        const bool selected) :
                cmpResult(defaultCmpResult),
                direction(defaultDirection),
                selectedForSynchronization(selected) {}

        typedef unsigned int FolderPairNr;

        FolderPairNr folderPairRef;

        FileDescrLine fileDescrLeft;
        FileDescrLine fileDescrRight;

        CompareFilesResult cmpResult;
        SyncDirection      direction;
        bool selectedForSynchronization;
    };
    typedef std::vector<FileCompareLine> FileComparison;


    struct FileComparison2
    {

        std::vector<FolderPair> directoryPairs; //directoryPairs - cmpLines 1:n

        std::vector<FileCompareLine> cmpLines;
    };




    struct FolderCompareLine //support for multiple folder pairs
    {
        FolderPair syncPair;  //directories to be synced (ending with separator)
        FileComparison fileCmp;

        void swap(FolderCompareLine& other)
        {
            std::swap(syncPair, other.syncPair);
            fileCmp.swap(other.fileCmp);
        }
    };
    typedef std::vector<FolderCompareLine> FolderComparison;

    //References to single lines(in FileComparison) inside FolderComparison
    typedef std::vector<std::set<int> > FolderCompRef;


    class AbortThisProcess  //Exception class used to abort the "compare" and "sync" process
    {
    public:
        AbortThisProcess() {}
        ~AbortThisProcess() {}
    };
}

#endif // FREEFILESYNC_H_INCLUDED
