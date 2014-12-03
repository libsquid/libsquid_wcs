libsquid_wcs
============

LibSQUID library for interfacing to Mark Calabretta's WCSLIB library.

============================================================================

This is a seperate library which provides functions for interfacing Mark
Calabretta's WCSLIB library at http://www.atnf.csiro.au/people/mcalabre/WCS to
the LibSQUID system.  The libsquid_wcs library is primarily used by the "squidfits"
utility to manipulate FITS images with WCS coordinate information.

The cmake system is used to build the library.  See the INSTALL file for more
information.  This library requires the libsquid, cfitsio, and wcslib
libraries to have been previously installed.

For more information see http://libsquid.github.io, the wiki, and the
pdf documentation in the libsquid_doc repository.

This library is released under the LGPL license.  If you use it, please let
me know via email to jwren@lanl.gov.  Enjoy!
