/**
 * @file  mrisutils.h
 * @brief more surface processing utils
 *
 */
/*
 * Original Authors: Segonne and Greve 
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2015/07/24 16:13:39 $
 *    $Revision: 1.24 $
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

#include "mri.h"
#include "mrisurf.h"

#ifndef SUTILS_INCLUDED
#define SUTILS_INCLUDED

// The LABEL2SURF structure is used to facilitate mapping label voxels
// on the surface. See functions with L2S as the prefix.
typedef struct {
  MRIS **surfs;  // list of surfaces (eg, left and right)
  int nsurfs;    // number of surfaces in list
  MRI *mri_template; // volume template for voxel to add
  double dmax;   // max allowable distance in mm bet voxel center and surface vertex
  LTA *vol2surf; // reg bet vol and surf, direction unimportant, set to null to use header reg
  LABEL **labels;  // list of labels, one for each surface
  MHT **hashes;  // hash tables, one for each surface
  float hashres; // eg, 16
  MRI **masks;   // binary masks of the label for each surface
  MRI *volmask;   // binary masks of the non-surfaace label points
  LABEL *vollabel; // label of non-surfaace points
  int nhopsmax; // number of nearest neighbor rings to expand the label
  int DilateWithinVoxel; // require neighboring vertices to be within voxel to include in label
  int DilateContiguous; // require neighboring vertices to be continguous with the center vertex
  int debug; // set to 1 to print out a bunch of stuff
  MATRIX *volcrs2surfxyz; // precomputed matrix converts vol CRS to surface tkRAS
  MATRIX *surfxyz2volcrs; // inverse
} LABEL2SURF;

// Structure to help create an average surface
typedef struct {
  char **subjectlist;
  int nsubjects;
  MRIS *targsurfreg; // an actual surface to use as the target registration
  int  icoorder; // only use if targsurfreg not spec
  char *targsubject; // only use if targsurfreg and icoorder not spec
  char *hemi; // lh or rh
  char *surfname; // eg, white
  char *surfregname; // eg, sphere.reg
  char *xform_name; // eg, talairach.xfm
  int ReverseMapFlag; // map unmapped vertices in the source, generally 1
  int UseHash; // use hash table for speed, generally 1
} AVERAGE_SURFACE_PARAMS;
MRIS *MakeAverageSurf(AVERAGE_SURFACE_PARAMS *asp);
AVERAGE_SURFACE_PARAMS *MRISaverageSurfaceParamAlloc(int nsubjects);
int MRISaverageSurfaceParamFree(AVERAGE_SURFACE_PARAMS **pasp);

void MRISsmoothSurface(MRI_SURFACE *mris,int niter,float step);
void MRISsmoothSurface2(MRI_SURFACE *mris,int niter,float step,int avrg);
void MRIScenterCOG(MRI_SURFACE *mris);
void MRIScenterCOG2(MRI_SURFACE *mris,double *xCOG,double *yCOG,double *zCOG);
MRI* MRISpeelVolume(MRIS *mris,
                    MRI *mri_src,
                    MRI *mri_dst,
                    int type,
                    unsigned char val,
                    unsigned long *NbVoxels);
MRIS *MRISmatchSurfaceToLabel(MRIS *mris,
                              MRI *mri_seg,
                              int label,
                              MRI_REGION *mri_region,
                              INTEGRATION_PARMS *integration_parms,
                              int connectivity);
MRIS *MRISloadSurfSubject(char *subj, char *hemi, char *surfid,
                          char *SUBJECTS_DIR);
int MRISfdr2vwth(MRIS *surf, double fdr, int signid,
                 int log10flag, int maskflag, double *vwth);

int    MRISfwhm2niters(double fwhm, MRIS *surf);
double MRISniters2fwhm(int niters, MRIS *surf);
int MRISfwhm2nitersSubj(double fwhm,char *subject,char *hemi,char *surfname);
double MRISfwhmFromAR1(MRIS *surf, double ar1);
int MRISseg2annot(MRIS *mris, MRI *surfseg, COLOR_TABLE *ctab);
MRI *MRISannotIndex2Seg(MRIS *mris);
double MRISvolumeInSurf(MRIS *mris);
MRI *MRISvolumeTH3(MRIS *w, MRIS *p, MRI *vol, MRI *mask, double *totvol);

LABEL *MRIScortexLabel(MRI_SURFACE *mris, MRI *mri_aseg, int min_vertices);
int MRISripZeros(MRIS *surf, MRI *mri);
int MRISsphericalCoords(MRIS *surf);
int MRISfindPath ( int *vert_vno, int num_vno, int max_path_length,
                   int *path, int *path_length, MRIS *mris );

double *
MRISsampleProfile(MRI_SURFACE *mris, MRI *mri, double *profile, int nsamples, int wm_samples, int outside_samples,
		  double wx,  double wy,  double wz,
		  double l4x, double l4y, double l4z,
		  double px,  double py,  double pz) ;
MRI_SP *MRISmakeTemplate(int nsubjects, char **subjlist, int nhemis, char **hemilist, char *surfregname);
int MRISsetPialUnknownToWhite(const MRIS *white, MRIS *pial);
int MRISripUnknown(MRIS *surf);
int MRISshiftCRAS(MRIS *mris, int shift);
int MRISscanner2Tkr(MRIS *mris);
int MRIStkr2Scanner(MRIS *mris);
int ComputeMRISvolumeTH3(char *subject, char *hemi, int DoMask, char *outfile);

LABEL2SURF *L2Salloc(int nsurfs, const char *subject);
int L2Sinit(LABEL2SURF *l2s);
int L2SaddPoint(LABEL2SURF *l2s, double col, double row, double slice, int PointType, int Operation);
int L2SaddVoxel(LABEL2SURF *l2s, double col, double row, double slice, int nsegs, int Operation);
int L2Sfree(LABEL2SURF **pl2s);
int L2SimportLabel(LABEL2SURF *l2s, LABEL *label, int surfno);
int L2Stest(const char *subject);

int MRISeulerNoSeg(MRI_SURFACE *mris, MRI *surfseg, int segno, int *pnvertices, int *pnfaces, int *pnedges, int *pv0);

#endif
