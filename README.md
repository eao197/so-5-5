Travis CI [![Build Status](https://travis-ci.org/masterspline/SObjectizer.svg?branch=master)](https://travis-ci.org/masterspline/SObjectizer)

Coverity [![Coverity Scan Build Status](https://scan.coverity.com/projects/4786/badge.svg)](https://scan.coverity.com/projects/4786)

This is unofficial SObjectizer https://sourceforge.net/p/sobjectizer/ repository mirror.
Project documentation in wiki https://github.com/masterspline/SObjectizer/wiki

SObjectizer is a framework for agent-oriented (a.k.a. actor model) programming
 in C++ (like Theron or C++ Actor Framework (CAF) for C++ and Akka for Java).
SObjectizer is a set of rules and principles for designing programms in
agent-oriented style. Also it provides set of C++ classes for implementing
agents, mailboxes for message exchange, different dispatcher types for running
agents and working environment. 

Supported platforms: Linux, Windows, FreeBSD, MacOs.

Building
========

on Linux:
```
  git clone --recursive https://github.com/masterspline/SObjectizer.git so
  mkdir build
  cd build
  cmake -DBUILD_ALL=ON -DCMAKE_INSTALL_PREFIX=installed_files ../so/dev
  make
  make test
  make install
```
on Windows:

First install "git for Windows" from https://git-for-windows.github.io/ or https://git-scm.com/download/win and cmake from https://cmake.org/download/ and make it avalable in PATH. Next from developer console (where msbuild and compiler executable is avalable and other environment variables is set)
```
  git clone --recursive https://github.com/masterspline/SObjectizer.git so
  mkdir build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=installed_files -DBUILD_ALL=ON -G "Visual Studio 12 2013" ../so/dev
  msbuild /m /t:Build /p:Configuration=Release ALL_BUILD.vcxproj
  msbuild /m /t:Build /p:Configuration=Release INSTALL.vcxproj
```
Also You can run cmake GUI instead command line cmake to configure build.

For more detailed info about cmake build system for SObjectizer and avalable params
see dev/cmake/CmakeQuickHowto.txt

NOTE. cmake is experimental building solution for SObjectizer, partly supported 
by SObjectizer developers.
The project files for CMake are contributed by users. Official building tool is
Mxx_ru. For more info see README (without .md) file.

For more detailed info about cmake build system for SObjectizer see
```
dev/cmake/CmakeQuickHowto.txt
```

When 'make install' finishes './installed_files' will contain two subfolders
'./bin' with samples and './lib' with shared libso.5.x.x.so

CMake build system currently supports this options:

| Option         | Description
  ---------------|--------------------------------------------------
  BUILD_ALL      | Enable building examples and tests [default: OFF]
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
SObjectizer distributed under 3-clause BSD license. For license information
please see LICENSE file.
