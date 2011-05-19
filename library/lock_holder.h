#ifndef LOCK_HOLDER_H_INCLUDED
#define LOCK_HOLDER_H_INCLUDED

#include <map>
#include "../shared/Zstring.h"
#include "dir_lock.h"
#include "status_handler.h"

namespace zen
{
const Zstring LOCK_FILE_ENDING = Zstr("ffs_lock"); //intermediate locks created by DirLock use this extension, too!

//convenience class for creating and holding locks for a number of directories
class LockHolder
{
public:
    void addDir(const Zstring& dirnameFmt, ProcessCallback& procCallback) //resolved dirname ending with path separator
    {
        if (dirnameFmt.empty()) return;
        if (lockHolder.find(dirnameFmt) != lockHolder.end()) return;
        assert(dirnameFmt.EndsWith(common::FILE_NAME_SEPARATOR)); //this is really the contract, formatting does other things as well, e.g. macro substitution

        class WaitOnLockHandler : public DirLockCallback
        {
        public:
            WaitOnLockHandler(ProcessCallback& pc) : pc_(pc) {}
            virtual void requestUiRefresh() { pc_.requestUiRefresh(); }  //allowed to throw exceptions
            virtual void reportInfo(const Zstring& text) { pc_.reportInfo(text); }
        private:
            ProcessCallback& pc_;
        } callback(procCallback);

        try
        {
            lockHolder.insert(std::make_pair(dirnameFmt, DirLock(dirnameFmt + Zstr("sync.") + LOCK_FILE_ENDING, &callback)));
        }
        catch (const FileError& e)
        {
            bool dummy = false; //this warning shall not be shown but logged only
            procCallback.reportWarning(e.msg(), dummy);
        }
    }

private:
    typedef std::map<Zstring, DirLock, LessFilename> DirnameLockMap;
    DirnameLockMap lockHolder;
};

}


#endif // LOCK_HOLDER_H_INCLUDED
