/**
 * @file  surfgrad.c
 * @brief Utilities to compute gradients on the surface
 *
 */
/*
 * Original Author: Doug Greve
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2015/03/31 20:27:34 $
 *    $Revision: 1.90 $
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


#ifndef SURFGRAD_H
#define SURFGRAD_H

#ifdef X
#undef X
#endif

#include "mrisurf.h"

typedef struct {
  double Din, Dout; // abs distance (mm) to project in and out
  double M, Q0; // BBR slope and offset
  int interp; // SAMPLE_TRILINEAR (1) or SAMPLE_CUBIC (3)
  MRI *mri;
  MRIS *surf;
  DMATRIX *sras2vox; // 4x4 maps surface RAS to mri CRS 
  DMATRIX *sras2vox3x3; // 3x3 version good for computing gradients
} 
BBRPARAMS;

typedef struct {
  int faceno;
  DMATRIX *pc, *pin, *pout; // xyz of center, inward point and outward point
  DMATRIX *vin, *vout; // col,row,slice of inward point and outward point
  double Iin,Iout; // intensity of inward point and outward point
  double Q,cost; // Q=relative contrast, cost=bbr cost
  DMATRIX *gradPin[3], *gradVin[3], *gradIin[3], *gradPout[3], *gradVout[3], *gradIout[3];
  DMATRIX *gradInterpIn[3], *gradInterpOut[3];
  DMATRIX *gradQ[3], *gradCost[3];
} 
BBRFACE;


#ifdef _SURFGRAD_SRC
int MRISfaceNormalFace_AddDeltaVertex = -1;
long double MRISfaceNormalFace_AddDelta[3]={0,0,0};
int MRISbbrCostFree=0;
#else
extern int MRISfaceNormalFace_AddDeltaVertex;
extern long double MRISfaceNormalFace_AddDelta[3];
extern int MRISbbrCostFree;
#endif

int MRISfaceNormalGradFace(MRIS *surf, int faceno);
int MRISfaceNormalGradTest(MRIS *surf, char *surfpath, double delta);
int MRISfaceNormalFace(MRIS *surf, int faceno, DMATRIX **pc, double *pcL);
double MRISfaceNormalGradFaceTest(MRIS *surf, int faceno, int wrtvtxno, long double delta, int verbose);


double MRISedgeAngleCostEdgeVertex(MRIS *surf, int edgeno, int wrtvtxno, DMATRIX **pgrad);
int MRISedgeGradDotEdgeVertex(MRIS *surf, int edgeno, int wrtvtxno);
int MRISedgeGradDot(MRIS *surf);
double MRISedgeGradDotEdgeVertexTest(MRIS *surf, int edgeno, int wrtvtxno, long double delta, int verbose);
int MRISfaceNormalGrad(MRIS *surf, int NormOnly);
int MRISedgeAngleCostEdgeVertexTest(MRIS *surf, int edgeno, int wrtvtxno, long double delta);
int MRISedgePrint(MRIS *surf, int edgeno, FILE *fp);
int MRISedgeMetricEdge(MRIS *surf, int edgeno);
int MRISfacePrint(MRIS *surf, int faceno, FILE *fp);

BBRFACE *BBRFaceAlloc(void);
int BBRFaceFree(BBRFACE **pbbrf);
BBRFACE *BBRCostFace(int faceno, int wrtvtxno, BBRPARAMS *bbrpar, BBRFACE *bbrf);
int BBRFacePrint(FILE *fp, BBRFACE *bbrf, BBRPARAMS *bbrpar);
BBRFACE *BBRFaceDiff(const BBRFACE *bbrf1, const BBRFACE *bbrf2, const double delta, BBRFACE *bbrfout);
double TestBBRCostFace(BBRPARAMS *bbrpar, int faceno, int wrtvtxno, double delta, int verbose);
int BBRPARsras2vox(BBRPARAMS *bbrpar);
long double MRISbbrCost(BBRPARAMS *bbrpar, DMATRIX *gradCost);
double MRISedgeCost(MRIS *surf, DMATRIX *gradCost);

#endif
