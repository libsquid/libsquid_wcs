//
// Given a fits file with valid wcs, convert x,y to ra,dec
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
//

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

#include <libsquid_wcs.h>

int main(int argc, char *argv[]) {
  struct wcsprm *wcs; // wcs coordinate struct
  char *header;
  fitsfile *fptr;
  int status=0, bitpix, naxis,i;
  long naxes[2];
  int nkeyrec, nreject, nwcs;
  double ra,dec,x,y;

  if (argc != 4) {
    printf("Example usage...\n");
    printf("%s infile x y\n",argv[0]);
    printf("infile is fits file with valid wcs header\n");
    exit(-1);
  }
  x=atof(argv[2]);
  y=atof(argv[3]);

  // Load original image and header
  if (fits_open_file(&fptr, argv[1], READONLY, &status)) {
    fits_report_error(stderr, status);
    exit(-1);
  }
  if (fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status)) {
    fits_report_error(stderr, status);
    exit(-1);
  }
  // Remove SCAMP PV* headers because they screw up wcslib
  for (i=0; i<1000; i++) {
    if (fits_delete_key(fptr,"PV?_*", &status)) {
      fits_clear_errmsg();
      status=0;
      break;
    }
  }
  // Now convert header into string for wcs parsing  
  if (fits_hdr2str(fptr, 1, NULL, 0, &header, &nkeyrec, &status)) {
    fits_report_error(stderr, status);
    exit(-1);
  }
  // Interpret the WCS keywords, deleting them afterwards
  if ((status = wcspih(header, nkeyrec, WCSHDR_all, -3, &nreject, &nwcs, &wcs))) {
    fprintf(stderr, "wcspih ERROR %d: %s.\n", status, wcshdr_errmsg[status]);
    fprintf(stderr,"%d rejected keywords\n",nreject);
    exit(-1);
  }
  // Now covert ra,dec to x,y
  if (wcs_pix2rd(wcs,x,y,&ra,&dec) < 0) {
    fprintf(stderr,"wcs_rd2pix failed in %s\n",argv[0]);
    exit(-1);
  }
  printf("%.3f %.3f\n",ra,dec);

  free(header);
  free(wcs);
  fits_close_file(fptr,&status);

  return(0);
}
