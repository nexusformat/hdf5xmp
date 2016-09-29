/******************************** Module Header ********************************\
Module Name:  HDF5ThumbnailProvider.cpp
Project:      CppShellExtThumbnailHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates the C++ implementation of a thumbnail handler 
for a new file type registered with the .HDF5 extension. 

A thumbnail image handler provides an image to represent the item. It lets you 
customize the thumbnail of files with a specific file extension. Windows Vista 
and newer operating systems make greater use of file-specific thumbnail images 
than earlier versions of Windows. Thumbnails of 32-bit resolution and as large 
as 256x256 pixels are often used. File format owners should be prepared to 
display their thumbnails at that size. 

The example thumbnail handler implements the IInitializeWithStream and 
IThumbnailProvider interfaces, and provides thumbnails for .HDF5 files. 
The .HDF5 file type is simply an XML file registered as a unique file name 
extension. It includes an element called "Picture", embedding an image file. 
The thumbnail handler extracts the embedded image and asks the Shell to 
display it as a thumbnail.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\*******************************************************************************/


#include "HDF5ThumbnailProvider.h"
#include <Shlwapi.h>
#include <Wincrypt.h>   // For CryptStringToBinary.
#include <msxml6.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "msxml6.lib")


extern HINSTANCE g_hInst;
extern long g_cDllRef;


HDF5ThumbnailProvider::HDF5ThumbnailProvider() : m_cRef(1), path("")
{
    InterlockedIncrement(&g_cDllRef);
}


HDF5ThumbnailProvider::~HDF5ThumbnailProvider()
{
    InterlockedDecrement(&g_cDllRef);
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP HDF5ThumbnailProvider::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(HDF5ThumbnailProvider, IThumbnailProvider),
        QITABENT(HDF5ThumbnailProvider, IInitializeWithFile), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) HDF5ThumbnailProvider::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) HDF5ThumbnailProvider::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

#pragma endregion


#pragma region IInitializeWithFile

HRESULT HDF5ThumbnailProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode) {
	HRESULT hr = HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
	std::wstring tempstring = pszFilePath;
	path = std::string(tempstring.begin(), tempstring.end());
	hr = 1;
	return hr;
}

#pragma endregion


#pragma region IThumbnailProvider

IFACEMETHODIMP HDF5ThumbnailProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, 
    WTS_ALPHATYPE *pdwAlpha)
{
	pdwAlpha = 0;
	H5::H5File file(path, H5F_ACC_RDONLY);

	// Gets the dataset with the thumbnail
	H5::DataSet dataset = file.openDataSet(DATA_SET);

	// Get image size and build char array of that size
	unsigned int size = dataset.getSpace().getSimpleExtentNpoints();
	char *imageData = new char[size];

	// read image into imageData
	dataset.read(imageData, dataset.getDataType());

	// write image to file

	std::string outfile = path + ".png";

	// Creates tempfile name
	outfile += "temporarythumbnailerfile~";

	std::fstream imageFile(outfile, std::ios::out | std::ios::binary);
	imageFile.write(imageData, size);
	imageFile.close();

	// Gets the path as wstring and converts that to LPCWSTR... Because Windows
	std::wstring outstr(outfile.begin(), outfile.end());
	LPCWSTR pathAsWindowsString;

	pathAsWindowsString = (LPCWSTR)outstr.c_str();

	// Sets the tempfile to hidden
	SetFileAttributes(pathAsWindowsString, FILE_ATTRIBUTE_HIDDEN);

	Gdiplus::GdiplusStartupInput startupInput;
	ULONG_PTR token;
	Gdiplus::GdiplusStartup(&token, &startupInput, NULL);


	std::cout << outfile << std::endl;

	std::cout << outstr.c_str() << std::endl;

	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(outstr.c_str(), false);

	bitmap->GetHBITMAP(Gdiplus::Color::White, phbmp);

	// cleanup
	delete[] imageData;
	file.close();

	DeleteFile((LPCWSTR)outfile.c_str());
	return S_OK;
}

#pragma endregion


#pragma region Helper Functions

#pragma endregion