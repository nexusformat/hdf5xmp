/****************************** Module Header ******************************\
Module Name:  HDF5ThumbnailProvider.h
Project:      CppShellExtThumbnailHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates the C++ implementation of a thumbnail handler 
for a new file type registered with the .HDF5 extension. 

A thumbnail image handler provides an image to represent the item. It lets 
you customize the thumbnail of files with a specific file extension. Windows 
Vista and newer operating systems make greater use of file-specific thumbnail 
images than earlier versions of Windows. Thumbnails of 32-bit resolution and 
as large as 256x256 pixels are often used. File format owners should be 
prepared to display their thumbnails at that size. 

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
\***************************************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <windows.h>
#include <intrin.h>
#include <gdiplus.h>
#include <atlstr.h>
#include <Shlwapi.h>
#include <string>
#include <vector>
#include <thumbcache.h>     // For IThumbnailProvider
#include <wincodec.h>       // Windows Imaging Codecs

#pragma comment(lib, "windowscodecs.lib")

// Defines the magic header value of different file-types
#define MAGIC_HDF  0x89484446
#define MAGIC_JPG  0xffd8ffd8
#define MAGIC_JFIF 0xffd8ffe0
#define MAGIC_EXIF 0xffd8ffe1
// This isn't the full magic numberbut there isn't any other starting like that anyways
#define MAGIC_GIF  0x47494638
#define MAGIC_PNG  0x89504e47

class HDF5ThumbnailProvider : 
    public IInitializeWithFile,
    public IThumbnailProvider
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IInitializeWithStream
	IFACEMETHODIMP Initialize(LPCWSTR pszFilePath, DWORD grfMode);

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

    HDF5ThumbnailProvider();

protected:
    ~HDF5ThumbnailProvider();

private:

	Gdiplus::Bitmap* LoadImageFromFileWithoutLocking(const WCHAR* fileName);
    // Reference count of component.
    long m_cRef;

    // Provided during initialization.
    //IStream *m_pStream;
	std::string path;
};