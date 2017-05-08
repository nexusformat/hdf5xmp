# HDF-5 Thumbnailer

This program's designed to provide thumbnailing services to file managers on Linux and Windows.

It contains:

    * A Python script that you can use to create HDF-5 files with thumbnails in them.
    * A thumbnail Provider for the file managers Dolphin, Nautilus and Explorer.
    * Example HDF-5 files with thumbnails inserted into them.


## Project structure

The folder _python_ contains the python-script to insert thumbnails.

_examples_ contains HDF-5 files with thumbnails inserted into them.

The _linux_ folder contains the files for the Linux versions.

The _windows_ folder contains the Visual-Studio project for the thumbnailer.


## How the thumbnailer works

The thumbnailer works by using the HDF-5 User Block
available at the beginning of every HDF-5 file.

### Thumbnailed file structure

The first 4 bytes define the size of the thumbnail.
They are directly followed by the image itself.

The image format can technically be any by the file managers recognized format.
But to ensure that it will work on any operating system JPEG, GIF or PNG are the recommended formats
depending on what compresses the best for the current image.

### Thumbnail extraction

When the thumbnailer gets called it first checks the first 4 bytes to see if it even has a thumbnail in it.
It does that by comparing the bytes to the magic number of HDF-5 files.
If these two match it exits and reports a failure to let the file manager know that there can't be any thumbnail extracted.

If the requested file is not a HDF-5 file without thumbnail then it continues by reading the next 4 bytes.
It then compares those 4 bytes with the magic numbers of supported image formats.
If it matches any of those it knows that this is a thumbnailed HDF-5 file and continues the execution.
Else it will once again exit the program with a failure exit code.

When the thumbnailer knows it has a thumbnailed HDF-5 file it will read the image data into memory
according to the file size written in the first 4 bytes.
What happens next is different for each thumbnailer version.

    * The thumbnailer for Dolphin reads the data into a QImage object and pass it to the file manager directly in memory.
    * The thumbnailer for Nautilus writes the extracted image to the by the file manager specified location.
    * The thumbnailer for Windows reads the data into a HBITMAP and pass it to the file manager directly in memory.


## Building

### Linux

The Linux version supports the file managers of GNOME and KDE environments, meaning you need to have either of them.

The KDE version uses some QT libraries, but KDE already comes with those installed.
The GNOME thumbnailer doesn't use any libraries at all.

If you only want one of the versions you should disable the other one (especially if you use GNOME and don't have QT installed),
this is done by using CMake arguments.

```
  -Dkde       # Whether or not to build the KDE Version (Default: -Dkde=ON, To disable: -Dkde=OFF)
  -Dgnome     # Whether or not to build the GNOME Version (Default: -Dgnome=ON, To disable: -Dgnome=OFF)
```

To build the thumbnailer you need to create a _build_ folder and then execute CMake with the appropriate arguments.

After building the thumbnailer it's a simple matter of using ```sudo make install``` in the build directory.

Example for GNOME only:

```
  cd linux
  mkdir build
  cd build
  cmake -Dkde=off ..
  sudo make install
```

### Windows

To build the Windows version you need to have Visual-Studio 2017 and VCRedist installed.
To build the installer for the thumbnailer you need the
[_Microsoft Visual Studio 2017 Installer Projects_](https://marketplace.visualstudio.com/items?itemName=VisualStudioProductTeam.MicrosoftVisualStudio2017InstallerProjects) plugin for Visual-Studio.

When you have the requirements installed, you can open the solution in the _windows_ folder with Visual-Studio.

From there you can build the thumbnailer and the installer which you can execute to install the thumbnailer.

