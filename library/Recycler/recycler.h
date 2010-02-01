#ifndef RECYCLER_DLL_H
#define RECYCLER_DLL_H

#ifdef RECYCLER_DLL_EXPORTS
#define RECYCLER_DLL_API extern "C" __declspec(dllexport)
#else
#define RECYCLER_DLL_API extern "C" __declspec(dllimport)
#endif


namespace Utility
{
//COM needs to be initialized before calling any of these functions! CoInitializeEx/CoUninitialize

RECYCLER_DLL_API
bool moveToRecycleBin(const wchar_t* fileNames[],
                      size_t         fileNo, //size of fileNames array
                      wchar_t*		 errorMessage,
                      size_t         errorBufferLen);
}



#endif //RECYCLER_DLL_H
