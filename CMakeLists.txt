cmake_minimum_required(VERSION 3.1)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
project (PythonEmbeddedExample)

# Check for C++11
set (CMAKE_CXX_STANDARD 11)

# Specify build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CPYTHON_BUILD_TYPE Debug)
else ()
  set(CPYTHON_BUILD_TYPE Release)
endif()

# Add the cpython as an external project that will be included in the build
if(MSVC)
  if(CMAKE_CL_64)
    set(CPYTHON_PLATFORM x64)
    set(CPYTHON_BUILD_DIR amd64)
  else()
    set(CPYTHON_PLATFORM x86)
    set(CPYTHON_BUILD_DIR win32)
  endif()
  ExternalProject_Add(CPYTHON
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/cpython
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cd ${CMAKE_SOURCE_DIR}/libs/cpython && MSBuild.exe /p:Configuration=${CPYTHON_BUILD_TYPE} /property:Platform=${CPYTHON_PLATFORM} "PCBuild/python.vcxproj" /nologo /verbosity:minimal /consoleloggerparameters:summar
    INSTALL_COMMAND ""
    TEST_COMMAND ""
  )
else()
  ExternalProject_Add(CPYTHON
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/cpython
    CONFIGURE_COMMAND cd ${CMAKE_SOURCE_DIR}/libs/cpython && ./configure --disable-static --enable-shared 
    BUILD_COMMAND cd ${CMAKE_SOURCE_DIR}/libs/cpython && make
    INSTALL_COMMAND ""
    TEST_COMMAND ""
  )
endif()

set(CPYTHON_STDLIB_DIR ${CMAKE_SOURCE_DIR}/libs/cpython/Lib)
if(MSVC)
  set(CPYTHON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/cpython/Include ${CMAKE_SOURCE_DIR}/libs/cpython/PC)
  set(CPYTHON_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/libs/cpython/PCBuild/${CPYTHON_BUILD_DIR})
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CPYTHON_BIN ${CMAKE_SOURCE_DIR}/libs/cpython/PCBuild/${CPYTHON_BUILD_DIR}/python38_d.dll)
  else()
    set(CPYTHON_BIN ${CMAKE_SOURCE_DIR}/libs/cpython/PCBuild/${CPYTHON_BUILD_DIR}/python38.dll)
  endif()
else()
  set(CPYTHON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/cpython/Include ${CMAKE_SOURCE_DIR}/libs/cpython)
  set(CPYTHON_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/libs/cpython)
  set(CPYTHON_LIBRARY python3.8m)
  set(CPYTHON_BIN ${CMAKE_SOURCE_DIR}/libs/cpython/libpython3.8m.so)
endif()

# Add the pybind11 library (optional)
ExternalProject_Add(PYBIND
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/pybind11
  CMAKE_ARGS -DPYBIND11_TEST=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DPYTHONLIBS_FOUND=ON -DPYTHON_MODULE_EXTENSION=.lib -DPYTHON_LIBRARY=${CMAKE_SOURCE_DIR}/libs/cpython/PCBuild/win32/python38_d.lib -DPYTHON_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/libs/cpython/PCBuild/win32
  BUILD_COMMAND cmake --build . --config ${CMAKE_BUILD_TYPE}
  INSTALL_COMMAND ""
  TEST_COMMAND ""
)

set(PYBIND_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/pybind11/include)

# Source and header files
FILE(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
FILE(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Linking directories
link_directories(${CPYTHON_LIBRARY_DIR})

# The target executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add macros
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX=1)

# Dependencies
add_dependencies(${PROJECT_NAME} PYBIND)
add_dependencies(${PROJECT_NAME} CPYTHON)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CPYTHON_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${PYBIND_INCLUDE_DIR})

# On MSVC build, the python library is automatically linked (crazy I know)
if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} ${CPYTHON_LIBRARY})
endif()

# Set the executable to console application if MSVC
if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

# Copy Python DLL to the build folder if different
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  -E copy_if_different ${CPYTHON_BIN} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Copy our python sources to the build folder
add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/app $<TARGET_FILE_DIR:${PROJECT_NAME}>/app
)

# Copy the Python stdlib into the build folder (needed by the embedded python)
add_custom_command(
  TARGET ${PROJECT_NAME} 
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} 
  -E copy_directory ${CPYTHON_STDLIB_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/lib
)
