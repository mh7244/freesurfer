/**
 * @file  annotation.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR 
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:09 $
 *    $Revision: 1.18 $
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


#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <vector>
#include <string>

#ifdef ANNOTATION_SRC
char *annotation_table_file = NULL;
#else
extern char *annotation_table_file;
#endif

int   	read_annotation_table(void) ;
int   	read_named_annotation_table(char *fname) ;
const char*	index_to_name(int index);
const char*	annotation_to_name(int annotation, int *pindex) ;
int   	annotation_to_index(int annotation) ;
int   	print_annotation_table(FILE *fp);
int   	print_annotation_colortable(FILE *fp);
int   	index_to_annotation(int index) ;
LABEL*	annotation2label(int annotid, MRIS *Surf);
int 	set_atable_from_ctable(COLOR_TABLE *pct);
int  	MRISdivideAnnotation(MRI_SURFACE *mris, int *nunits) ;
int  	MRISdivideAnnotationUnit(MRI_SURFACE *mris, int annot, int nunits) ;
int  	MRISmergeAnnotations(MRIS *mris, int nparcs, std::vector<std::string> parcnames, const char *newparcname);
MRI*	MRISannot2seg(MRIS *surf, int base);
MRI*	MRISannot2border(MRIS *surf);
int 	MRISaparc2lobes(MRIS *surf, int a_lobeDivisionType);
int 	MRISfbirnAnnot(MRIS *surf);
double*	MRISannotDice(MRIS *surf1, MRIS *surf2, int *nsegs, int **segidlist);

#endif
