//
// Test the libsquid_wcs library to see if XPH projection works.
// If not, then the version of wcslib is too old to support Healpix.
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
  int i;
  squid_type tside;
  squid_type squid;
  struct wcsprm *wcs;
  double ra,dec;
  double rarr[4],darr[4];
  double xarr[5],yarr[5];
  
  tside=450;
  //squid=209;
  //squid=210;
  squid=512;
  //squid=160;
  printf("squid=%ld tside=%ld\n",(long)squid,(long)tside);
  if (squid_corners(HSC, squid, rarr, darr) == -1) {
    fprintf(stderr,"squid_corners failed in %s\n",argv[0]);
    exit(-1);
  }
  for (i=0; i<4; i++) {
    printf("corner %d is %.5f,%.5f\n",i,rarr[i]/DD2R,darr[i]/DD2R);
  }
  if (squid2sph(HSC, squid, &ra, &dec) == -1) {
    fprintf(stderr,"squid2rd failed in %s\n",argv[0]);
    exit(-1);
  }
  printf("center is %.5f,%.5f\n",ra/DD2R,dec/DD2R);

  if (tile_getwcs(HSC, squid, tside, &wcs) == -1) {
    fprintf(stderr,"tile_getwcs failed in %s\n",argv[0]);
    exit(-1);
  }
  
  xarr[0]=0.0; xarr[1]=tside; xarr[2]=0.0; xarr[3]=tside; xarr[4]=tside/2.0;
  yarr[0]=0.0; yarr[1]=0.0; yarr[2]=tside; yarr[3]=tside; yarr[4]=tside/2.0;
  for (i=0; i<5; i++) {
    if (wcs_pix2rd(wcs, xarr[i], yarr[i], &ra, &dec) == -1) {
      fprintf(stderr,"wcs_rd2pix failed in %s\n",argv[0]);
      exit(-1);
    }
    printf("x,y=%.1f,%.1f --> ra,dec=%.5f,%.5f\n",xarr[i],yarr[i],ra,dec);
  }

  exit(1);
}
