# -------------------------- LICENSE -----------------------------------
#
# This file is part of the LibSQUID software library.
#
# LibSQUID is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# LibSQUID is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with LibSQUID.  If not, see <http://www.gnu.org/licenses/>.
#
# Copyright 2014 James Wren and Los Alamos National Laboratory
#

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Use sensible default for undefined LIB_SUFFIX
if (NOT DEFINED LIB_SUFFIX)
  if(NOT (CMAKE_SIZEOF_VOID_P EQUAL 4))
    set(LIB_SUFFIX 64)
  endif()
endif()
# Force LIB_SUFFIX availability in ccmake
set(LIB_SUFFIX ${LIB_SUFFIX} CACHE STRING "Library suffix, appended to library path.")
mark_as_advanced(LIB_SUFFIX)

# Parse libsquid_wcs.h LIBSQUIDWCS_VERSION
file(STRINGS "${CMAKE_SOURCE_DIR}/libsquid_wcs.h" LIBSQUIDWCS_VERSION_STRING REGEX
     "^[ \t]*#define[ \t]+LIBSQUIDWCS_VERSION[ \t]+\"[0-9]+\\.[0-9]+\\.[0-9]+\"[ \t]*$")
if (LIBSQUIDWCS_VERSION_STRING STREQUAL "")
  message(FATAL_ERROR "LIBSQUIDWCS_VERSION not found in libsquid_wcs.h")
endif()
string(REGEX REPLACE ".*\"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*" "\\1.\\2.\\3"
       LIBSQUIDWCS_VERSION_STRING "${LIBSQUIDWCS_VERSION_STRING}")
# Parse libsquid_wcs.h LIBSQUIDWCS_RELEASE
file(STRINGS "${CMAKE_SOURCE_DIR}/libsquid_wcs.h" LIBSQUIDWCS_RELEASE_STRING REGEX
     "^[ \t]*#define[ \t]+LIBSQUIDWCS_RELEASE[ \t]+\"[0-9]+\"[ \t]*$")
if (LIBSQUIDWCS_RELEASE_STRING STREQUAL "")
  message(FATAL_ERROR "LIBSQUIDWCS_RELEASE not found in libsquid_wcs.h")
endif()
string(REGEX REPLACE ".*\"([0-9]+)\".*" "\\1"
       LIBSQUIDWCS_RELEASE_STRING "${LIBSQUIDWCS_RELEASE_STRING}")

# Overall project options
project(LIBSQUIDWCS)
set(PACKAGE_VERSION "${LIBSQUIDWCS_VERSION_STRING}")
set(LIBSQUIDWCS_VERSION "${LIBSQUIDWCS_VERSION_STRING}")
set(LIBSQUIDWCS_RELEASE "${LIBSQUIDWCS_RELEASE_STRING}")

option(BUILD_SHARED_LIBS "Build shared libraries." ON)
option(BUILD_STATIC_LIBS "Build static libraries." OFF)

# Find necessary libraries
#libsquid
find_package(LIBSQUID REQUIRED)
if (${LIBSQUID_VERSION_STRING} VERSION_LESS "0.5.3")
  message(FATAL_ERROR "LIBSQUID version ${LIBSQUID_VERSION_STRING} is too old")
endif()
#cfitsio
find_package(CFITSIO REQUIRED)
if (${CFITSIO_VERSION_STRING} VERSION_LESS "3.0")
  message(FATAL_ERROR "CFITSIO version ${CFITSIO_VERSION_STRING} is too old")
endif()
#wcslib
find_package(WCSLIB REQUIRED)
if (${WCSLIB_VERSION_STRING} VERSION_LESS "4.19")
  message(FATAL_ERROR "WCSLIB version ${WCSLIB_VERSION_STRING} is too old")
endif()

# Build options
set(LIBS squid_wcs)
set(LIBS_PRIVATE
  ${LIBSQUID_LIBRARIES}
  ${CFITSIO_LIBRARIES}
  ${WCSLIB_LIBRARIES}
  m)
include_directories(
  ${LIBSQUIDWCS_SOURCE_DIR}
  ${LIBSQUID_INCLUDEDIR}
  ${CFITSIO_INCLUDEDIR}
  ${WCSLIB_INCLUDEDIR})
