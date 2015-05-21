//
// Primary file for libsquid_wcs library which handles wcs info for squid tile images
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

// Transform image coords to squid and append to squidarr if not there already
int wcs_addsquid(int projection, struct wcsprm *wcs, int k, double x, double y, squid_type squidarr[], long squidarr_len, long *squidarr_used) {
   double ra_deg, dec_deg;
   double ra_rad, dec_rad;
   squid_type squid;
   long i;

   // Test for NULL counter
   if (NULL == squidarr_used) {
      fprintf(stderr, "NULL counter (NULL == squidarr_used) in wcs_addsquid\n");
      return(-1);
   }

   // Test for full array
   if (squidarr_len <= (*squidarr_used)) {
      fprintf(stderr, "Full array (%li <= %li) (squidarr_len <= (*squidarr_used)) in wcs_addsquid\n", squidarr_len, (*squidarr_used));
      return(-1);
   }

   // Transform (x, y) to (ra, dec)
   if (0 != wcs_pix2rd(wcs, x, y, &ra_deg, &dec_deg)) {
      fprintf(stderr, "wcs_pix2rd failed in wcs_addsquid\n");
      return(-1);
   }

   // Degrees to radians
   ra_rad  = fmod(ra_deg*DD2R+2*PI,2*PI);
   dec_rad = dec_deg*DD2R;

   // Transform (ra,dec) to squid
   if (0 != sph2squid(projection, ra_rad, dec_rad, k, &squid)) {
      fprintf(stderr, "sph2squid failed in wcs_addsquid\n");
      return(-1);
   }

   // Test for duplicates
   for (i=0; i<(*squidarr_used); i++) {
      if (squid == squidarr[i]) {
         // squid aready in squidarr
         return(0);
      }
   }

   // Append squid
   squidarr[(*squidarr_used)] = squid;
   ++(*squidarr_used);

   return(0);
}

// Get array of squid ids at k that are within image
// squidarr should be pre-allocated with size squidarr_len
// When starting from zero, squidarr_used is number of ids found
int wcs_getsquids(int projection, struct wcsprm *wcs, double cdelt, long naxes[], int k, squid_type squidarr[], long squidarr_len, long *squidarr_used) {
   double N; // Nside
   double omega; // healpix width in deg
   double step; // step size
   double x,y; // img coords

   // Test for NULL counter
   if (NULL == squidarr_used) {
      fprintf(stderr, "NULL counter (NULL == squidarr_used) in wcs_getsquids\n");
      return(-1);
   }

   // Test for full array
   if (squidarr_len <= (*squidarr_used)) {
      fprintf(stderr, "Full array (%li <= %li) (squidarr_len <= (*squidarr_used)) in wcs_getsquids\n", squidarr_len, (*squidarr_used));
      return(-1);
   }

   N=pow(2,(double)k);
   omega=90.0/N; // in degrees
   step=floor(omega/(4*cdelt));
   //printf("omega=%.3f step=%.3f\n",omega,step);

   // first search all pix on edge of image
   x=0;
   for (y=0; y<naxes[1]; y=y+1) {
      if (0 != wcs_addsquid(projection, wcs, k, x, y, squidarr, squidarr_len, squidarr_used)) {
         fprintf(stderr, "wcs_addsquid failed in wcs_getsquids\n");
         return(-1);
      }
   }
   x=naxes[0]-1;
   for (y=0; y<naxes[1]; y=y+1) {
      if (0 != wcs_addsquid(projection, wcs, k, x, y, squidarr, squidarr_len, squidarr_used)) {
         fprintf(stderr, "wcs_addsquid failed in wcs_getsquids\n");
         return(-1);
      }
   }
   y=0;
   for (x=0; x<naxes[0]; x=x+1) {
      if (0 != wcs_addsquid(projection, wcs, k, x, y, squidarr, squidarr_len, squidarr_used)) {
         fprintf(stderr, "wcs_addsquid failed in wcs_getsquids\n");
         return(-1);
      }
   }
   y=naxes[1]-1;
   for (x=0; x<naxes[0]; x=x+1) {
      if (0 != wcs_addsquid(projection, wcs, k, x, y, squidarr, squidarr_len, squidarr_used)) {
         fprintf(stderr, "wcs_addsquid failed in wcs_getsquids\n");
         return(-1);
      }
   }

   // now search grid in interior
   for (y=0; y<naxes[1]; y=y+step) {
      for (x=0; x<naxes[0]; x=x+step) {
         if (0 != wcs_addsquid(projection, wcs, k, x, y, squidarr, squidarr_len, squidarr_used)) {
            fprintf(stderr, "wcs_addsquid failed in wcs_getsquids\n");
            return(-1);
         }
      }
   }

   return(0);
}

