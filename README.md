This is unofficial SObjectizer https://sourceforge.net/p/sobjectizer/ repository mirror.

SObjectizer is a framework for agent-oriented (a.k.a. actor model) programming
 in C++ (like Theron or C++ Actor Framework (CAF) for C++ and Akka for Java).
SObjectizer is a set of rules and principles for designing programms in
agent-oriented style. Also it provides set of C++ classes for implementing
agents, mailboxes for message exchange, different dispatcher types for running
agents and working environment. 

Supported platforms: Linux, Windows, FreeBSD.

Building
========

on Linux:
```
  git clone https://github.com/masterspline/SObjectizer.git so
  mkdir build
  cd build
  cmake -DBUILD_ALL=ON -DCMAKE_INSTALL_PREFIX=installed_files ../so/dev
  make
  make test
  make install
```
For more detailed info about cmake build system for SObjectizer and avalable params
see dev/cmake/CmakeQuickHowto.txt

NOTE. cmake is experimental building solution for SObjectizer, not supported 
by SObjectizer developers.
The project files for CMake are contributed by users. Official building tool is
Mxx_ru. For more info see README (without .md) file.

For more detailed info about cmake build system for SObjectizer see
```
dev/cmake/CmakeQuickHowto.txt
```

When 'make install' finished './installed_files' will contain two subfolders
'./bin' with samples and './lib' with shared libso.5.x.x.so

CMake build system currently supports this options:

  BUILD_ALL      | Enable building examples and tests [default: OFF]
  ---------------|--------------------------------------------------
  BUILD_EXAMPLES | Enable building examples [default: OFF]
  BUILD_TESTS    | Enable building tests    [default: OFF]

To build SObjectizer under Windows by MS Visual Studio 2013 from command line:

```
  mkdir cmake_build
  cd cmake_build
  cmake -DCMAKE_INSTALL_PREFIX=installed_files -G "Visual Studio 12 2013" ../dev
  msbuild /m /t:Build /p:Configuration=Release ALL_BUILD.vcxproj
  msbuild /m /t:Build /p:Configuration=Release INSTALL.vcxproj
```

If it necessary to build examples too, use BUILD_ALL in cmake invocation:

```
  cmake -DCMAKE_INSTALL_PREFIX=installed_files -DBUILD_ALL=ON -G "Visual Studio 12 2013" ../dev
```

License
=======

SObjectizer is distributed under 3-clause BSD license. For license information
please see LICENSE file.
