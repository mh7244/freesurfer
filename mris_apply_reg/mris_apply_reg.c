/**
 * @file  mris_apply_reg.c
 * @brief Applies multiple surface registrations
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: Douglas N. Greve
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2016/12/06 19:40:48 $
 *    $Revision: 1.9 $
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



/*!
\file mris_apply_reg.c
\brief Example c file that can be used as a template.
\author Douglas Greve

*/


// $Id: mris_apply_reg.c,v 1.9 2016/12/06 19:40:48 greve Exp $

/*
  BEGINHELP

  ENDHELP
*/

/*
  BEGINUSAGE

  ENDUSAGE
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

#include "macros.h"
#include "utils.h"
#include "fio.h"
#include "version.h"
#include "cmdargs.h"
#include "error.h"
#include "diag.h"
#include "mrisurf.h"
#include "resample.h"
#include "pdf.h"
#include "icosahedron.h"
#include "mrisutils.h"
#include "mri2.h"

static int  parse_commandline(int argc, char **argv);
static void check_options(void);
static void print_usage(void) ;
static void usage_exit(void);
static void print_help(void) ;
static void print_version(void) ;
static void dump_options(FILE *fp);
void usage_message(FILE *stream);
void usage(FILE *stream);
int main(int argc, char *argv[]) ;

static char vcid[] = "$Id: mris_apply_reg.c,v 1.9 2016/12/06 19:40:48 greve Exp $";
const char *Progname = NULL;
char *cmdline, cwd[2000];
int debug=0;
int checkoptsonly=0;
struct utsname uts;

char *SrcValFile=NULL;
char *TrgValFile=NULL;
char *SurfRegFile[100];
int ReverseMapFlag = 1;
int DoJac = 0;
int UseHash = 1;
int nsurfs = 0;
int DoSynthRand = 0;
int DoSynthOnes = 0;
int SynthSeed = -1;
char *AnnotFile = NULL;
char *LabelFile = NULL;
char *SurfXYZFile = NULL;
LABEL *MRISmask2Label(MRIS *surf, MRI *mask, int frame, double thresh);

/*---------------------------------------------------------------*/
int main(int argc, char *argv[]) {
  int nargs,n,err;
  MRIS *SurfReg[100],*SurfSrc;
  MRI *SrcVal, *TrgVal;
  char *base;
  COLOR_TABLE *ctab=NULL;

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

  // Load in surface registrations
  for(n=0; n<nsurfs;n++){
    printf("%d Loading %s\n",n+1,SurfRegFile[n]);
    base = fio_basename(SurfRegFile[n],".tri");
    if(strcmp(base,"ic7")==0){
      // Have to do it this way to rescale. Need to find a better more robust way.
      printf("   reading as ico 7, rescaling radius to 100\n");
      SurfReg[n] = ReadIcoByOrder(7, 100);
    }
    else
      SurfReg[n] = MRISread(SurfRegFile[n]);
    free(base);
    if(SurfReg[n]==NULL) exit(1);
  }

  // Load in source data
  SrcVal = NULL;
  if(DoSynthRand) {
    if (SynthSeed < 0) SynthSeed = PDFtodSeed();
    printf("INFO: synthesizing, seed = %d\n",SynthSeed);
    srand48(SynthSeed);
    MRIrandn(SrcVal->width, SrcVal->height, SrcVal->depth,
             SrcVal->nframes,0, 1, SrcVal);
  }
  else if(DoSynthOnes != 0) {
    printf("INFO: filling input with all 1s\n");
    MRIconst(SrcVal->width, SrcVal->height, SrcVal->depth,
             SrcVal->nframes, 1, SrcVal);
  }
  else if(AnnotFile) {
    printf("Loading annotation %s\n",AnnotFile);
    err = MRISreadAnnotation(SurfReg[0], AnnotFile);
    if(err) exit(1);
    SrcVal = MRISannotIndex2Seg(SurfReg[0]);
    ctab = CTABdeepCopy(SurfReg[0]->ct);
  }
  else if(SurfXYZFile) {
    printf("Loading surface xyz %s\n",SurfXYZFile);
    SurfSrc = MRISread(SurfXYZFile);
    if(SurfSrc==NULL)  exit(1);
    SrcVal = MRIcopyMRIS(NULL, SurfSrc, 2, "z"); // start at z to autoalloc
    MRIcopyMRIS(SrcVal, SurfSrc, 0, "x");
    MRIcopyMRIS(SrcVal, SurfSrc, 1, "y");
  }
  else if(LabelFile) {
    LABEL *srclabel;
    printf("Loading label %s\n",LabelFile);
    srclabel = LabelRead(NULL, LabelFile);
    if(srclabel == NULL) exit(1);
    SrcVal = MRISlabel2Mask(SurfReg[0],srclabel,NULL);
    printf("   %d points in input label\n",srclabel->n_points);
    LabelFree(&srclabel);
  }
  else {
    printf("Loading %s\n",SrcValFile);
    SrcVal = MRIread(SrcValFile);
    if(SrcVal==NULL) exit(1);
    MRI *tmpmri;
    tmpmri = MRIreshape1d(SrcVal,NULL);
    if(tmpmri == NULL) exit(1);
    MRIfree(&SrcVal);
    SrcVal = tmpmri;
    if(SrcVal->type != MRI_FLOAT) {
      printf("Converting source to float\n");
      tmpmri = MRISeqchangeType(SrcVal,MRI_FLOAT,0,0,0);
      if (tmpmri == NULL) {
        printf("ERROR: could change type\n");
        exit(1);
      }
      MRIfree(&SrcVal);
      SrcVal = tmpmri;
    }
  }

  // Apply registration to source
  TrgVal = MRISapplyReg(SrcVal, SurfReg, nsurfs, ReverseMapFlag, DoJac, UseHash);
  if(TrgVal == NULL) exit(1);

  // Save output
  if(AnnotFile){
    printf("Converting to target annot\n");
    err = MRISseg2annot(SurfReg[nsurfs-1],TrgVal,ctab);
    if(err) exit(1);
    printf("Writing %s\n",TrgValFile);
    MRISwriteAnnotation(SurfReg[nsurfs-1], TrgValFile);
  } 
  else if(LabelFile){
    LABEL *label;
    label = MRISmask2Label(SurfReg[nsurfs-1], TrgVal, 0, 10e-5);
    printf("   %d points in output label\n",label->n_points);
    err = LabelWrite(label,TrgValFile);
    if(err){
      printf("ERROR: writing label file %s\n",TrgValFile);
      exit(1);
    }
    LabelFree(&label);
  }
  else if(SurfXYZFile){
    printf("Writing surface to %s\n",TrgValFile);
    MRIScopyMRI(SurfReg[nsurfs-1],TrgVal,0,"x");
    MRIScopyMRI(SurfReg[nsurfs-1],TrgVal,1,"y");
    MRIScopyMRI(SurfReg[nsurfs-1],TrgVal,2,"z");
    MRISwrite(SurfReg[nsurfs-1], TrgValFile);
  }
  else{
    printf("Writing %s\n",TrgValFile);
    MRIwrite(TrgVal,TrgValFile);
  }
  
  printf("mris_apply_reg done\n");
  return 0;
}
/*------------------------------------------------------------*/
/*------------------------------------------------------------*/
/*------------------------------------------------------------*/
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
    else if (!strcasecmp(option, "--norev")) ReverseMapFlag = 0;
    else if (!strcasecmp(option, "--no-rev")) ReverseMapFlag = 0;
    else if (!strcasecmp(option, "--nnf")) ReverseMapFlag = 0;
    else if (!strcasecmp(option, "--nnfr")) ReverseMapFlag = 1;
    else if (!strcasecmp(option, "--no-hash")) UseHash = 0;
    else if (!strcasecmp(option, "--jac")) DoJac = 1;
    else if (!strcasecmp(option, "--no-jac")) DoJac = 0;
    else if (!strcasecmp(option, "--randn")) DoSynthRand = 1;
    else if (!strcasecmp(option, "--ones")) DoSynthOnes = 1;

    else if (!strcasecmp(option, "--src") || !strcasecmp(option, "--sval")) {
      if (nargc < 1) CMDargNErr(option,1);
      SrcValFile = pargv[0];
      if(!fio_FileExistsReadable(SrcValFile)){
	printf("ERROR: %s does not exist or is not readable by you\n",SrcValFile);
	exit(1);
      }
      nargsused = 1;
    } 
    else if (!strcasecmp(option, "--sval-annot") || !strcasecmp(option, "--src-annot")){
      if (nargc < 1) CMDargNErr(option,1);
      AnnotFile = pargv[0];
      if(!fio_FileExistsReadable(AnnotFile)){
	printf("ERROR: %s does not exist or is not readable by you\n",AnnotFile);
	exit(1);
      }
      DoJac = 0;
      ReverseMapFlag = 0;
      nargsused = 1;
    } 
    else if (!strcasecmp(option, "--src-xyz")){
      if (nargc < 1) CMDargNErr(option,1);
      SurfXYZFile = pargv[0];
      if(!fio_FileExistsReadable(SurfXYZFile)){
	printf("ERROR: %s does not exist or is not readable by you\n",SurfXYZFile);
	exit(1);
      }
      nargsused = 1;
    } 
    else if (!strcasecmp(option, "--sval-label") || !strcasecmp(option, "--src-label")){
      if (nargc < 1) CMDargNErr(option,1);
      LabelFile = pargv[0];
      if(!fio_FileExistsReadable(LabelFile)){
	printf("ERROR: %s does not exist or is not readable by you\n",LabelFile);
	exit(1);
      }
      DoJac = 0;
      ReverseMapFlag = 0;
      nargsused = 1;
    } 
    else if (!strcasecmp(option, "--trg") || !strcasecmp(option, "--tval") 
	     || !strcasecmp(option, "--o")) {
      if (nargc < 1) CMDargNErr(option,1);
      TrgValFile = pargv[0];
      nargsused = 1;
    } 
    else if (!strcasecmp(option, "--streg")) {
      if (nargc < 2) CMDargNErr(option,2);
      SurfRegFile[nsurfs] = pargv[0];
      nsurfs++;
      SurfRegFile[nsurfs] = pargv[1];
      nsurfs++;
      nargsused = 2;
    } 
    else if (!strcasecmp(option, "--lta")) {
      if(nargc < 3) CMDargNErr(option,3);
      printf("Reading in %s\n",pargv[0]);
      MRIS *ltasurf = MRISread(pargv[0]);
      if(ltasurf==NULL) exit(1);
      printf("Reading in %s\n",pargv[1]);
      LTA *lta = LTAread(pargv[1]);
      if(lta==NULL) exit(1);
      int err = MRISltaMultiply(ltasurf, lta);
      if(err) exit(1);
      printf("Writing surf to %s\n",pargv[2]);
      err = MRISwrite(ltasurf,pargv[2]);
      if(err) exit(1);
      nargsused = 3;
      printf("mris_apply_reg done\n");
      exit(0);
    } 

    else {
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
/*--------------------------------------------------------------*/
static void usage_exit(void) {
  print_usage() ;
  exit(1) ;
}
/*--------------------------------------------------------------*/
static void print_usage(void) {
  printf("USAGE: %s \n",Progname) ;
  printf("\n");
  printf(" Input specifcation (pick one):\n");
  printf("   --src srcvalfile : source values (surface overlay)\n");
  printf("   --src-annot srcannotfile : source annotation (implies --no-rev)\n");
  printf("   --src-label labelfile : source label (implies --no-rev)\n");
  printf("   --src-xyz surfacefile : use xyz coords from given surface as input\n");
  printf(" Output specifcation (format depends on type of input):\n");
  printf("   --trg trgvalfile : (Can also use --o)\n");
  printf(" Registration specifcation (srcreg1->trgreg1->srcreg2->trgreg2...):\n");
  printf(" Need at least one --streg pair but can have any number\n");
  printf("   --streg srcreg1 trgreg1 : source and target reg files\n");
  printf("   --streg srcreg2 trgreg2 : more source and target reg files ...\n");
  printf("\n");
  printf("   --jac : use jacobian correction\n");
  printf("   --no-rev : do not do reverse mapping\n");
  printf("   --randn : replace input with WGN\n");
  printf("   --ones  : replace input with ones\n");
  printf("\n");
  printf("   --lta source-surf ltafile output-surf : apply LTA transform\n");
  printf("    other options do not apply to --lta\n");
  printf("\n");
  printf("   --debug     turn on debugging\n");
  printf("   --checkopts don't run anything, just check options and exit\n");
  printf("   --help      print out information on how to use this program\n");
  printf("   --version   print out version and exit\n");
  printf("\n");
  printf("%s\n", vcid) ;
  printf("\n");
}
/*--------------------------------------------------------------*/
static void print_help(void) {
  print_usage() ;
  usage(stdout);
  exit(1) ;
}
/*--------------------------------------------------------------*/
static void print_version(void) {
  printf("%s\n", vcid) ;
  exit(1) ;
}
/*--------------------------------------------------------------*/
static void check_options(void) {
  int n;
  if(SrcValFile == NULL && AnnotFile == NULL && LabelFile == NULL && SurfXYZFile == NULL){
    printf("ERROR: need to specify source value file\n");
    exit(1);
  }
  if(SrcValFile && AnnotFile){
    printf("ERROR: cannot spec both --src and --src-annot\n");
    exit(1);
  }
  if(SrcValFile && LabelFile){
    printf("ERROR: cannot spec both --src and --src-label\n");
    exit(1);
  }
  if(AnnotFile && LabelFile){
    printf("ERROR: cannot spec both --src-annot and --src-label\n");
    exit(1);
  }
  if(AnnotFile && SurfXYZFile){
    printf("ERROR: cannot spec both --src-annot and --src-xyz\n");
    exit(1);
  }
  if(SrcValFile && SurfXYZFile){
    printf("ERROR: cannot spec both --src and --src-xyz\n");
    exit(1);
  }
  if(TrgValFile == NULL){
    printf("ERROR: need to specify target value file\n");
    exit(1);
  }
  if(nsurfs == 0){
    printf("ERROR: must specify at least one source:target registration pair\n");
    exit(1);
  }
  for(n=0; n<nsurfs;n++){
    if(!fio_FileExistsReadable(SurfRegFile[n])){
      printf("ERROR: %s does not exist or is not readable by you\n",SurfRegFile[n]);
      exit(1);
    }
  }
  return;
}
/*--------------------------------------------------------------*/
static void dump_options(FILE *fp) {
  fprintf(fp,"\n");
  fprintf(fp,"%s\n",vcid);
  fprintf(fp,"cwd %s\n",cwd);
  fprintf(fp,"cmdline %s\n",cmdline);
  fprintf(fp,"sysname  %s\n",uts.sysname);
  fprintf(fp,"hostname %s\n",uts.nodename);
  fprintf(fp,"machine  %s\n",uts.machine);
  fprintf(fp,"user     %s\n",VERuser());
  fprintf(fp,"srcvalfile  %s\n",SrcValFile);
  fprintf(fp,"trgvalfile  %s\n",TrgValFile);
  fprintf(fp,"nsurfs  %d\n",nsurfs);
  fprintf(fp,"jac  %d\n",DoJac);
  fprintf(fp,"revmap  %d\n",ReverseMapFlag);
  return;
}

#include "mris_apply_reg.help.xml.h"
void usage(FILE *stream)
{
  outputHelpXml(mris_apply_reg_help_xml,mris_apply_reg_help_xml_len);
} /* end usage() */

/* EOF */

/*!
  \fn LABEL *MRISmask2Label(MRIS *surf, MRI *mask, int frame, double thresh)
  \brief Converts a surface overlay (mask) to a surface label. mask can be
  non-binary.  Values over thresh are used. The mask value is set to
  be the label stat.
 */
LABEL *MRISmask2Label(MRIS *surf, MRI *mask, int frame, double thresh)
{
  LABEL *label;
  int c, r, s, n, vtxno;
  double val;
  VERTEX *v;

  // Count number of points in the label
  n = 0;
  for(s=0; s < mask->depth; s++){
    for(r=0; r < mask->height; r++){
      for(c=0; c < mask->width; c++){
	val = MRIgetVoxVal(mask,c,r,s,frame);
	if(val < thresh) continue;
	n++;
      }
    }
  }

  // Alloc
  label = LabelAlloc(n,surf->subject_name,NULL);
  label->n_points = n;

  // Asign values
  n = 0;
  vtxno = -1;
  for(s=0; s < mask->depth; s++){
    for(r=0; r < mask->height; r++){
      for(c=0; c < mask->width; c++){
	vtxno++;
	val = MRIgetVoxVal(mask,c,r,s,frame);
	if(val < thresh) continue;
	v = &surf->vertices[vtxno];
	label->lv[n].vno = vtxno;
	label->lv[n].x = v->x;
	label->lv[n].y = v->y;
	label->lv[n].z = v->z;
	label->lv[n].stat = val;
	n++;
      }
    }
  }

  return(label);
}
