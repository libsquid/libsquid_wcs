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

#include <libsquid_wcs.h>

// Given a wcs struct, convert image x,y to sky ra,dec (in deg)
int wcs_pix2rd(struct wcsprm *wcs, double x, double y, double *ra, double *dec) {
  double pixcrd[2],imgcrd[2],phi[1],theta[1],wcor[2];
  int status, wstat[1];

  pixcrd[0]=x;
  pixcrd[1]=y;
  if ((status=wcsp2s(wcs,1,2,pixcrd,imgcrd,phi,theta,wcor,wstat)) > 0) {
    fprintf(stderr, "wcsp2s returned status=%d wstat=%d in hpxwcs_pix2rd()\n", status, wstat[0]);
    return(-1);
  }
  *ra = wcor[0];
  *dec = wcor[1];

  return(0);
}

// Given a wcs struct, convert sky ra,dec (in deg) to image x,y
int wcs_rd2pix(struct wcsprm *wcs, double ra, double dec, double *x, double *y) {
  double pixcrd[2],imgcrd[2],phi[1],theta[1],wcor[2];
  int status, wstat[1];

  wcor[0]=ra;
  wcor[1]=dec;
  if ((status=wcss2p(wcs,1,2,wcor,phi,theta,imgcrd,pixcrd,wstat)) > 0) {
    //fprintf(stderr, "wcss2p returned status=%d in wcs_rd2pix\n", status);
    fprintf(stderr,"wcss2p returned status=%d wstat=%d in wcs_rd2pix\n", status, wstat[0]);
    return(-1);
  }
  *x = pixcrd[0];
  *y = pixcrd[1];

  return(0);
}

