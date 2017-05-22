Travis CI [![Build Status](https://travis-ci.org/eao197/so-5-5.svg?branch=master)](https://travis-ci.org/eao197/so-5-5)

Coverity [![Coverity Scan Build Status](https://scan.coverity.com/projects/4786/badge.svg)](https://scan.coverity.com/projects/4786)

This is an experimental SObjectizer https://sourceforge.net/p/sobjectizer/ repository mirror.
Project documentation in wiki http://sourceforge.net/p/sobjectizer/wiki/Home/.

SObjectizer is a framework for agent-oriented programming in C++ (like Theron 
or C++ Actor Framework (CAF) for C++ and Akka for Java). From one side, 
SObjectizer is a set of rules and principles for designing programms in
agent-oriented style. On other side SObjectizer provides set of C++ classes 
for agents implementation.

Supported platforms: Linux, Windows, FreeBSD, MacOS X, Android

Building
========

[![Join the chat at https://gitter.im/masterspline/SObjectizer](https://badges.gitter.im/masterspline/SObjectizer.svg)](https://gitter.im/masterspline/SObjectizer?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

There are two ways for building SObjectizer. The first one by using [Mxx_ru](https://sourceforge.net/projects/mxxru/)
tool. The second one by using [CMake](http://www.cmake.org/).

NOTE. Since v.5.5.15.2 there is a support of Android platform.  Building for
Android is possible by CMake only. See the corresponding section below.

Building via Mxx_ru
-------------------

NOTE. This is a standard way for building SObjectizer. This way is used in
SObjectizer development process.

To build SObjectizer it is necessary to use Ruby language and Mxx_ru tool.
Install Ruby and then install Mxx_ru via RubyGems command:
```
  gem install Mxx_ru
```
If you already have Mxx_ru installed please update to at least version 1.6.11:
```
  gem update Mxx_ru
```
To build SObjectizer:
```
  git clone https://github.com/eao197/so-5-5 so-5-5
  cd so-5-5/dev
  ruby build.rb
```
Or to build SObjectizer with all tests and samples:
```
  git clone https://github.com/eao197/so-5-5 so-5-5
  cd so-5-5/dev
  ruby build_all.rb
```
Please note that under Linux and FreeBSD it could be necessary to define `LD_LIBRARY_PATH`
environment variable. And the actual build command sequence under Linux could
be as follows:
```
  cd so-5-5/dev
  export LD_LIBRARY_PATH=target/release
  ruby build_all.rb
```
To build html-format documentation for SObjectizer the Doxygen tool is
necessary. If it is installed then:
```
  cd so-5-5/doxygen
  doxygen
```
Generated html-files will be located in so-5-5/dev/doc/html.

NOTE. If you do not specify `MXX_RU_CPP_TOOLSET` by youself then Mxx_ru will
try to detect your C++ toolset automatically. If you want to use C++ compiler
which is not default in your system please define `MXX_RU_CPP_TOOLSET`
environment variable manually. It could look like:
```
  export MXX_RU_CPP_TOOLSET="clang_linux compiler_name=clang++-3.5 linker_name=clang++-3.5"
```
More information about tuning Mxx_ru for your needs you can find in the
corresponding [documentation](https://sourceforge.net/projects/mxxru/files/Mxx_ru%201.6/mxx_ru-1.6.6.pdf).

Building via CMake
------------------

NOTE. This way of building is not used by SObjectizer developers. But
CMake-related files are in actual state, they maintained by SObjectizer Team
and can be used for building SObjectizer, its samples and tests.

To build SObjectizer via CMake it is necessary to have CMake and some knowledge
of how to use it. The following action is just a demonstration. For more 
detailed info about cmake build system for SObjectizer see
`dev/cmake/CmakeQuickHowto.txt`

To get and build SObjectizer under Linux/FreeBSD in command line run:
```
  git clone https://github.com/eao197/so-5-5 so-5-5
  cd so-5-5
  mkdir cmake_build
  cd cmake_build
  cmake -DCMAKE_INSTALL_PREFIX=installed_files ../dev
  make
  make install
```
Those commands will create all necessary Makefile, then build SObjectizer. If
it necessary to build examples and tests too, use
```
  cmake -DBUILD_ALL=ON -DCMAKE_INSTALL_PREFIX=installed_files ../dev
```
When `make install` finished `./installed_files` will contain two subfolders
`./bin` with samples and `./lib` with shared libso.5.x.x.so

CMake build system currently supports this options:

| Option         | Description
  ---------------|--------------------------------------------------
  BUILD_ALL      | Enable building examples and tests [default: OFF]
  BUILD_EXAMPLES | Enable building examples [default: OFF]
  BUILD_TESTS    | Enable building tests    [default: OFF]

Also You can run cmake GUI instead command line cmake to configure build.

To build SObjectizer under Windows by MS Visual Studio 2013 from command line:
```
  mkdir cmake_build
  cd cmake_build
  cmake -DCMAKE_INSTALL_PREFIX=installed_files -G "Visual Studio 12 2013" ../dev
  msbuild /m /t:Build /p:Configuration=Release ALL_BUILD.vcxproj
  msbuild /m /t:Build /p:Configuration=Release INSTALL.vcxproj
```
MinGW can also be used under Windows. All necessary makefiles for MinGW can be
created from command line:
```
  mkdir cmake_build
  cd cmake_build
  cmake -DCMAKE_INSTALL_PREFIX=installed_files -G "MinGW Makefiles" ../dev
  mingw32-make
  mingw32-make install
```

Building for Android
--------------------

Building for Android is possible via [CrystaX NDK](https://www.crystax.net/android/ndk).
You need CrystaX NDK v.10.4.0 or higher already installed in your system.
CMake is used for building SObjectizer:
```
  git clone http://github.com/eao197/so-5-5 so-5-5
  cd so-5-5
  mkdir cmake_build
  cd cmake_build
  export NDK=/path/to/the/crystax-ndk
  cmake -DBUILD_ALL -DCMAKE_INSTALL_PREFIX=result -DCMAKE_TOOLCHAIN_FILE=$NDK/cmake/toolchain.cmake -DANDROID_ABI=arm64-v8a ../dev
  make
  make test
  make install
```

License
=======
SObjectizer distributed under 3-clause BSD license. For license information
please see LICENSE file.
