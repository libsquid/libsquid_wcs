//
// Library for performing wcs x,y to r,d conversions using wcslib
//
// -------------------------- LICENSE -----------------------------------
//
// This file is part of the LibSQUID software libraray.
//
// LibSQUID is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LibSQUID is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with LibSQUID.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2014 James Wren and Los Alamos National Laboratory
//

#ifndef LIBSQUIDWCS_H
#define LIBSQUIDWCS_H

#define LIBSQUIDWCS_VERSION "0.6.1"
#define LIBSQUIDWCS_MAJOR 0
#define LIBSQUIDWCS_MINOR 6
#define LIBSQUIDWCS_AGE 1
#define LIBSQUIDWCS_RELEASE ""

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>

#include <libsquid.h>

#include <fitsio.h>
#include <wcs.h>
#include <wcshdr.h>
#include <wcsfix.h>

#define CARD_EXCLUDE {\
    "SIMPLE", "BITPIX", "NAXIS", "NAXIS1", "NAXIS2", NULL}

// max length of fits header
#define HDR_MAXLEN 5000

// Max array size (order) for SIP matrices.
// In reality these numbers are typically small (2 or 3, maybe 4)
// and would almost never exceed 10, you would be over-fitting
// at that point.
#define SIP_ARRAY_MAX 10

// Parameter struct for handling SIP distortions in FITS WCS headers.
struct sip_param {
   int have_sip; // 1 if sip in header, 0 if not
   int a_order; // for forward transform (from sky to img)
   int b_order; // for forward transform (from sky to img)
   int ap_order; // for reverse transform (from img to sky)
   int bp_order; // for reverse transform (from img to sky)
   double a[SIP_ARRAY_MAX][SIP_ARRAY_MAX]; // forward x coefficients
   double b[SIP_ARRAY_MAX][SIP_ARRAY_MAX]; // forward y coefficients
   double ap[SIP_ARRAY_MAX][SIP_ARRAY_MAX]; // reverse x coefficients
   double bp[SIP_ARRAY_MAX][SIP_ARRAY_MAX]; // reverse y coefficients
   double crval1; // lon reference point
   double crval2; // lat reference point
   double crpix1; // img x reference point
   double crpix2; // img y reference point
};

int wcs_pix2rd(struct wcsprm *wcs, double x, double y, double *ra, double *dec);
int wcs_rd2pix(struct wcsprm *wcs, double ra, double dec, double *x, double *y);
int wcs_addquid(int proj, struct wcsprm *wcs, int k, double x, double y, squid_type squidarr[], long *nidx);
int wcs_getsquids(int proj, struct wcsprm *wcs, double cdelt,long naxes[], int k, squid_type squidarr[], long *nidx);
int tile_getwcs(int proj, squid_type squid, squid_type tside, struct wcsprm **wcs);
int quadcube_getwcs(int proj, squid_type squid, squid_type tside, struct wcsprm **wcs);
int hsc_getwcs_pole(squid_type squid, squid_type tside, struct wcsprm **wcs);
int hsc_getwcs_equator(squid_type squid, squid_type tside, struct wcsprm **wcs);
int tile_addwcs(int proj, squid_type squid, struct wcsprm *wcs, char *ihdr, fitsfile *ofptr);
int sip_read(fitsfile *fptr, struct sip_param *sparam);
int sip_forward(struct sip_param *sparam, double x, double y, double *xout, double *yout);
int sip_reverse(struct sip_param *sparam, double x, double y, double *xout, double *yout);


#ifdef __cplusplus
}
#endif

#endif //LIBSQUIDWCS_H

