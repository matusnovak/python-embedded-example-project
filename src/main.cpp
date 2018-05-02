#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define IS_WINDOWS
#include <Windows.h> // Needed by GetModuleFileNameW
#else
#include <libgen.h> // Needed by readlink
#endif

#include <iostream>
#include <Python.h>
#ifndef IS_WINDOWS
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <pybind11/pybind11.h>
#include <pybind11/eval.h>
#include <pybind11/embed.h>
#ifndef IS_WINDOWS
#pragma GCC diagnostic pop
#endif

namespace py = pybind11;
using namespace py::literals;

///=============================================================================
#ifdef IS_WINDOWS
std::wstring getExecutableDir() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    const auto executableDir = std::wstring(exePath);
    const auto pos = executableDir.find_last_of('\\');
    if (pos != std::string::npos) return executableDir.substr(0, pos);
    return L"\\";
}
#else
std::wstring getExecutableDir() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        const auto path = std::string(dirname(result));
        return std::wstring(path.begin(), path.end());
    }
    return L"/";
}
#endif

///=============================================================================
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Get executable dir and build python PATH variable
    const auto exeDir = getExecutableDir();
#ifdef IS_WINDOWS
    const auto pythonHome = exeDir + L"\\lib";
    const auto pythonPath = exeDir + L"\\lib;" + exeDir + L"\\app;";
#else
    const auto pythonHome = exeDir + L"/lib";
    const auto pythonPath = exeDir + L"/lib:" + exeDir + L"/app";
#endif

    // Initialize python
    Py_OptimizeFlag = 1;
    Py_SetProgramName(L"PythonEmbeddedExample");
    Py_SetPath(pythonPath.c_str());
    Py_SetPythonHome(pythonHome.c_str());

    std::wcout << "Python PATH set to: " << pythonPath << std::endl;

    try {
        py::scoped_interpreter guard{};

        // Disable build of __pycache__ folders
        py::exec(R"(
            import sys
            sys.dont_write_bytecode = True
        )");

        // This imports example.py from app/example.py
        // The app folder is the root folder so you don't need to specify app.example.
        // The app/example script that is being imported is from the actual build folder!
        // Cmake will copy the python scripts after you have compiled the source code.
        std::cout << "Importing module..." << std::endl;
        auto example = py::module::import("example");

        std::cout << "Initializing class..." << std::endl;
        const auto myExampleClass = example.attr("Example");
        auto myExampleInstance = myExampleClass("Hello World"); // Calls the constructor
        // Will print in the terminal window:
        // Example constructor with msg: Hello World

        const auto msg = myExampleInstance.attr("getMsg")(); // Calls the getMsg
        std::cout << "Got msg back on C++ side: " << msg.cast<std::string>() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Something went wrong: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
