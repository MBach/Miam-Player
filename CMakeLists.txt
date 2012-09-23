cmake_minimum_required(VERSION 2.8)

# Name of the project
project(Madame-Miam-Miam-Music-Player)

# Required packages to build
find_package(Qt4 REQUIRED QtCore QtGui)
set(QT_USE_PHONON TRUE)

find_package(TAGLIB REQUIRED)

file(
    GLOB_RECURSE
    source_files
    src/*
)

INCLUDE(taglib)
IF(NOT TAGLIB_FOUND)
  FIND_LIBRARY(TAGLIB_LIBRARIES NAMES libtag libtag.dll PATHS "./libs/MacOS" "./libs/Windows" DOC "Freetype library")
  FIND_PATH(TAGLIB_INCLUDE_DIRS ftbuild.h "./includes" DOC "Freetype includes")
ENDIF(NOT TAGLIB_FOUND)
INCLUDE_DIRECTORIES(${TAGLIB_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(Madame-Miam-Miam-Music-Player ${TAGLIB_LIBRARIES})


# Target


