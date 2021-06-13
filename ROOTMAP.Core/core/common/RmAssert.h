#ifndef RmAssert_H
#define RmAssert_H
/////////////////////////////////////////////////////////////////////////////
// Name:        RmAssert.h
// Purpose:     
// Created:     24/04/2006
// Author:      RvH
// $Date: 2009-07-19 19:34:59 +0800 (Sun, 19 Jul 2009) $
// $Revision: 72 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "wx/defs.h"
#include "wx/debug.h"

#define RmAssert(cond,msg) wxASSERT_MSG((cond),(msg))


#endif // #ifndef RmAssert_H
