# - Try to find WCSLIB
# Once done this will define
#
#  WCSLIB_FOUND - system has WCSLIB
#  WCSLIB_INCLUDE_DIR - the WCSLIB include directory
#  WCSLIB_LIBRARIES - Link these to use WCSLIB
#  WCSLIB_VERSION_STRING - Human readable version number
#  WCSLIB_VERSION_MAJOR - major version number
#  WCSLIB_VERSION_MINOR - minor version number

# Copyright (c) 2006, Jasem Mutlaq <mutlaqja@ikarustech.com>
# Based on FindLibfacile by Carsten Niehaus, <cniehaus@gmx.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (WCSLIB_INCLUDE_DIR AND WCSLIB_LIBRARIES)

  # in cache already
  set(WCSLIB_FOUND TRUE)
  message(STATUS "Found WCSLIB: ${WCSLIB_LIBRARIES}, ${WCSLIB_INCLUDE_DIR}")


else (WCSLIB_INCLUDE_DIR AND WCSLIB_LIBRARIES)

  if (NOT WIN32)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(PC_WCSLIB wcslib)
    endif (PKG_CONFIG_FOUND)
  endif (NOT WIN32)

  find_path(WCSLIB_INCLUDE_DIR wcs.h
    PATH_SUFFIXES wcslib
    ${PC_WCSLIB_INCLUDE_DIRS}
    ${_obIncDir}
    ${GNUWIN32_DIR}/include
  )

  find_library(WCSLIB_LIBRARIES NAMES wcs
    PATHS
    ${PC_WCSLIB_LIBRARY_DIRS}
    ${_obLinkDir}
    ${GNUWIN32_DIR}/lib
  )

  if(WCSLIB_INCLUDE_DIR AND WCSLIB_LIBRARIES)
    set(WCSLIB_FOUND TRUE)
  else (WCSLIB_INCLUDE_DIR AND WCSLIB_LIBRARIES)
    set(WCSLIB_FOUND FALSE)
  endif(WCSLIB_INCLUDE_DIR AND WCSLIB_LIBRARIES)


  if (WCSLIB_FOUND)
    # Find version from wcs.h header file
    FILE(READ "${WCSLIB_INCLUDE_DIR}/wcs.h" WCSLIB_H)
    STRING(REGEX REPLACE ".*wcs.h,v[^0-9]*([0-9]+)\\.([0-9]+).*" "\\1.\\2" WCSLIB_VERSION_STRING "${WCSLIB_H}")
    STRING(REGEX REPLACE "^([0-9]+)[.]([0-9]+)" "\\1" WCSLIB_VERSION_MAJOR ${WCSLIB_VERSION_STRING})
    STRING(REGEX REPLACE "^([0-9]+)[.]([0-9]+)" "\\2" WCSLIB_VERSION_MINOR ${WCSLIB_VERSION_STRING})
    message(STATUS "found version string ${WCSLIB_VERSION_STRING}")
    if (NOT WCSLIB_FIND_QUIETLY)
      message(STATUS "Found WCSLIB: ${WCSLIB_LIBRARIES}, ${WCSLIB_INCLUDE_DIR}")
    endif (NOT WCSLIB_FIND_QUIETLY)
  else (WCSLIB_FOUND)
    if (WCSLIB_FIND_REQUIRED)
      message(FATAL_ERROR "WCSLIB not found. Please install wcslib and try again.")
    endif (WCSLIB_FIND_REQUIRED)
  endif (WCSLIB_FOUND)

  mark_as_advanced(WCSLIB_INCLUDE_DIR WCSLIB_LIBRARIES)

endif (WCSLIB_INCLUDE_DIR AND WCSLIB_LIBRARIES)
