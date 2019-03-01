/**
 * @file  loop.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR 
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:11 $
 *    $Revision: 1.4 $
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


#ifndef TOPOLOGY_LOOP_H
#define TOPOLOGY_LOOP_H

#include "globals.h"

class Loop
{
private:
  void _ReAlloc(int maxpts=-1);
public:
  int npoints,maxpoints;
  int *points;
public:
  //constructor/destructor
  Loop(void);
  Loop(int maxpts);
  ~Loop(void);

  void Alloc(int maxpts);
  void AddPoint(int pt);
  void Print() const;
  int End();
  void Pop();
  int Replace(int pt, int new_pt);
  int operator[](int n)
  {
    ASSERT((n >= 0 ) && (n < npoints));
    return points[n];
  }
  const Loop & operator=(const Loop& loop);
};

#endif
