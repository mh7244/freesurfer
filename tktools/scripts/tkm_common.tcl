##
## tkm_common.tcl
##
## CVS Revision Info:
##    $Author: nicks $
##    $Date: 2011/03/01 01:41:22 $
##    $Revision: 1.5 $
##
## Copyright (C) 2003-2011, CorTechs Labs, Inc. (La Jolla, CA) and
## The General Hospital Corporation (Boston, MA).
##
## Terms and conditions for use, reproduction, distribution and contribution
## are found in the 'FreeSurfer/CorTechs Software License Agreement' contained
## in the file 'license.cortechs.txt' found in the FreeSurfer distribution,
## and here:
##
## https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferCorTechsLicense
##
## Reporting: freesurfer@nmr.mgh.harvard.edu
##

proc dputs { isString } {

    if { [catch {DebugPrint $isString}] != 0 } {
	puts $isString
    }
}