// Get wcs struct for a given squid for any tside
// Here tside is the number of pixels per side of the tile.
int tile_getwcs(int projection, squid_type squid, squid_type tside, struct wcsprm **wcs) {
   double ra,dec;

   // Make sure squid is valid
   if (squid_validate(squid) == 0) {
      fprintf(stderr,"invalid squid argument in tile_getwcs\n");
      return(-1);
   }

   squid2sph(projection,squid,&ra,&dec);
   if ((projection == TSC)||(projection == CSC)||(projection == QSC)) {
      if (quadcube_getwcs(projection, squid, tside, wcs) == -1) {
         fprintf(stderr,"quadcube_getwcs failed in tile_getwcs\n");
         return(-1);
      }
   } else if (projection == HSC) {
      if (fabs(dec) < THETAX) {
         if (hsc_getwcs_equator(squid, tside, wcs) == -1) {
            fprintf(stderr,"hsc_getwcs_equator failed in tile_getwcs\n");
            return(-1);
         }
      } else {
         if (hsc_getwcs_pole(squid, tside, wcs) == -1) {
            fprintf(stderr,"hsc_getwcs_pole failed in tile_getwcs\n");
            return(-1);
         }
      }
   } else {
      fprintf(stderr,"unknown projection in tile_getwcs\n");
      return(-1);
   }

   return(0);
}


