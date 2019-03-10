/**
 * @file  mris_seg2annot.c
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR 
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2014/11/15 00:07:19 $
 *    $Revision: 1.10 $
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


// $Id: mris_seg2annot.c,v 1.10 2014/11/15 00:07:19 greve Exp $

/*
  BEGINHELP

Converts a surfaced-based segmentation into a custom annotation file.

--seg surfseg

Surface segmentation file. This could be as simple as a binarized
functional map. The values are whole numbers indicating the index into
the color table. This file is similar to the volume-based aseg.  The
hard part to getting a custom annotation is in creating this file and
corresponding color table.

--ctab colortable

Color table used to map segmentation index to name and color. This is
something that can be created by the user to create custom
annotations. This color table is then imbedded in the annotation
file. Be default, it will look for this file in $FREESURFER_HOME.  If
this is not where your color table is, then add a './' in front of the
name. The format should be the same as in
$FREESURFER_HOME/FreeSurferColorsLUT.txt.

--s subject
--h hemi

Subject and hemisphere. Used to load in the surface upon which the
annotation is created.

--o annot

Output annotation file. By default, it will be stored in the subject's
label directory. If you do not want it there, then supply some path
in front of it (eg, './'). This is a file like lh.aparc.annot.

EXAMPLE:

  mris_seg2annot --seg lh.FL_002.sig.th8.mgh \\
    --s FL_002 --h lh --ctab ./MyColorLUT.txt \\
    --o ./lh.myaparc.annot

lh.myaparc.annot can then be loaded into tksurfer as with any other
parcellation/annotation.

  ENDHELP
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
double round(double x);
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "macros.h"
#include "utils.h"
#include "mrisurf.h"
#include "mrisutils.h"
#include "error.h"
#include "diag.h"
#include "mri.h"
#include "mri2.h"
#include "fio.h"
#include "version.h"
#include "label.h"
#include "matrix.h"
#include "annotation.h"
#include "fmriutils.h"
#include "cmdargs.h"
#include "fsglm.h"
#include "pdf.h"
#include "fsgdf.h"
#include "timer.h"
#include "matfile.h"
#include "volcluster.h"
#include "surfcluster.h"

static int  parse_commandline(int argc, char **argv);
static void check_options(void);
static void print_usage(void) ;
static void usage_exit(void);
static void print_help(void) ;
static void print_version(void) ;
static void dump_options(FILE *fp);
int main(int argc, char *argv[]) ;

static char vcid[] = "$Id: mris_seg2annot.c,v 1.10 2014/11/15 00:07:19 greve Exp $";
const char *Progname = NULL;
char *cmdline, cwd[2000];
int debug=0;
int checkoptsonly=0;
struct utsname uts;

static int annot = 0 ;

char *surfsegfile=NULL;
char *subject=NULL, *hemi=NULL;
char *ctabfile=NULL, *annotfile=NULL;
char  *SUBJECTS_DIR;

COLOR_TABLE *ctab = NULL;
int AutoCTab = 0;
char *outctabfile = NULL;
MRI_SURFACE *mris;
MRI *surfseg, *mritmp;
char *surfname = "white";

/*---------------------------------------------------------------*/
int main(int argc, char *argv[]) {
  int nargs,nv;
  char tmpstr[2000];

  nargs = handle_version_option (argc, argv, vcid, "$Name:  $");
  if (nargs && argc - nargs == 1) exit (0);
  argc -= nargs;
  cmdline = argv2cmdline(argc,argv);
  uname(&uts);
  getcwd(cwd,2000);

  Progname = argv[0] ;
  argc --;
  argv++;
  ErrorInit(NULL, NULL, NULL) ;
  DiagInit(NULL, NULL, NULL) ;
  if (argc == 0) usage_exit();
  parse_commandline(argc, argv);
  check_options();
  if (checkoptsonly) return(0);
  dump_options(stdout);



  printf("Reading surface seg %s\n",surfsegfile);
  surfseg = MRIread(surfsegfile);
  if (surfseg == NULL) exit(1);

  nv = surfseg->width * surfseg->height * surfseg->depth;
  if (surfseg->height != 1 || surfseg->depth != 1) {
    printf("Reshaping\n");
    mritmp = mri_reshape(surfseg, nv, 1, 1, surfseg->nframes);
    if (mritmp == NULL) {
      printf("ERROR: mri_reshape could not alloc\n");
      exit(1);
    }
    MRIfree(&surfseg);
    surfseg = mritmp;
  }

  SUBJECTS_DIR = getenv("SUBJECTS_DIR");
  if (SUBJECTS_DIR == NULL) {
    printf("ERROR: SUBJECTS_DIR not defined in environment\n");
    exit(1);
  }
  sprintf(tmpstr,"%s/%s/surf/%s.%s",SUBJECTS_DIR,subject,hemi,surfname);
  printf("Reading surface %s\n",tmpstr);
  mris = MRISread(tmpstr);
  if (mris==NULL) exit(1);

  if (mris->nvertices != nv) {
    printf("ERROR: dimension mismatch. Surface has %d vertices, seg has %d\n",
           mris->nvertices,nv);
    printf("Make sure the surface segmentation matches the subject and hemi\n");
    exit(1);
  }

  if(AutoCTab == 0){
    // Have to read ctab both ways (what does this mean?)
    printf("Reading ctab %s\n",ctabfile);
    ctab = CTABreadASCII(ctabfile);
    if (ctab == NULL) {
      printf("ERROR: reading %s\n",ctabfile);
      exit(1);
    }
  }
  else {
    int *segidlist, nsegs;
    segidlist = MRIsegIdListNot0(surfseg, &nsegs, 0);
    printf("AutoCTab nsegs = %d\n",nsegs);
    ctab = CTABalloc(nsegs+1);
    if(outctabfile != NULL) CTABwriteFileASCII(ctab,outctabfile);
    free(segidlist);
  }

  if (annot)
  {
    int annot, vno ;

    for (vno = 0 ; vno < mris->nvertices ; vno++)
    {
      annot = MRIgetVoxVal(surfseg, vno, 0, 0, 0) ;
      if (vno == Gdiag_no)
      {
	int index, r, g, b ;
	const char *name ;
	AnnotToRGB(annot, r, g, b) ;
	printf("annot %x = %d  %d  %d\n", annot, r, g, b) ;
	name = CTABgetAnnotationName(ctab, annot) ;
	CTABfindAnnotation(ctab, annot, &index);
	printf("v %d, annot (%x)[%d] = %s\n", vno,  annot,index,name) ;
	DiagBreak() ;
      }
      mris->vertices[vno].annotation = annot ;
    }
  }
  else
  {
    int err ;
    err = MRISseg2annot(mris, surfseg, ctab);
    if (err) exit(1);
  }

  printf("Writing annot to %s\n",annotfile);
  MRISwriteAnnotation(mris, annotfile);

  return(0);
}
/* --------------------------------------------- */
static int parse_commandline(int argc, char **argv) {
  int  nargc , nargsused;
  char **pargv, *option ;

  if (argc < 1) usage_exit();

  nargc   = argc;
  pargv = argv;
  while (nargc > 0) {

    option = pargv[0];
    if (debug) printf("%d %s\n",nargc,option);
    nargc -= 1;
    pargv += 1;

    nargsused = 0;

    if (!strcasecmp(option, "--help"))  print_help() ;
    else if (!strcasecmp(option, "--version")) print_version() ;
    else if (!strcasecmp(option, "--debug"))   debug = 1;
    else if (!strcasecmp(option, "--checkopts"))   checkoptsonly = 1;
    else if (!strcasecmp(option, "--nocheckopts")) checkoptsonly = 0;

    else if (!strcasecmp(option, "--s")) {
      if (nargc < 1) CMDargNErr(option,1);
      subject = pargv[0];
      nargsused = 1;
    } else if (!strcasecmp(option, "--seg")) {
      if (nargc < 1) CMDargNErr(option,1);
      surfsegfile = pargv[0];
      nargsused = 1;
    } else if (!strcasecmp(option, "--annot")) {
      annot = 1 ;
      nargsused = 0;
    } else if (!strcasecmp(option, "--h") || !strcasecmp(option, "--hemi")) {
      if (nargc < 1) CMDargNErr(option,1);
      hemi = pargv[0];
      nargsused = 1;
    } else if (!strcasecmp(option, "--surf")){
      if (nargc < 1) CMDargNErr(option,1);
      surfname = pargv[0];
      nargsused = 1;
    } else if (!strcasecmp(option, "--ctab")) {
      if (nargc < 1) CMDargNErr(option,1);
      ctabfile = pargv[0];
      nargsused = 1;
    } 
    else if (!strcasecmp(option, "--ctab-auto")){
      AutoCTab = 1;
      nargsused = 0;
      if(CMDnthIsArg(nargc, pargv, 0)) {
        outctabfile = pargv[0];
	nargsused = 1;
      } 
    }
    else if (!strcasecmp(option, "--o")) {
      if (nargc < 1) CMDargNErr(option,1);
      annotfile = pargv[0];
      nargsused = 1;
    } else {
      fprintf(stderr,"ERROR: Option %s unknown\n",option);
      if (CMDsingleDash(option))
        fprintf(stderr,"       Did you really mean -%s ?\n",option);
      exit(-1);
    }
    nargc -= nargsused;
    pargv += nargsused;
  }
  return(0);
}
/* ------------------------------------------------------ */
static void usage_exit(void) {
  print_usage() ;
  exit(1) ;
}
/* --------------------------------------------- */
static void print_usage(void) {
  printf("USAGE: %s \n",Progname) ;
  printf("\n");
  printf("   --seg  surfseg    : volume-encoded surface segmentation \n");
  printf("   --ctab colortable : color table (like FreeSurferColorLUT.txt)\n");
  printf("   --ctab-auto <outcolortable> : create a random color table, optionally save ctab\n");
  printf("   --s subject   : subject name\n");
  printf("   --h hemi      : surface hemifield\n");
  printf("   --o annot     : output annotation file\n");
  printf("\n");
  printf("   --surf surfname : default is white\n");
  printf("   --debug     turn on debugging\n");
  printf("   --checkopts don't run anything, just check options and exit\n");
  printf("   --help      print out information on how to use this program\n");
  printf("   --version   print out version and exit\n");
  printf("\n");
  printf("%s\n", vcid) ;
  printf("\n");
}
/* --------------------------------------------- */
static void print_help(void) {
  print_usage() ;
  printf("\n");
  printf("Converts a surfaced-based segmentation into a custom annotation file.\n");
  printf("\n");
  printf("--seg surfseg\n");
  printf("\n");
  printf("Surface segmentation file. This could be as simple as a binarized\n");
  printf("functional map. The values are whole numbers indicating the index into\n");
  printf("the color table. This file is similar to the volume-based aseg.  The\n");
  printf("hard part to getting a custom annotation is in creating this file and\n");
  printf("corresponding color table.\n");
  printf("\n");
  printf("--ctab colortable\n");
  printf("\n");
  printf("Color table used to map segmentation index to name and color. This is\n");
  printf("something that can be created by the user to create custom\n");
  printf("annotations. This color table is then imbedded in the annotation\n");
  printf("file. Be default, it will look for this file in $FREESURFER_HOME.  If\n");
  printf("this is not where your color table is, then add a './' in front of the\n");
  printf("name. The format should be the same as in\n");
  printf("$FREESURFER_HOME/FreeSurferColorsLUT.txt.\n");
  printf("\n");
  printf("--s subject\n");
  printf("--h hemi\n");
  printf("\n");
  printf("Subject and hemisphere. Used to load in the surface upon which the \n");
  printf("annotation is created.\n");
  printf("\n");
  printf("--o annot\n");
  printf("\n");
  printf("Output annotation file. By default, it will be stored in the subject's \n");
  printf("label directory. If you do not want it there, then supply some path\n");
  printf("in front of it (eg, './'). This is a file like lh.aparc.annot. \n");
  printf("\n");
  printf("EXAMPLE:\n");
  printf("\n");
  printf("  mris_seg2annot --seg lh.FL_002.sig.th8.mgh \\\n");
  printf("    --s FL_002 --h lh --ctab ./MyColorLUT.txt \\\n");
  printf("    --o ./lh.myaparc.annot\n");
  printf("\n");
  printf("lh.myaparc.annot can then be loaded into tksurfer as with any other \n");
  printf("parcellation/annotation.\n");
  printf("\n");
  exit(1) ;
}
/* --------------------------------------------- */
static void print_version(void) {
  printf("%s\n", vcid) ;
  exit(1) ;
}
/* --------------------------------------------- */
static void check_options(void) {
  if (subject == NULL) {
    printf("ERROR: subject not specified\n");
    exit(1);
  }
  if (hemi == NULL) {
    printf("ERROR: hemi not specified\n");
    exit(1);
  }
  if(ctabfile == NULL && ! AutoCTab) {
    printf("ERROR: ctab not specified\n");
    exit(1);
  }
  if (annotfile == NULL) {
    printf("ERROR: output not specified\n");
    exit(1);
  }
  if (surfsegfile == NULL) {
    printf("ERROR: surfseg not specified\n");
    exit(1);
  }
  return;
}

/* --------------------------------------------- */
static void dump_options(FILE *fp) {
  fprintf(fp,"\n");
  fprintf(fp,"%s\n",vcid);
  fprintf(fp,"cwd %s\n",cwd);
  fprintf(fp,"cmdline %s\n",cmdline);
  fprintf(fp,"sysname  %s\n",uts.sysname);
  fprintf(fp,"hostname %s\n",uts.nodename);
  fprintf(fp,"machine  %s\n",uts.machine);
  fprintf(fp,"user     %s\n",VERuser());
  fprintf(fp,"subject   %s\n",subject);
  fprintf(fp,"hemi      %s\n",hemi);
  fprintf(fp,"surfseg   %s\n",surfsegfile);
  fprintf(fp,"ctab      %s\n",ctabfile);
  fprintf(fp,"annotfile %s\n",annotfile);

  return;
}
