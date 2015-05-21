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

// Given a wcs struct, convert image x,y to sky ra,dec (in deg).
// Header arg necessary to check for sip distortions which are not handled by wcslib.
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
// Header arg necessary to check for sip distortions which are not handled by wcslib.
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

// Read in SIP keywords from header.
// Returns: sip parameter structure pointer
//          sparam->have_sip set to 0 if no sip params fournd and rest left undefined
// Function returns 0 on success and -1 on failure
int sip_read(fitsfile *fptr, struct sip_param *sparam) {
   char tmp_key[100]; // temp header key string
   char tmp_str[100]; // temp header key string value
   int tmp_int; // fits header key int value
   double tmp_double; // fits header key double value
   char tmp_comment[100]; // fits header comment
   int status=0;  // cfitsio error status
   int i,j; // loop counters

   // First check if the wcs has SIP distortions or not using the CTYPE1 card.
   if (fits_read_key(fptr, TSTRING, "CTYPE1", tmp_str, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   if (strstr(tmp_str, "-SIP") == NULL) {
      printf("The fits header does not have SIP parameters\n");
      sparam->have_sip=0;
      return(0);      
   } else {
      sparam->have_sip=1;
   }

   // Read in crval1, crval2, crpix1, crpix2
   if (fits_read_key(fptr, TDOUBLE, "CRVAL1", &tmp_double, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->crval1=tmp_double;
   if (fits_read_key(fptr, TDOUBLE, "CRVAL2", &tmp_double, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->crval2=tmp_double;
   if (fits_read_key(fptr, TDOUBLE, "CRPIX1", &tmp_double, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->crpix1=tmp_double;
   if (fits_read_key(fptr, TDOUBLE, "CRPIX2", &tmp_double, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->crpix2=tmp_double;

   // Next read in the sip order parameters
   if (fits_read_key(fptr, TINT, "A_ORDER", &tmp_int, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->a_order=tmp_int;
   if (fits_read_key(fptr, TINT, "B_ORDER", &tmp_int, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->b_order=tmp_int;
   if (fits_read_key(fptr, TINT, "AP_ORDER", &tmp_int, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->ap_order=tmp_int;
   if (fits_read_key(fptr, TINT, "BP_ORDER", &tmp_int, tmp_comment, &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   sparam->bp_order=tmp_int;

   // Now populate matrices
   for (i=0; i<=sparam->a_order; i++) {
      for (j=0; j<=sparam->a_order; j++) {
         sprintf(tmp_key,"A_%d_%d",i,j);
         if (fits_read_key(fptr,TDOUBLE, tmp_key, &tmp_double, tmp_comment, &status)) {
            //fits_report_error(stderr, status);
            sparam->a[i][j]=0.0;
            status=0;
         } else {
            sparam->a[i][j]=tmp_double;
         }
      }
   }
   for (i=0; i<=sparam->b_order; i++) {
      for (j=0; j<=sparam->b_order; j++) {
         sprintf(tmp_key,"B_%d_%d",i,j);
         if (fits_read_key(fptr,TDOUBLE, tmp_key, &tmp_double, tmp_comment, &status)) {
            //fits_report_error(stderr, status);
            sparam->b[i][j]=0.0;
            status=0;
         } else {
            sparam->b[i][j]=tmp_double;
         }
      }
   }
   for (i=0; i<=sparam->ap_order; i++) {
      for (j=0; j<=sparam->ap_order; j++) {
         sprintf(tmp_key,"AP_%d_%d",i,j);
         if (fits_read_key(fptr,TDOUBLE, tmp_key, &tmp_double, tmp_comment, &status)) {
            //fits_report_error(stderr, status);
            sparam->ap[i][j]=0.0;
            status=0;
         } else {
            sparam->ap[i][j]=tmp_double;
         }
      }
   }
   for (i=0; i<=sparam->bp_order; i++) {
      for (j=0; j<=sparam->bp_order; j++) {
         sprintf(tmp_key,"BP_%d_%d",i,j);
         if (fits_read_key(fptr,TDOUBLE, tmp_key, &tmp_double, tmp_comment, &status)) {
            //fits_report_error(stderr, status);
            sparam->bp[i][j]=0.0;
            status=0;
         } else {
            sparam->bp[i][j]=tmp_double;
         }
      }
   }

   return(0);
}

// Apply SIP distortion if forward direction (going from image to sky/earth)
int sip_forward(struct sip_param *sparam, double x, double y, double *xout, double *yout) {
   double f,g; // sip polynomial sums for x,y respectively
   double u,v; // pix values relative to crpix1 and crpix2
   int i,j; // loop counters

   // get relative pixel locations
   u=x-sparam->crpix1;
   v=y-sparam->crpix2;

   // calculate f and xout
   f=0;
   for (i=0; i<=sparam->a_order; i++) {
      for (j=0; j<=sparam->a_order; j++) {
         f=f+(sparam->a[i][j]*pow(u,i)*pow(v,j));
      }
   }  
   *xout=x+f;

   // calculate g and yout
   g=0;
   for (i=0; i<=sparam->b_order; i++) {
      for (j=0; j<=sparam->b_order; j++) {
         g=g+(sparam->b[i][j]*pow(u,i)*pow(v,j));
      }
   }  
   *yout=y+g;

   return(0);
}

// Apply SIP distortion in reverse direction (going from sky/earth to image)
int sip_reverse(struct sip_param *sparam, double x, double y, double *xout, double *yout) {
   double f,g; // sip polynomial sums for x,y respectively
   double u,v; // pix values relative to crpix1 and crpix2
   int i,j; // loop counters

   // get relative pixel locations
   u=x-sparam->crpix1;
   v=y-sparam->crpix2;

   // calculate f and xout
   f=0;
   for (i=0; i<=sparam->ap_order; i++) {
      for (j=0; j<=sparam->ap_order; j++) {
         f=f+(sparam->ap[i][j]*pow(u,i)*pow(v,j));
      }
   }  
   *xout=x+f;

   // calculate g and yout
   g=0;
   for (i=0; i<=sparam->bp_order; i++) {
      for (j=0; j<=sparam->bp_order; j++) {
         g=g+(sparam->bp[i][j]*pow(u,i)*pow(v,j));
      }
   }  
   *yout=y+g;

   return(0);
}
