/****************************** Module Header ******************************\
Module Name:  dllmain.cpp
Project:      CppShellExtThumbnailHandler
Copyright (c) Microsoft Corporation.

The file implements DllMain, and the DllGetClassObject, DllCanUnloadNow, 
DllRegisterServer, DllUnregisterServer functions that are necessary for a COM 
DLL. 

DllGetClassObject invokes the class factory defined in ClassFactory.h/cpp and 
queries to the specific interface.

DllCanUnloadNow checks if we can unload the component from the memory.

DllRegisterServer registers the COM server and the thumbnail handler in the 
registry by invoking the helper functions defined in Reg.h/cpp. The thumbnail 
handler is associated with the .HDF5 file class.

DllUnregisterServer unregisters the COM server and the thumbnail handler. 

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include <windows.h>
#include <Guiddef.h>
#include <shlobj.h>                 // For SHChangeNotify
#include "ClassFactory.h"           // For the class factory
#include "Reg.h"


// When you write your own handler, you must create a new CLSID by using the 
// "Create GUID" tool in the Tools menu, and specify the CLSID value here.
// {FFBF393D-63EE-4570-8DB6-32BABB555608}
const CLSID CLSID_HDF5ThumbnailProvider =
{ 0xffbf393d, 0x63ee, 0x4570,{ 0x8d, 0xb6, 0x32, 0xba, 0xbb, 0x55, 0x56, 0x8 } };



HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
        // Hold the instance of this DLL module, we will use it to get the 
        // path of the DLL to register the component.
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//
//   FUNCTION: DllGetClassObject
//
//   PURPOSE: Create the class factory and query to the specific interface.
//
//   PARAMETERS:
//   * rclsid - The CLSID that will associate the correct data and code.
//   * riid - A reference to the identifier of the interface that the caller 
//     is to use to communicate with the class object.
//   * ppv - The address of a pointer variable that receives the interface 
//     pointer requested in riid. Upon successful return, *ppv contains the 
//     requested interface pointer. If an error occurs, the interface pointer 
//     is NULL. 
//
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	std::cout << "Getting Class Object" << std::endl;
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(CLSID_HDF5ThumbnailProvider, rclsid))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory *pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}


//
//   FUNCTION: DllCanUnloadNow
//
//   PURPOSE: Check if we can unload the component from the memory.
//
//   NOTE: The component can be unloaded from the memory when its reference 
//   count is zero (i.e. nobody is still using the component).
// 
STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}


//
//   FUNCTION: DllRegisterServer
//
//   PURPOSE: Register the COM server and the thumbnail handler.
// 
STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Register the component.
    hr = RegisterInprocServer(szModule, CLSID_HDF5ThumbnailProvider, 
        L"CppShellExtThumbnailHandler.HDF5ThumbnailProvider Class", 
        L"Apartment");
    if (SUCCEEDED(hr))
    {
        // Register the thumbnail handler. The thumbnail handler is associated
        // with the .HDF5 file class.
        hr = RegisterShellExtThumbnailHandler(L".hdf5", 
            CLSID_HDF5ThumbnailProvider);
        if (SUCCEEDED(hr))
        {
			std::cout << "Thumbnail handler registered. Now invalidating thumbnail cache" << std::endl;
            // This tells the shell to invalidate the thumbnail cache. It is 
            // important because any .HDF5 files viewed before registering 
            // this handler would otherwise show cached blank thumbnails.
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        }
    }

    return hr;
}


//
//   FUNCTION: DllUnregisterServer
//
//   PURPOSE: Unregister the COM server and the thumbnail handler.
// 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Unregister the component.
    hr = UnregisterInprocServer(CLSID_HDF5ThumbnailProvider);
    if (SUCCEEDED(hr))
    {
        // Unregister the thumbnail handler.
        hr = UnregisterShellExtThumbnailHandler(L".hdf5");
		if (SUCCEEDED(hr)) {
			std::cout << "Succesfully unregistered thumbnail handler!" << std::endl;
		}
    }

    return hr;
}