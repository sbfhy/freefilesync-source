// **************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under *
// * GNU General Public License: http://www.gnu.org/licenses/gpl.html       *
// * Copyright (C) 2008-2011 ZenJu (zhnmju123 AT gmx.de)                    *
// **************************************************************************
//
#ifndef RECYCLER_H_INCLUDED
#define RECYCLER_H_INCLUDED

#include "file_error.h"
#include "zstring.h"

namespace zen
{
/*
--------------------
|Recycle Bin Access|
--------------------

Windows
-------
Recycler always available: during runtime either SHFileOperation or (since Vista) IFileOperation will be dynamically selected

Linux
-----
Include compilation flag:
`pkg-config --cflags gtkmm-2.4`

Linker flag:
`pkg-config --libs gtkmm-2.4`
*/

bool recycleBinExists(); //test existence of Recycle Bin API on current system

//move a file or folder to Recycle Bin
bool moveToRecycleBin(const Zstring& fileToDelete); //return "true" if file/dir was actually deleted; throw (FileError)
}

#endif // RECYCLER_H_INCLUDED