// Get quadcube projection wcs struct for a given squid for any tside.
// Here tside is the number of pixels per side of the tile.
// This works for TSC, CSC, and QSC, but not HSC.
int quadcube_getwcs(int projection, squid_type squid, squid_type tside, struct wcsprm **wcs) {
   struct wcsprm *wcs0; // temp wcs pointer
   double pc11,pc12,pc21,pc22; //wcs rotation parameters
   double wx,wy,wxr,wyr; // world wcs pix coords
   double rac,decc; // img center coords
   double crval1,crval2,cdelt1,cdelt2,crpix1,crpix2;
   char fithdr[HDR_MAXLEN];
   char naxis1card[81],naxis2card[81];
   char ctype1card[81],ctype2card[81];
   char crval1card[81],crpix1card[81],cdelt1card[81];
   char crval2card[81],crpix2card[81],cdelt2card[81];
   char pc11card[81],pc12card[81],pc21card[81],pc22card[81];
   int status, nreject, nwcs; // output from wcslib
   int k, face; // squid resolution, quadcube face number
   double rot; // image rotation in deg
   long latpole,lonpole; // pole of ref coord system
   char latpolecard[81],lonpolecard[81]; // header cards
   double fx, fy; // face xy coords (goes from 0 to 1 across face)
   double fxx, fyy; // same as fx,fy but goes from -45 to 45 across face

   // get center coords
   if ((k=squid_getres(squid)) < 0) {
      fprintf(stderr,"squid_getres failed in quadcube_getwcs\n");
      return(-1);
   }
   if (squid2sph(projection,squid,&rac,&decc) < 0) {
      fprintf(stderr,"squid2sph failed in quadcube_getwcs\n");
      return(-1);
   }
   if (sph2xyf(projection, rac, decc, &fx, &fy, &face) < 0) {
      fprintf(stderr,"sph2xyf failed in quadcube_getwcs\n");
      return(-1);
   }

   // compute image rotation values
   rot=0.0; // projection rotation
   crval1=LON_POLE/DD2R; // ra of reference
   crval2=0.0; // dec of reference
   lonpole=180; // longitude of reference
   latpole=0; // latitiude of reference

   // rotation matrix
   pc11=cos(rot*DD2R);
   pc21=sin(rot*DD2R);
   pc12=-1*pc21;
   pc22=pc11;

   // get world x,y for wcs
   fxx=LON_DIR*45.0*(2.0*fx-1.0);
   fyy=45.0*(2.0*fy-1.0);
   if (face == 0) {
      wx=fxx;
      wy=fyy+90.0;
   } else if (face == 1) {
      wx=fxx;
      wy=fyy;
   } else if (face == 2) {
      wx=fxx+90.0;
      wy=fyy;
   } else if (face == 3) {
      wx=fxx+180.0;
      wy=fyy;
   } else if (face == 4) {
      wx=fxx-90.0;
      wy=fyy;
   } else if (face == 5) {
      wx=fxx;
      wy=fyy-90.0;
   } else {
      fprintf(stderr,"invalid face in quadcube_getwcs\n");
      return(-1);
   }
   wxr=pc11*wx+pc12*wy;
   wyr=pc21*wx+pc22*wy;

   // compute wcs header values
   cdelt2=90.0/((double)tside*pow(2,k)); 
   cdelt1=LON_DIR*cdelt2;
   crpix1=((double)tside/2.0)-(wxr/cdelt1);
   crpix2=((double)tside/2.0)-(wyr/cdelt2);

   // set up variable header strings
   sprintf(naxis1card,"NAXIS1  = %20ld / length of axis 1                               ",(long)tside); 
   sprintf(naxis2card,"NAXIS2  = %20ld / length of axis 2                               ",(long)tside);
   if (projection == TSC) {
      sprintf(ctype1card,"CTYPE1  = 'RA---TSC'           / Tangental Spherical Cube RA                    ");   
      sprintf(ctype2card,"CTYPE2  = 'DEC--TSC'           / Tangental Spherical Cube DEC                   ");   
   } else if (projection == CSC) {
      sprintf(ctype1card,"CTYPE1  = 'RA---CSC'           / COBE Spherical Cube RA                         ");   
      sprintf(ctype2card,"CTYPE2  = 'DEC--CSC'           / COBE Spherical Cube DEC                        ");   
   } else if (projection == QSC) {
      sprintf(ctype1card,"CTYPE1  = 'RA---QSC'           / Quadrilateral Spherical Cube RA                ");   
      sprintf(ctype2card,"CTYPE2  = 'DEC--QSC'           / Quadrilateral Spherical Cube DEC               ");   
   } else {
      fprintf(stderr,"unrecognized projection for quadcube_getwcs\n");
      return(-1);
   }
   sprintf(crval1card,"CRVAL1  = %20.15f / ref world                                      ",crval1);
   sprintf(crpix1card,"CRPIX1  = %20.12f / ref pix                                        ",crpix1);
   sprintf(cdelt1card,"CDELT1  = %20.15f / deg/pix                                        ",cdelt1);
   sprintf(crval2card,"CRVAL2  = %20.15f / ref world                                      ",crval2);
   sprintf(crpix2card,"CRPIX2  = %20.12f / ref pix                                        ",crpix2);
   sprintf(cdelt2card,"CDELT2  = %20.15f / deg/pix                                        ",cdelt2);
   sprintf(pc11card,"PC1_1   = %20.15f / rotation matrix                                ",pc11);
   sprintf(pc12card,"PC1_2   = %20.15f / rotation matrix                                ",pc12);
   sprintf(pc21card,"PC2_1   = %20.15f / rotation matrix                                ",pc21);
   sprintf(pc22card,"PC2_2   = %20.15f / rotation matrix                                ",pc22);
   sprintf(lonpolecard,"LONPOLE = %20ld / native lon of pole                             ",lonpole);
   sprintf(latpolecard,"LATPOLE = %20ld / native lat of pole                             ",latpole);

   // create fithdr string
   fithdr[0]='\0';
   strncat(fithdr,"SIMPLE  =                    T / file does conform to FITS standard             ",81);   
   strncat(fithdr,"BITPIX  =                   16 / number of bits per data pixel                  ",81);   
   strncat(fithdr,"NAXIS   =                    2 / number of data axes                            ",81);   
   strncat(fithdr,naxis1card,81);   
   strncat(fithdr,naxis2card,81);
   strncat(fithdr,"EXTEND  =                    T / FITS dataset may contain extensions            ",81);   
   strncat(fithdr,ctype1card,81);   
   strncat(fithdr,crval1card,81);   
   strncat(fithdr,crpix1card,81);   
   strncat(fithdr,cdelt1card,81);   
   strncat(fithdr,ctype2card,81);   
   strncat(fithdr,crval2card,81);   
   strncat(fithdr,crpix2card,81);   
   strncat(fithdr,cdelt2card,81);   
   strncat(fithdr,pc11card,81);   
   strncat(fithdr,pc12card,81);   
   strncat(fithdr,pc21card,81);   
   strncat(fithdr,pc22card,81);   
   strncat(fithdr,latpolecard,81);   
   strncat(fithdr,lonpolecard,81);   
   strncat(fithdr,"END                                                                             ",81);   

   // Interpret the WCS keywords to create wcs struct
   if ((status = wcspih(fithdr, 21, WCSHDR_all, -3, &nreject, &nwcs, &wcs0))) {
      fprintf(stderr, "wcspih ERROR %d: %s.\n", status, wcshdr_errmsg[status]);
      return(-1);
   }
   *wcs=wcs0;

   return(0);

}  

