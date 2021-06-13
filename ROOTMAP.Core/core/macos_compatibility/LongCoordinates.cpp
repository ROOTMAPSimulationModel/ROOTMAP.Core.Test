/******************************************************************************
 LongCoordinates.c

        Utility functions to convert QuickDraw and their long coordinate
        counterparts back and forth, and long coordinate versions
        of the most frequently used point and rectangle utilities.

    Copyright © 1991, 1994 Symantec Corporation. All rights reserved.


    TCL 1.1.3 Changes
    [
        - Dereference pointer argument in RectInQDSpace
    ]
******************************************************************************/

#ifdef TCL_PCH
#include <TCLHeaders>
#endif

#include "LongCoordinates.h"
#include <limits.h>
//#include "Global.h"


short int CLIP(long r_value)
{
    if (r_value < SHRT_MIN) return SHRT_MIN;

    if (r_value > SHRT_MAX) return SHRT_MAX;

    return (short int)r_value;
}

long LONGMAX(long x, long y)
{
    return (x < y) ? y : x;
}

long LONGMIN(long x, long y)
{
    return (x < y) ? x : y;
}

/******************************************************************************
 QDPtToLongPt

     Convert a Point to a LongPt
******************************************************************************/

void QDToLongPt(Point srcPt, LongPt* destPt)
{
    destPt->h = srcPt.h;
    destPt->v = srcPt.v;
}

/******************************************************************************
 LongPtToQDPt

     Convert a LongPt to a Point. Values are clipped to 16 bits.
******************************************************************************/

void LongToQDPt(LongPt* srcPt, Point* destPt)
{
    destPt->h = CLIP(srcPt->h);
    destPt->v = CLIP(srcPt->v);
}


/******************************************************************************
 SetLongPt

     Set the members of a LongPt.
******************************************************************************/

void SetLongPt(LongPt* pt, long h, long v)
{
    pt->h = h;
    pt->v = v;
}


/******************************************************************************
 AddLongPt

     Adds srcPt and destPt, returns the result in destPt.
******************************************************************************/

void AddLongPt(LongPt* srcPt, LongPt* destPt)
{
    destPt->h += srcPt->h;
    destPt->v += srcPt->v;
}


/******************************************************************************
 SubLongPt

     Subtracts srcPt from destPt, returns the result in destPt.
******************************************************************************/

void SubLongPt(LongPt* srcPt, LongPt* destPt)
{
    destPt->h -= srcPt->h;
    destPt->v -= srcPt->v;
}


/******************************************************************************
 EqualLongPt

     Returns TRUE if two LongPts are equal.
******************************************************************************/

Boolean EqualLongPt(LongPt* pt1, LongPt* pt2)
{
    return (pt1->h == pt2->h) && (pt1->v == pt2->v);
}


/******************************************************************************
 PtInQDSpace

     Returns TRUE if a LongPt is within the 16-bit QuickDraw coordinate space
******************************************************************************/

Boolean PtInQDSpace(LongPt* pt)
{
    return ((pt->h >= INT_MIN) && (pt->h <= INT_MAX) &&
        (pt->v >= INT_MIN) && (pt->v <= INT_MAX));
}


/******************************************************************************
 QDRectToLongRect

     Convert a Rect to a LongRect
******************************************************************************/

void QDToLongRect(register Rect* srcRect, register LongRect* destRect)
{
    destRect->left = srcRect->left;
    destRect->top = srcRect->top;
    destRect->right = srcRect->right;
    destRect->bottom = srcRect->bottom;
}


/******************************************************************************
 LongRectToQDRect

     Convert aLongRect to a Rect. Values are clipped to 16 bit QuickDraw space.
******************************************************************************/

void LongToQDRect(register LongRect* srcRect, register Rect* destRect)
{
    destRect->left = CLIP(srcRect->left);
    destRect->top = CLIP(srcRect->top);
    destRect->right = CLIP(srcRect->right);
    destRect->bottom = CLIP(srcRect->bottom);
}


