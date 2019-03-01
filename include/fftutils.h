/**
 * @file  fftutils.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR 
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:09 $
 *    $Revision: 1.3 $
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



#ifndef FFTUTILS_INC
#define FFTUTILS_INC


void CFFTforward(float* re, float* im, int length);
void CFFTbackward(float* re, float* im, int length);

void RFFTforward(float* data,int length, float* re, float* im );


void RFFT( float* data, int data_length, int length, int direction );
void FFTdebugAssert(int b, const char *string);
int FFTisPowerOf2( int x ); 
int FFTpow2( int exponent );
float FFTdist(int x,int y,int z,int len);
int FFTlog2( int x );
void FFTswitch_with_z (float *** vect, int dimension, int is_y);
void FFTmodarg_to_reim(float *** re_mod, float *** im_arg, int l);
float ***FFTinv_quarter(float *** vect, int dimension);
void FFTreim_to_modarg (float *** re_mod, float *** im_arg, int l);

#endif