link_directories(${LIBSQUIDWCS_BINARY_DIR})
link_libraries(${LIBS_PRIVATE})

# Additional builds
add_subdirectory(bin)

# Generate list of source and header files
file(GLOB LIBSQUIDWCS_HEADERS *.h)
file(GLOB LIBSQUIDWCS_SOURCES *.c)

# Define targets and installs
if (BUILD_SHARED_LIBS)
  add_library(libsquidwcs_shared SHARED ${LIBSQUIDWCS_SOURCES})
  set_target_properties(libsquidwcs_shared PROPERTIES
                        OUTPUT_NAME squid_wcs
                        SOVERSION ${LIBSQUIDWCS_VERSION_STRING})
  install(TARGETS libsquidwcs_shared
          LIBRARY DESTINATION lib${LIB_SUFFIX}
          RUNTIME DESTINATION lib${LIB_SUFFIX}
          COMPONENT libsquidwcs)
endif()
if (BUILD_STATIC_LIBS)
  add_library(libsquidwcs_static STATIC ${LIBSQUIDWCS_SOURCES})
  set_target_properties(libsquidwcs_static PROPERTIES
                        OUTPUT_NAME squid_wcs)
  install(TARGETS libsquidwcs_static
          ARCHIVE DESTINATION lib${LIB_SUFFIX}
          COMPONENT libsquidwcs)
endif()
install(FILES ${LIBSQUIDWCS_HEADERS} 
        DESTINATION include
        COMPONENT libsquidwcs-devel)

# Define packages
set(CPACK_PACKAGE_NAME "libsquid_wcs")
set(CPACK_PACKAGE_VERSION ${LIBSQUIDWCS_VERSION})
set(CPACK_PACKAGE_RELEASE ${LIBSQUIDWCS_RELEASE})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The LibSQUID WCS Interface Library")
set(CPACK_PACKAGE_CONTACT "Jim Wren, jwren@lanl.gov, Los Alamos National Laboratory")
set(CPACK_PACKAGE_VENDOR ${CPACK_PACKAGE_CONTACT})
set(CPACK_RPM_PACKAGE_REQUIRES "libsquid >= 0.5.3, cfitsio >= 3.0, wcslib >= 4.19")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsquid (>= 0.5.3), cfitsio (>= 3.0), wcslib (>= 4.19)")
set(CPACK_GENERATOR "RPM" "DEB")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "~$" "[.]bak$" "[.]swp$" "[.]svn$")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST
    "${CMAKE_INSTALL_PREFIX}"
    "${CMAKE_INSTALL_PREFIX}/include"
    "${CMAKE_INSTALL_PREFIX}/bin"
    "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}"
    "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig")
set(CPACK_RPM_PACKAGE_RELEASE ${LIBSQUIDWCS_RELEASE})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}")
include(CPack)

# Define package_source
add_custom_target(libsquidwcs_dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# Prepare pkg-config
if (UNIX)
  # Vars
  SET(PC_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
  SET(PC_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
  # Libs
  foreach(libname ${LIBS})
    set (PC_LIBS_STR ${PC_LIBS_STR} "-l${libname}")
  endforeach()
  string(REPLACE ";" " " PC_LIBS_STR "${PC_LIBS_STR}")
  # Libs.private
  foreach(libname ${LIBS_PRIVATE})
    set (PC_LIBS_PRIVATE_STR ${PC_LIBS_PRIVATE_STR} "-l${libname}")
  endforeach()
  string(REPLACE ";" " " PC_LIBS_PRIVATE_STR "${PC_LIBS_PRIVATE_STR}")
  # Replace
  configure_file(${LIBSQUIDWCS_SOURCE_DIR}/libsquid_wcs.pc.in
                 ${LIBSQUIDWCS_BINARY_DIR}/libsquid_wcs.pc @ONLY)
  # Install
  INSTALL(FILES ${LIBSQUIDWCS_BINARY_DIR}/libsquid_wcs.pc
          DESTINATION ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig
          COMPONENT libsquid_wcs-utilities)
endif()
