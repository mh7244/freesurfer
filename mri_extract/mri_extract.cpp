/**
 * @file  mri_extract.c
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR 
 * CVS Revision Info:
 *    $Author: fischl $
 *    $Date: 2016/06/08 13:42:17 $
 *    $Revision: 1.8 $
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "mri.h"
#include "macros.h"
#include "error.h"
#include "diag.h"
#include "proto.h"
#include "version.h"

int main(int argc, char *argv[]) ;
static int get_option(int argc, char *argv[]) ;

const char *Progname ;
static int reductions = 1 ;
static int verbose = 0 ;
static int pad = 0 ;
static float thresh = 0.0 ;

int
main(int argc, char *argv[]) {
  char   **av ;
  int    ac, nargs, x0, y0, z0, dx, dy, dz ;
  MRI    *mri_src, *mri_dst = NULL ;
  char   *in_dir, *out_dir ;
  MRI_REGION box ;

  /* rkt: check for and handle version tag */
  nargs = handle_version_option (argc, argv, "$Id: mri_extract.c,v 1.8 2016/06/08 13:42:17 fischl Exp $", "$Name:  $");
  if (nargs && argc - nargs == 1)
    exit (0);
  argc -= nargs;

  Progname = argv[0] ;
  ErrorInit(NULL, NULL, NULL) ;
  DiagInit(NULL, NULL, NULL) ;

  ac = argc ;
  av = argv ;
  for ( ; argc > 1 && ISOPTION(*argv[1]) ; argc--, argv++) {
    if (isdigit(*(argv[1]+1)))
      break ; // not an option - a negative number
    nargs = get_option(argc, argv) ;
    argc -= nargs ;
    argv += nargs ;
  }

  /*
     command line:

     mri_extract <src_dir> x0 y0 z0 dx dy dz <dst_dir>
  */

  if (argc < 8)
    ErrorExit(ERROR_BADPARM,
              "usage: %s <src volume> x0 y0 z0 dx dy dz <dst volume>", Progname) ;

  in_dir = argv[1] ;
  if (sscanf(argv[2], "%d", &x0) != 1)
    ErrorExit(ERROR_BADPARM,
              "%s: could not scan x0 from '%s'", Progname, argv[2]) ;
  if (sscanf(argv[3], "%d", &y0) != 1)
    ErrorExit(ERROR_BADPARM,
              "%s: could not scan y0 from '%s'", Progname, argv[3]) ;
  if (sscanf(argv[4], "%d", &z0) != 1)
    ErrorExit(ERROR_BADPARM,
              "%s: could not scan z0 from '%s'", Progname, argv[4]) ;
  if (sscanf(argv[5], "%d", &dx) != 1)
    ErrorExit(ERROR_BADPARM,
              "%s: could not scan dx from '%s'", Progname, argv[5]) ;
  if (sscanf(argv[6], "%d", &dy) != 1)
    ErrorExit(ERROR_BADPARM,
              "%s: could not scan dy from '%s'", Progname, argv[6]) ;
  if (sscanf(argv[7], "%d", &dz) != 1)
    ErrorExit(ERROR_BADPARM,
              "%s: could not scan dz from '%s'", Progname, argv[7]) ;

  out_dir = argv[8] ;

  if (verbose)
    fprintf(stderr, "reading from %s...", in_dir) ;
  mri_src = MRIread(in_dir) ;
  if (verbose)
    fprintf(stderr, "done\n") ;

  if (!mri_src)
    exit(1) ;

  MRIboundingBox(mri_src, thresh, &box) ;
  if (x0 < 0)
    x0 = box.x-pad ;
  if (y0 < 0)
    y0 = box.y-pad ;
  if (z0 < 0)
    z0 = box.z-pad ;
  if (dx < 0)
    dx = box.dx+2*pad ;
  if (dy < 0)
    dy = box.dy+2*pad ;
  if (dz < 0)
    dz = box.dz+2*pad ;
  printf("using bounding box (%d, %d, %d) -> (%d, %d, %d)\n", x0, y0,z0, x0+dx-1, y0+dy-1,z0+dz-1) ;

  mri_dst = MRIextract(mri_src, NULL, x0, y0, z0, dx, dy, dz) ;

  if (!mri_dst)
    exit(1) ;

  if (verbose)
    fprintf(stderr, "\nwriting to %s", out_dir) ;
  MRIwrite(mri_dst, out_dir) ;
  if (verbose)
    fprintf(stderr, "\n") ;
  exit(0) ;
  return(0) ;
}

/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int
get_option(int argc, char *argv[]) {
  int  nargs = 0 ;
  char *option ;

  option = argv[1] + 1 ;            /* past '-' */
  switch (toupper(*option)) {
  case 'V':
    verbose = 1 ;
    break ;
  case 'P':
    pad = atoi(argv[2]) ;
    printf("using pad = %d\n", pad) ;
    nargs = 1 ;
    break ;
  case 'T':
    thresh = atof(argv[2]) ;
    printf("using threshold = %f\n", thresh) ;
    nargs = 1 ;
    break ;
  case 'N':
    sscanf(argv[2], "%d", &reductions) ;
    fprintf(stderr, "reducing %d times\n", reductions) ;
    nargs = 1 ;
    break ;
  case '?':
  case 'U':
    printf("usage: %s [input directory] [output directory]\n", argv[0]) ;
    exit(1) ;
    break ;
  default:
    fprintf(stderr, "unknown option %s\n", argv[1]) ;
    exit(1) ;
    break ;
  }

  return(nargs) ;
}
