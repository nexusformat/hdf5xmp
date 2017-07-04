# Readme

This is a thumbnail-provider for Windows

## Developing

The thumbnail-provider is a Visual Studio 2017 Community Edition project and should work directly by opening the .sln file.

You may need to add the ```common``` Folder to the Include path in the Project for it to compile.

## Install

*   Put CppShellExtThumbnailHandler.dll into any directory.

    IMPORTANT: Don't move it to another directory after this installation.

*   ```cd``` into the folder you put the file in the previous step.
*   Execute ```regsvr32.exe CppShellExtThumbnailHandler.dll```


## Uninstall

*   ```cd``` into the folder with the CppShellExtThumbnailHandler.dll
*   Execute ```regsvr32.exe /u CppShellExtThumbnailHandler.dll```
*   Delete  the CppShellExtThumbnailHandler.dll