// Get HSC projection wcs struct for a given squid for any tside
// Here tside is the number of pixels per side of the tile.
int hsc_getwcs_pole(squid_type squid, squid_type tside, struct wcsprm **wcs) {
   struct wcsprm *wcs0; // temp wcs pointer
   double pc11,pc12,pc21,pc22; //wcs rotation parameters
   double wx,wy,wxf,wyf;
   double xc,yc,rac,decc; // img center coords
   double crrot; // rotation angle for crval1
   double crval1,crval2,cdelt1,cdelt2,crpix1,crpix2;
   char fithdr[HDR_MAXLEN];
   char naxis1card[81],naxis2card[81];
   char crval1card[81],crpix1card[81],cdelt1card[81];
   char crval2card[81],crpix2card[81],cdelt2card[81];
   char pc11card[81],pc12card[81],pc21card[81],pc22card[81];
   char latpolecard[81], lonpolecard[81];
   int status, nreject, nwcs; // libwcs output
   int k,face; // squid resolution, face number
   double rot; // image rotation in deg
   squid_type tside2; // # pix across entire face

   // get center coords
   k=squid_getres(squid);
   tside2=(squid_type)pow(2,k)*tside;
   face=squid_getface(squid);
   squid2sph(HSC,squid,&rac,&decc);

   // reference coords
   xc=tside/2.0;
   yc=tside/2.0;

   // compute image rotation values
   crrot=LON_DIR*(QUARTPI+LON_POLE); // rotation value for ocrval1
   if ((LON_DIR*decc) < 0) {
      rot=225.0; // projection rotation
      crval1=fmod(crrot/DD2R+360.0,360.0); // ra of pole
      crval2=-90.0*LON_DIR; // dec of pole
   } else {
      rot=135.0; // projection rotation
      crval1=fmod(-1.0*crrot/DD2R+360.0,360.0); // ra of pole
      crval2=90.0*LON_DIR; // dec of pole
   }
   pc11=cos(rot*DD2R);
   pc21=sin(rot*DD2R);
   pc12=-1*pc21;
   pc22=pc11;

   if (sph2xyf(HSC,rac,decc,&wxf,&wyf,&face) == -1) {
      printf("sph2xyf failed!\n");
      return(-1);
   }
   wx=tside2*(0.5-wxf); // world x coord of pole
   wy=tside2*(0.5-wyf); // world y coord of pole

   cdelt2=90.0/((double)tside*pow(2,k)); 
   cdelt1=LON_DIR*cdelt2;
   crpix1=wx+xc;
   crpix2=wy+yc;

   // set up variable header strings
   sprintf(naxis1card,"NAXIS1  = %20ld / length of axis 1                               ",(long)tside); 
   sprintf(naxis2card,"NAXIS2  = %20ld / length of axis 2                               ",(long)tside);
   sprintf(latpolecard,"LATPOLE = %20.15f / native lat of pole                             ",crval2);
   sprintf(lonpolecard,"LONPOLE = %20.15f / native lon of pole                             ",crval1);
   sprintf(crval1card,"CRVAL1  = %20.15f / ref world                                      ",0.0);
   sprintf(crpix1card,"CRPIX1  = %20.12f / ref pix                                        ",crpix1);
   sprintf(cdelt1card,"CDELT1  = %20.15f / deg/pix                                        ",cdelt1);
   sprintf(crval2card,"CRVAL2  = %20.15f / ref world                                      ",crval2);
   sprintf(crpix2card,"CRPIX2  = %20.12f / ref pix                                        ",crpix2);
   sprintf(cdelt2card,"CDELT2  = %20.15f / deg/pix                                        ",cdelt2);
   sprintf(pc11card,"PC1_1   = %20.15f / rotation matrix                                ",pc11);
   sprintf(pc12card,"PC1_2   = %20.15f / rotation matrix                                ",pc12);
   sprintf(pc21card,"PC2_1   = %20.15f / rotation matrix                                ",pc21);
   sprintf(pc22card,"PC2_2   = %20.15f / rotation matrix                                ",pc22);

   // create fithdr string
   fithdr[0]='\0';
   strncat(fithdr,"SIMPLE  =                    T / file does conform to FITS standard             ",81);   
   strncat(fithdr,"BITPIX  =                   16 / number of bits per data pixel                  ",81);   
   strncat(fithdr,"NAXIS   =                    2 / number of data axes                            ",81);   
   strncat(fithdr,naxis1card,81);   
   strncat(fithdr,naxis2card,81);
   strncat(fithdr,"EXTEND  =                    T / FITS dataset may contain extensions            ",81);   
   strncat(fithdr,"CTYPE1  = 'RA---XPH'           / Healpix RA                                     ",81);   
   strncat(fithdr,crval1card,81);   
   strncat(fithdr,crpix1card,81);   
   strncat(fithdr,cdelt1card,81);   
   strncat(fithdr,"CTYPE2  = 'DEC--XPH'           / Healpix DEC                                    ",81);   
   strncat(fithdr,crval2card,81);   
   strncat(fithdr,crpix2card,81);   
   strncat(fithdr,cdelt2card,81);   
   strncat(fithdr,pc11card,81);   
   strncat(fithdr,pc12card,81);   
   strncat(fithdr,pc21card,81);   
   strncat(fithdr,pc22card,81);   
   strncat(fithdr,latpolecard,81);   
   strncat(fithdr,lonpolecard,81);   
   strncat(fithdr,"END                                                                             ",81);   

   // Interpret the WCS keywords to create wcs struct
   if ((status = wcspih(fithdr, 21, WCSHDR_all, -3, &nreject, &nwcs, &wcs0))) {
      fprintf(stderr, "wcspih ERROR %d: %s.\n", status, wcshdr_errmsg[status]);
      return(-1);
   }
   *wcs=wcs0;

   return(0);

}

