/******************************************************************************
 LongCoordinates.h



 Copyright © 1991, 1994 Symantec Corporation. All rights reserved.

 TCL 1.1.3 Changes
 [
 - use #pragma once
 - bracket declarations of these utility functions with extern "C".
 ]
 ******************************************************************************/

#ifndef __TCL_LONGCOORDINATES__
#define __TCL_LONGCOORDINATES__

 //#include <Types.h>
#include "macos_compatibility.h"

typedef struct LongPt
{
    long v, h;
} LongPt, *LongPtPtr, **LongPtHndl;

typedef struct LongRect
{
    long top, left, bottom, right;
} LongRect, *LongRectPtr, **LongRectHndl;

// access top left and bottom right LongPt of a LongRect
#define topLeftL(r) (((LongPt *) &(r))[0])
#define botRightL(r) (((LongPt *) &(r))[1])

/*
* LongPt utilities
*
*/

void QDToLongPt(Point srcPt, LongPt* destPt);
void LongToQDPt(LongPt* srcPt, Point* destPt);
void SetLongPt(LongPt* pt, long h, long v);
void AddLongPt(LongPt* srcPt, LongPt* destPt);
void SubLongPt(LongPt* srcPt, LongPt* destPt);
Boolean EqualLongPt(LongPt* pt1, LongPt* pt2);
Boolean PtInQDSpace(LongPt* pt);


/*
* LongRect utilities
*
*/


void QDToLongRect(Rect* srcRect, LongRect* destRect);
void LongToQDRect(LongRect* srcRect, Rect* destRect);
void SetLongRect(LongRect* r, long left, long top, long right, long bottom);
void OffsetLongRect(LongRect* r, long dh, long dv);
void InsetLongRect(LongRect* r, long dh, long dv);
Boolean SectLongRect(LongRect* src1, LongRect* src2, LongRect* destRect);
void UnionLongRect(LongRect* src1, LongRect* src2, LongRect* destRect);
Boolean PtInLongRect(LongPt* pt, LongRect* r);
void Pt2LongRect(LongPt* pt1, LongPt* pt2, LongRect* r);
Boolean EqualLongRect(LongRect* r1, LongRect* r2);
Boolean EmptyLongRect(LongRect* r);
Boolean RectInQDSpace(LongRect* r);


#endif // #ifndef __TCL_LONGCOORDINATES__
