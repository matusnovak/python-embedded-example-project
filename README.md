# Python Embedded Example Project

This is an example project using mebedded python 3.8 (cpython) in C++ console application using CMake. This example project also contains pybind11 library for easy binding between C++ and python. 

Tested on Windows 10 with Visual Studio 2013, 2015, and 2017 (both x86 and x64). Also tested on Ubuntu with GCC (x64).

**Note this example has nothing to do with embedding C++ in python! This is the other way around! Embeding python in C++.**

## About

Normally, you have a python installed in your system and then you can launch python scripts from command line. What if you want to create C++ application, but want to use python scripts (for example as modding for games)? One option is to let the user install python on their system and then install your app. No problem there, except, you are forcing the user to modify their system. Additonally, you can never guarantee that the user will install the specific python version (2.7 or 3.8) you require. Even if all of that is sorted out, the python that will get run will probably need third party packages (or you need to explicitly disable some packages), this can't be done easily.

What if we can embed entire python in C++ executable? This way, the user won't have to install python on their system nor additional dependencies. Everything will be bundled up in the executable. 

This project comes with cpython 3.8 (as a git submodule) and pybind11. Once you build the project, your build folder will look like this:

```
python-embedded-example-project/
  build/
    Release/
      PythonEmbeddedExample.exe
      app/
        example.py
      lib/
        a lot of Python standard library files
```

The script files (such as example.py) are copied to the build directory from `src/app` and the python standard libraries are copied over from git submodule located in `python-embedded-example-project/libs/cpython/Lib`. You can't get rid of those standard libraries! Python needs them to initialise. However, you could limit the standard libraries by only selectively copying over the ones you need.

When the executable runs, it will load an `example` module and creates an instance of `Example` class from `example.py` file. That's all it does.

**The python PATH is limited only to the app and lib folder located next to the executable.** The embedded python won't have any access to installed packages in your operating system. You will have to include them in any of those two folders.

## Requirements

Visual Studio 2013 (or newer) or Linux with GCC. MinGW is sadly not supported, and clang has not been tested.

## Download

Don't forget to initialise and update the submodules!

```
git clone https://github.com/matusnovak/python-embedded-example-project 
cd python-embedded-example-project 
git submodile init
git submodule update
```

## Build using Visual Studio on Windows

**Please note:** that you have to explicitly specify `CMAKE_BUILD_TYPE`. If you specify Debug, then you must use Debug in your Visual Studio. You won't be able to change to Release using the dropdown list in the main menu. Python will be build using the `CMAKE_BUILD_TYPE` flag regarding the chosen configuration in Visual Studio. To change from Debug to Release, re-run the cmake and set the `CMAKE_BUILD_TYPE` to Release.

```
cd python-embedded-example-project 
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=Debug
```

Then open the generated solution file "PythonEmbeddedExample.sln" in the build folder. You should see the following projects:

* **ALL_BUILD** - Building this will build all projects
* **CPYTHON** - The embedded python project
* **PYBIND** - The pybind11 library for binding
* **PythonEmbeddedExample** - This is the example project

Build the PythonEmbeddedExample and then run the `PythonEmbeddedExample.exe` from command line. That's all.

## Build using GCC on Linux

```
cd python-embedded-example-project 
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

Then build the example by running:

```
make
```

The `PythonEmbeddedExample` executable will be generated.

## Example output

Example output of the `PythonEmbeddedExample` executable.

```
Python PATH set to: C:\Users\matus\Documents\cpp\python-embedded-example-project\build\Debug\lib;C:\Users\matus\Documents\cpp\python-embedded-example-project\build\Debug\app;
Importing module...Initializing class...
Example constructor with msg: Hello World
Got msg back on C++ side: Hello World
```