// Get wcs struct for a given squid for any tside
// Here tside is the number of pixels per side of the tile.
int hsc_getwcs_equator(squid_type squid, squid_type tside, struct wcsprm **wcs) {
   struct wcsprm *wcs0; // temp wcs pointer
   double pc11,pc12,pc21,pc22; //wcs rotation parameters
   double ra,ra2,dec,wx,wy,wxr,wyr;
   double xc,yc,rac,decc; // img center coords
   double crval1,crval2,cdelt1,cdelt2,crpix1,crpix2;
   char fithdr[HDR_MAXLEN];
   char naxis1card[81],naxis2card[81];
   char crval1card[81],crpix1card[81],cdelt1card[81];
   char crval2card[81],crpix2card[81],cdelt2card[81];
   char pc11card[81],pc12card[81],pc21card[81],pc22card[81];
   int status, nreject, nwcs; // output from libwcs
   int k; // squid resolution parameter
   double rot; // image rotation angle in deg
   long latpole,lonpole; // wcs projection pole coords
   char latpolecard[81],lonpolecard[81];

   // get center coords
   k=squid_getres(squid);
   squid2sph(HSC,squid,&rac,&decc);

   // reference coords
   ra=rac/DD2R;
   dec=decc/DD2R;
   xc=tside/2.0;
   yc=tside/2.0;

   // compute image rotation values
   rot=0.0; // projection rotation
   crval1=LON_POLE/DD2R; // ra of reference
   crval2=0.0; // dec of reference
   lonpole=180; // longitude of reference
   latpole=0; // latitiude of reference

   pc11=cos(rot*DD2R);
   pc21=sin(rot*DD2R);
   pc12=-1*pc21;
   pc22=pc11;

   if (ra < 180.0) {
      ra2=ra;
   } else {
      ra2=ra-360.0;
   }
   wx=ra2-LON_POLE/DD2R;
   wy=67.5*sin(dec*DD2R);
   wxr=pc11*wx+pc12*wy;
   wyr=pc21*wx+pc22*wy;

   cdelt2=90.0/((double)tside*pow(2,k)); 
   cdelt1=LON_DIR*cdelt2;

   crpix1=(double)xc-wxr/cdelt1;
   crpix2=(double)yc-wyr/cdelt2;

   // set up variable header strings
   sprintf(naxis1card,"NAXIS1  = %20ld / length of axis 1                               ",(long)tside); 
   sprintf(naxis2card,"NAXIS2  = %20ld / length of axis 2                               ",(long)tside);
   sprintf(crval1card,"CRVAL1  = %20.15f / ref world                                      ",crval1);
   sprintf(crpix1card,"CRPIX1  = %20.12f / ref pix                                        ",crpix1);
   sprintf(cdelt1card,"CDELT1  = %20.15f / deg/pix                                        ",cdelt1);
   sprintf(crval2card,"CRVAL2  = %20.15f / ref world                                      ",crval2);
   sprintf(crpix2card,"CRPIX2  = %20.12f / ref pix                                        ",crpix2);
   sprintf(cdelt2card,"CDELT2  = %20.15f / deg/pix                                        ",cdelt2);
   sprintf(pc11card,"PC1_1   = %20.15f / rotation matrix                                ",pc11);
   sprintf(pc12card,"PC1_2   = %20.15f / rotation matrix                                ",pc12);
   sprintf(pc21card,"PC2_1   = %20.15f / rotation matrix                                ",pc21);
   sprintf(pc22card,"PC2_2   = %20.15f / rotation matrix                                ",pc22);
   sprintf(lonpolecard,"LONPOLE = %20ld / native lat of pole                             ",lonpole);
   sprintf(latpolecard,"LATPOLE = %20ld / native lat of pole                             ",latpole);

   // create fithdr string
   fithdr[0]='\0';
   strncat(fithdr,"SIMPLE  =                    T / file does conform to FITS standard             ",81);   
   strncat(fithdr,"BITPIX  =                   16 / number of bits per data pixel                  ",81);   
   strncat(fithdr,"NAXIS   =                    2 / number of data axes                            ",81);   
   strncat(fithdr,naxis1card,81);   
   strncat(fithdr,naxis2card,81);
   strncat(fithdr,"EXTEND  =                    T / FITS dataset may contain extensions            ",81);   
   strncat(fithdr,"CTYPE1  = 'RA---HPX'           / Healpix RA                                     ",81);   
   strncat(fithdr,crval1card,81);   
   strncat(fithdr,crpix1card,81);   
   strncat(fithdr,cdelt1card,81);   
   strncat(fithdr,"CTYPE2  = 'DEC--HPX'           / Healpix DEC                                    ",81);   
   strncat(fithdr,crval2card,81);   
   strncat(fithdr,crpix2card,81);   
   strncat(fithdr,cdelt2card,81);   
   strncat(fithdr,pc11card,81);   
   strncat(fithdr,pc12card,81);   
   strncat(fithdr,pc21card,81);   
   strncat(fithdr,pc22card,81);   
   strncat(fithdr,latpolecard,81);   
   strncat(fithdr,lonpolecard,81);   
   strncat(fithdr,"END                                                                             ",81);   

   // Interpret the WCS keywords to create wcs struct
   if ((status = wcspih(fithdr, 21, WCSHDR_all, -3, &nreject, &nwcs, &wcs0))) {
      fprintf(stderr, "wcspih ERROR %d: %s.\n", status, wcshdr_errmsg[status]);
      return(-1);
   }
   *wcs=wcs0;

   return(0);

}


