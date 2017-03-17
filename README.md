# HDF-5 Thumbnailer

The folder _python_ contains two python-scripts (One is not working right now) to insert and extract thumbnails.

_hdf5files_ contains two HDF-5 files with thumbnails inserted into them.

The _linux_ folder contains the files for the Linux versions

The _windows_ folder contains the visual-studio project for the thumbnailer.



## Building

### Linux

To build the Linux version you need to use CMake.

*   Create a _build_ folder in the ```linux/cpp``` folder and ```cd``` into it.
*   execute ```cmake ..```
*   ```make install```

#### CMake arguments

```
  -DKDE       # Wether or not to build the KDE Version (Default: ON)
  -DGNOME     # Wether or not to build the GNOME Version (Default: ON)
```


### Windows

To build the Windows version you need to open the project solution in Visual Studio and build it.
