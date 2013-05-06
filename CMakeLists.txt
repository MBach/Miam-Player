cmake_minimum_required(VERSION 2.8.8)

# Name of the project
project(MmeMiamMiamMusicPlayer)

# Find includes in corresponding build directories
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Flags for C++0x / C++11
set(CMAKE_REQUIRED_FLAGS "-std=c++11")

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the libraries
find_package(Qt5Widgets)
find_package(Qt5Multimedia)

# Tell CMake to create the executable
#add_executable(MmeMiamMiamMusicPlayer main.cpp)

# Use the following modules from Qt5: 
#qt5_use_modules(MmeMiamMiamMusicPlayer Widgets Multimedia)

file(
    GLOB_RECURSE
    source_files
    code/src/*
)

MESSAGE("Detecting 3rd party library Taglib")
find_library(taglib TAGLIB_VERSION)
MESSAGE("Detecting 3rd party library Taglib - " ${taglib})
option(USE_BUILTIN_TAGLIB "If the system's version of Taglib is too old, compile the builtin version instead" ON)
if (USE_BUILTIN_TAGLIB AND TAGLIB_VERSION VERSION_LESS 1.8)
	set(TAGLIB_VERSION 1.8)
	set(extProj_PREFIX "${CMAKE_SOURCE_DIR}/code/src/3rdparty/taglib-1.8")
	set(extProj_GIT_REPO_URL git://github.com/taglib/taglib.git)
	set(extProj_INSTALL_DIR "${extProj_PREFIX}/taglib-install")
	set(extProj_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${extProj_INSTALL_DIR}")

	include(ExternalProject)
	ExternalProject_Add("taglib"
	   PREFIX ${extProj_PREFIX}
	   GIT_REPOSITORY ${extProj_GIT_REPO_URL}
	   CMAKE_ARGS ${extProj_CMAKE_ARGS}
	   INSTALL_DIR ${extProj_INSTALL_DIR}
	   GIT_TAG "v1.8"
	)
endif()


#set(CMAKE_REQUIRED_INCLUDES "${TAGLIB_INCLUDE_DIRS}")
#set(CMAKE_REQUIRED_LIBRARIES "${TAGLIB_LIBRARIES}")
set(CMAKE_REQUIRED_INCLUDES)
set(CMAKE_REQUIRED_LIBRARIES)

# Target