//
// Add wcs header to squid tile image fits file
// 
int tile_addwcs(int projection, squid_type squid, struct wcsprm *wcs, char *ihdr, fitsfile *ofptr) {
   char *hcardx[] = CARD_EXCLUDE;
   int xlen;
   char *ihdrcpy, *ihdrcpy0;
   int status=0; // return value for cfitsio calls
   long i,j;
   double rac,decc; // img center coords
   double dtval;
   char *wheader, *wheader0, *card, *card0; // wcs header
   char *keyname; // card name
   int nkeyrec, ifkeys;
   int k;
   int ihdrlen;
   int cskip;
   char *maptype; // e.g. "TSC"

   // Make sure squid is valid
   if (squid_validate(squid) == 0) {
      fprintf(stderr,"invalid squid argument in tile_addwcs\n");
      return(-1);
   }

   // get center coords
   k=squid_getres(squid);
   if (squid2sph(projection,squid,&rac,&decc) == -1) {
      fprintf(stderr,"squid2sph failed in tile_addwcs\n");
      return(-1);
   }
   rac=rac/DD2R;
   decc=decc/DD2R;

   //
   // Add header from original image
   //
   ihdrcpy=strdup(ihdr);
   ihdrcpy0=ihdrcpy;
   ihdrlen=strlen(ihdr);
   ifkeys=ihdrlen/80-1;
   // loop over keys from original image
   for (i=0; i<ifkeys; i++) {
      // get single card
      card=strndup(ihdrcpy,80);
      ihdrcpy=ihdrcpy+80; // advance to next card
      // first check if card is a comment
      if (strncmp("COMMENT",card,7) == 0) {
         if (fits_write_comment(ofptr, card, &status)) {
            fprintf(stderr,"fits_update_card failed in tile_addwcs\n");
            fits_report_error(stderr, status);
            free(card);
            return(-1);
         }
         free(card);
         continue;
      }
      // check if card is on excluded list
      j=0;
      cskip=0;
      while (hcardx[j] != NULL) {
         xlen=strlen(hcardx[j]);
         if (strncmp(hcardx[j],card,xlen) == 0) {
            cskip=1;
            break;
         }
         j++;
      }
      // take out SIP parameters
      if (strncmp("A_",card,2) == 0) cskip=1;
      else if (strncmp("B_",card,2) == 0) cskip=1;
      else if (strncmp("AP_",card,3) == 0) cskip=1;
      else if (strncmp("BP_",card,3) == 0) cskip=1;
      if (cskip) {
         free(card);
         continue;
      }
      // get key name
      card0=strndup(card,80);
      keyname=strtok(card0,"=");
      if (keyname == NULL) {
         free(card0);
         continue;
      }
      // update card in new header
      if (fits_update_card(ofptr, keyname, card, &status)) {
         fprintf(stderr,"fits_update_card failed in tile_addwcs\n");
         fits_report_error(stderr, status);
         return(-1);
      }
      free(keyname); // frees card0 as well
      free(card);
   }
   free(ihdrcpy0);

   //
   // write wcs header values
   //
   if (projection == TSC) {
      maptype=strdup("TSC");
   } else if (projection == CSC) {
      maptype=strdup("CSC");
   } else if (projection == QSC) {
      maptype=strdup("QSC");
   } else if (projection == HSC) {
      maptype=strdup("HSC");
   } else {
      maptype=strdup("???");
   }
   if (fits_update_key(ofptr, TSTRING, "MAPTYPE", maptype,
            "Map Projection Type", &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   free(maptype);
   if (fits_update_key(ofptr, TLONG, "MAPID", &squid,
            "Map ID of Image Region", &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   if (fits_update_key(ofptr, TINT, "MAPRES", &k,
            "Map Resolution Parameter", &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }

   if ((status=wcshdo(0, wcs, &nkeyrec, &wheader)) > 0) {
      fprintf(stderr,"wcshdo failed in tile_addwcs, status=%d\n",status);
      return(-1);
   }
   wheader0=wheader;

   for (i=0; i < nkeyrec; i++) {
      card=strndup(wheader,80);
      if (strstr(card,"RESTFRQ") != NULL) {
         free(card);
         continue;
      }
      if (strstr(card,"RESTWAV") != NULL) {
         free(card);
         continue;
      }
      card0=strdup(card);
      if (card0 == NULL) continue;
      keyname=strtok(card0,"=");
      if (keyname == NULL) continue;
      if (fits_update_card(ofptr, keyname, card, &status)) {
         fprintf(stderr,"tile_addwcs: fits_update_card failed on %s, status=%d\n",keyname,status);
         return(-1);
      }
      free(keyname); // frees card0 as well
      free(card);
      wheader=wheader+80;
   }
   free(wheader0);

   dtval=rac;
   if (fits_update_key(ofptr, TDOUBLE, "CENTER1", &dtval,
            "img center RA (deg)", &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }
   dtval=decc;
   if (fits_update_key(ofptr, TDOUBLE, "CENTER2", &dtval,
            "img center DEC (deg)", &status)) {
      fits_report_error(stderr, status);
      return(-1);
   }

   return(0);

}