/******************************************************************************
 SetLongRect

     Fill in the members of a LongRect.
******************************************************************************/

void SetLongRect(register LongRect* r, long left, long top, long right, long bottom)
{
    r->left = left;
    r->right = right;
    r->top = top;
    r->bottom = bottom;
}


/******************************************************************************
 OffsetLongRect

     Translates a LongRect. Positive values are to the right and down.
******************************************************************************/

void OffsetLongRect(register LongRect* r, long dh, long dv)
{
    r->left += dh;
    r->right += dh;
    r->top += dv;
    r->bottom += dv;
}


/******************************************************************************
 InsetLongRect

     Insets the sides of a LongRect. Positive values move the sizes inward.
******************************************************************************/

void InsetLongRect(register LongRect* r, long dh, long dv)
{
    r->left += dh;
    r->right -= dh;
    r->top += dv;
    r->bottom -= dv;
}


/******************************************************************************
 SectLongRect

     Calculates the intersection of two LongRects and returns the result
     in destRect. destRect may be the same as either src1 or src2. Returns TRUE
     if the result is non-empty.
******************************************************************************/

Boolean SectLongRect(register LongRect* src1, register LongRect* src2,
    register LongRect* destRect)
{
    destRect->left = LONGMAX(src1->left, src2->left);
    destRect->right = LONGMIN(src1->right, src2->right);
    destRect->top = LONGMAX(src1->top, src2->top);
    destRect->bottom = LONGMIN(src1->bottom, src2->bottom);

    return !EmptyLongRect(destRect);
}


/******************************************************************************
 UnionLongRect

    Calculates the union of two LongRects and returns the result
     in destRect. destRect may be the same as either src1 or src2.
******************************************************************************/

void UnionLongRect(LongRect* src1, LongRect* src2, LongRect* destRect)
{
    destRect->left = LONGMIN(src1->left, src2->left);
    destRect->right = LONGMAX(src1->right, src2->right);
    destRect->top = LONGMIN(src1->top, src2->top);
    destRect->bottom = LONGMAX(src1->bottom, src2->bottom);
}


/******************************************************************************
 PtInLongRect

     Returns TRUE if pt lies within r.
******************************************************************************/

Boolean PtInLongRect(register LongPt* pt, register LongRect* r)
{
    return ((pt->h >= r->left) && (pt->h < r->right) &&
        (pt->v >= r->top) && (pt->v < r->bottom));
}


/******************************************************************************
 Pt2LongRect

     Calculates the minimal rect enclose the two given points.
******************************************************************************/

void Pt2LongRect(register LongPt* pt1, register LongPt* pt2,
    register LongRect* r)
{
    r->left = LONGMIN(pt1->h, pt2->h);
    r->top = LONGMIN(pt1->v, pt2->v);
    r->right = LONGMAX(pt1->h, pt2->h);
    r->bottom = LONGMAX(pt1->v, pt2->v);
}


/******************************************************************************
 EqualLongRect

     Returns true if r1 and r2 are equal.
******************************************************************************/

Boolean EqualLongRect(register LongRect* r1, register LongRect* r2)
{
    return ((r1->left == r2->left) && (r1->top == r2->top) &&
        (r1->right == r2->right) && (r1->bottom == r2->bottom));
}


/******************************************************************************
 EmptyLongRect

     Returns TRUE if r encloses no points.
******************************************************************************/

Boolean EmptyLongRect(register LongRect* r)
{
    return ((r->top >= r->bottom) || (r->left >= r->right));
}


/******************************************************************************
 RectInQDSpace

     Returns TRUE if r is entirely within QD space.
******************************************************************************/

Boolean RectInQDSpace(LongRect* r)
{
    // Dereference longrect pointer
    // TCL 1.1.3 11/30/92 BF
    return PtInQDSpace(&topLeftL(*r)) && PtInQDSpace(&botRightL(*r));
}
