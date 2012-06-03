# Simple logger interface for Qt powered applications
Designed to impact main application as minimum as possible.
Implemented as shared library.
Contains extensible log writers, so destination of log messages
may be changed by implementing custom log writer. See documentation
for details.

## Build
### Release
Run
    mkdir build
    cd build
    cmake ..
    make
to build release version of shared library

### Debug
Replace
    cmake ..
with
    cmake -DCMAKE_BUILD_TYPE=Debug
to build debug version

### Documentation
*Requires Doxygen and Graphviz (dot util)*.
Replace
    make
with 
    make doc
to build html documentation inside build directory

## License
Licensed under the terms of BSD New License. Copy of license
may be found in LICENSE file.

Uses Qt as shared library, so allows to use LGPL-licensed Qt.

&copy; Ilya Arefiev <arefiev.id@gmail.com>

