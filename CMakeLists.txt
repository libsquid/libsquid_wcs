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

project(LIBSQUIDWCS)
cmake_minimum_required(VERSION 2.8)

# Read libsquid_wcs.h and get version and release string
file(READ "${LIBSQUIDWCS_SOURCE_DIR}/libsquid_wcs.h" LIBSQUIDWCS_H)
string(FIND "${LIBSQUIDWCS_H}" "LIBSQUIDWCS_VERSION" LIBSQUIDWCS_VERSION_CHECK)
if (LIBSQUIDWCS_VERSION_CHECK EQUAL -1)
   message(FATAL_ERROR "version number not in libsquid_wcs.h")
endif()
string(REGEX REPLACE ".*#define LIBSQUIDWCS_VERSION[^0-9]*([0-9]+)\\.([0-9]+)\\.([0-9]+).*" 
	     "\\1.\\2.\\3" LIBSQUIDWCS_VERSION_STRING "${LIBSQUIDWCS_H}")
if (LIBSQUIDWCS_VERSION_STRING STREQUAL ifdef)
   message(FATAL_ERROR "version string not set in libsquid_wcs.h")
endif()
message("LIBSQUIDWCS_VERSION_STRING ${LIBSQUIDWCS_VERSION_STRING}")
string(FIND "${LIBSQUIDWCS_H}" "LIBSQUIDWCS_RELEASE" LIBSQUIDWCS_RELEASE_CHECK)
if (LIBSQUIDWCS_RELEASE_CHECK EQUAL -1)
   set(LIBSQUIDWCS_RELEASE_STRING "")
else()
   string(REGEX REPLACE ".*#define LIBSQUIDWCS_RELEASE[^0-9a-zA-Z\\.]*([0-9a-zA-Z\\.\\-]+).*" 
   	     "\\1" LIBSQUIDWCS_RELEASE_STRING "${LIBSQUIDWCS_H}")
   if (LIBSQUIDWCS_RELEASE_STRING STREQUAL ifdef)
      set(LIBSQUIDWCS_RELEASE_STRING "")
   endif()
   message("LIBSQUIDWCS_RELEASE_STRING ${LIBSQUIDWCS_RELEASE_STRING}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
		      "${CMAKE_SOURCE_DIR}/cmake_modules"
		      "${CMAKE_SOURCE_DIR}/../cmake_modules")

# Get list of source and header files
set(CMAKE_INCLUDE_CURRENT_DIR ON)
file(GLOB LIBSQUIDWCS_SOURCES *.c)
file(GLOB LIBSQUIDWCS_HEADERS *.h)

# Find necessary libraries
if (NOT CMAKE_PROJECT_NAME STREQUAL SQUIDALL)
   find_package(LIBSQUID REQUIRED)
   message("LIBSQUID_FOUND = ${LIBSQUID_FOUND}")
   message("LIBSQUID_INCLUDE_DIR = ${LIBSQUID_INCLUDE_DIR}")
   message("LIBSQUID_LIBRARIES = ${LIBSQUID_LIBRARIES}")
   if (NOT LIBSQUID_FOUND)
      message(FATAL_ERROR "LIBSQUID library not found")
   endif() 
endif()
find_package(CFITSIO REQUIRED)
find_package(WCSLIB REQUIRED)
include_directories(${LIBSQUID_SOURCE_DIR}
	${LIBSQUID_INCLUDE}
	${CFITSIO_INCLUDE_DIR}
	${WCSLIB_INCLUDE_DIR})
set(LIBS ${LIBS}
	 ${CFITSIO_LIBRARIES}
	 ${WCSLIB_LIBRARIES})
link_libraries(${LIBS})

add_library(libsquidwcs_static STATIC ${LIBSQUIDWCS_SOURCES})
set_target_properties(libsquidwcs_static PROPERTIES OUTPUT_NAME squid_wcs)
add_library(libsquidwcs_shared SHARED ${LIBSQUIDWCS_SOURCES})
set_target_properties(libsquidwcs_shared PROPERTIES 
			     OUTPUT_NAME squid_wcs
			     SOVERSION ${LIBSQUIDWCS_VERSION_STRING})
add_subdirectory(bin)

add_custom_target(libsquidwcs_dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

#install(TARGETS libsquidwcs_static libsquidwcs_shared
#	RUNTIME DESTINATION bin COMPONENT Runtime
#	LIBRARY DESTINATION lib COMPONENT Runtime
#	ARCHIVE DESTINATION lib COMPONENT Develop
#)
install(TARGETS libsquidwcs_static
	LIBRARY	DESTINATION lib
	ARCHIVE	DESTINATION lib
	COMPONENT libsquidwcs
)
install(TARGETS libsquidwcs_shared
	LIBRARY	DESTINATION lib
	ARCHIVE	DESTINATION lib
	COMPONENT libsquidwcs
)
install(FILES ${LIBSQUIDWCS_HEADERS} 
	DESTINATION include
	COMPONENT libsquidwcs
)

set(CPACK_PACKAGE_NAME "libsquid_wcs")
set(CPACK_PACKAGE_VERSION ${LIBSQUIDWCS_VERSION_STRING})
if (LIBSQUIDWCS_RELEASE_STRING STREQUAL "")
   set(CPACK_PACKAGE_RELEASE "")
else()
   set(CPACK_PACKAGE_RELEASE "~${LIBSQUIDWCS_RELEASE_STRING}")
endif()
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
    "The LibSQUID WCS Interface Library")
set(CPACK_PACKAGE_CONTACT "Jim Wren; jwren@lanl.gov")
set(CPACK_PACKAGE_VENDOR ${CPACK_PACKAGE_CONTACT})
if (NOT LIBSQUIDWCS_RELEASE_STRING STREQUAL "")
   set(CPACK_RPM_PACKAGE_RELEASE "${LIBSQUIDWCS_RELEASE_STRING}")
endif()
set(CPACK_RPM_PACKAGE_REQUIRES "libsquid > 0.1, cfitsio >= 3.0, wcslib >= 4.19")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsquid (> 0.1), cfitsio (>= 3.0), wcslib (>= 4.19)")
set(CPACK_GENERATOR "RPM" "DEB")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "~$;[.]bak$/;[.]swp$;[.]svn$")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}${CPACK_PACKAGE_RELEASE}")
include(CPack)
