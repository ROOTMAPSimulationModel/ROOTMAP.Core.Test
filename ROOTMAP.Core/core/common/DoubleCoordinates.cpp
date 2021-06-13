/******************************************************************************
DoubleCoordinates.c

Utility functions to convert QuickDraw and their double coordinate
counterparts back and forth, and double coordinate versions
of the most frequently used point and rectangle utilities.

Might be Copyright © 1991, 1994 Symantec Corporation, All rights reserved,
since i pilferred this heavily from the LongCoordinates.cp file.

Changes    : Changed ‘Long’s to ‘Double’s, and added some Double<=>Long utilities.
        : Changed from usage of CS’s MAXINT and MININT to
            SC++’s SHRT_MIN, SHRT_MAX,
                or LONG_MIN, LONG_MAX where appropriate.
        : Changed usage of Max() and Min() macros to CSMax() and CSMin()

Notes    : DoubleCoordinateToDoublePt() is defined in Utilities.h, see comments there
          for why this is so.
        :
******************************************************************************/

#include "core/common/DoubleCoordinates.h"
#include "core/utility/Utility.h"
#include <limits>
#include <cmath>

#ifdef WIN32
#pragma warning(disable:4244)
#endif // #ifdef WIN32

namespace rootmap
{
    using Utility::CSMax;
    using Utility::CSMin;

    DoublePt::DoublePt(const DoubleCoordinate& coord, ViewDirection viewDir)
        : h(0.0), v(0.0)
    {
        switch (viewDir)
        {
        case vFront: h = coord.x;
            v = coord.z;
            break;
        case vSide: h = coord.y;
            v = coord.z;
            break;
        case vTop: h = coord.x;
            v = coord.y;
            break;
        case vNONE:
        default: h = 0.0;
            v = 0.0;
            break;
        } // switch (viewDir)
    }

    /*
    void DoubleCoordinate::set(const DoublePt & pt, ViewDirection viewdir)
    {
        switch (viewdir)
        {
            case vFront: x = pt.h; z = pt.v; break;
            case vSide: y = pt.h; z = pt.v; break;
            case vTop: x = pt.h; y = pt.v; break;
        } // switch (viewdir)
    }
    */
    DoublePt DoublePt::Abs() const
    {
        DoublePt absed(fabs(h), fabs(v));
        return absed;
    }

    DoublePt& DoublePt::Abs()
    {
        h = fabs(h);
        v = fabs(v);
        return *this;
    }

    DoublePt& DoublePt::Abs(const DoublePt& unabs)
    {
        h = fabs(unabs.h);
        v = fabs(unabs.v);
        return *this;
    }

    DoublePt& DoublePt::operator-(const DoublePt& other)
    {
        h -= other.h;
        v -= other.v;

        return *this;
    }


    double DoubleRect::GetWidth() const { return (fabs(right - left)); }
    double DoubleRect::GetHeight() const { return (fabs(bottom - top)); }

    void DoubleRect::SetPosition(const DoublePt& position)
    {
        SetPosition(position.v, position.h);
    }

    void DoubleRect::SetPosition(double v, double h)
    {
        double width = fabs(right - left);
        double height = fabs(bottom - top);

        top = v;
        left = h;

        bottom = top + height;
        right = left + width;
    }


    /* QDPtToDoublePt - Convert a Point to a DoublePt */
    void QDToDoublePt(Point srcPt, DoublePt* destPt)
    {
        destPt->h = srcPt.h;
        destPt->v = srcPt.v;
    }

    /* DoublePtToQDPt - Convert a DoublePt to a Point. Values are clipped to 16 bits. */
    void DoubleToQDPt(DoublePt* srcPt, Point* destPt)
    {
        destPt->h = CSMin(CSMax(static_cast<double>(SHRT_MIN), srcPt->h), static_cast<double>(SHRT_MAX));
        destPt->v = CSMin(CSMax(static_cast<double>(SHRT_MIN), srcPt->v), static_cast<double>(SHRT_MAX));
    }

    /* LongToDoublePt - Convert a LongPt to a DoublePt */
    void LongToDoublePt(LongPt srcPt, DoublePt* destPt)
    {
        destPt->h = srcPt.h;
        destPt->v = srcPt.v;
    }

    /*  DoublePtToLongPt - Convert a DoublePt to a LongPt. Values are clipped to 32 bits. */
    void DoubleToLongPt(DoublePt* srcPt, LongPt* destPt)
    {
        destPt->h = CSMin(CSMax(static_cast<double>(LONG_MIN), srcPt->h), static_cast<double>(LONG_MAX));
        destPt->v = CSMin(CSMax(static_cast<double>(LONG_MIN), srcPt->v), static_cast<double>(LONG_MAX));
    }

    /* SetDoublePt - Set the members of a DoublePt. */

    void SetDoublePt(DoublePt* pt, double h, double v)
    {
        pt->h = h;
        pt->v = v;
    }

    /* AddDoublePt - Adds srcPt and destPt, returns the result in destPt.*/
    void AddDoublePt(DoublePt* srcPt, DoublePt* destPt)
    {
        destPt->h += srcPt->h;
        destPt->v += srcPt->v;
    }

    /* SubDoublePt - Subtracts srcPt from destPt, returns the result in destPt.*/
    void SubDoublePt(DoublePt* srcPt, DoublePt* destPt)
    {
        destPt->h -= srcPt->h;
        destPt->v -= srcPt->v;
    }

    /* EqualDoublePt - Returns TRUE if two DoublePts are equal.*/
    bool EqualDoublePt(DoublePt* pt1, DoublePt* pt2)
    {
        return (pt1->h == pt2->h) && (pt1->v == pt2->v);
    }

    /* DoublePtInQDSpace - Returns TRUE if a DoublePt is within the 16-bit QuickDraw coordinate space*/
    bool DoublePtInQDSpace(DoublePt* pt)
    {
        return ((pt->h >= SHRT_MIN) && (pt->h <= SHRT_MAX) &&
            (pt->v >= SHRT_MIN) && (pt->v <= SHRT_MAX));
    }

    /* DoublePtInLongSpace - Returns TRUE if a DoublePt is within the 32-bit Long coordinate space*/
    bool DoublePtInLongSpace(DoublePt* pt)
    {
        return ((pt->h >= LONG_MIN) && (pt->h <= LONG_MAX) &&
            (pt->v >= LONG_MIN) && (pt->v <= LONG_MAX));
    }

    /* QDRectToDoubleRect - Convert a Rect to a DoubleRect*/
    void QDToDoubleRect(register Rect* srcRect, register DoubleRect* destRect)
    {
        destRect->left = srcRect->left;
        destRect->top = srcRect->top;
        destRect->right = srcRect->right;
        destRect->bottom = srcRect->bottom;
    }

    /* LongToDoubleRect - Convert a Rect to a DoubleRect*/
    void LongToDoubleRect(register LongRect* srcRect, register DoubleRect* destRect)
    {
        destRect->left = srcRect->left;
        destRect->top = srcRect->top;
        destRect->right = srcRect->right;
        destRect->bottom = srcRect->bottom;
    }

    /* DoubleRectToQDRect - Convert aDoubleRect to a Rect. Values are clipped to 16 bit QuickDraw space.*/

    void DoubleToQDRect(register DoubleRect* srcRect, register Rect* destRect)
    {
        destRect->left = CSMin(CSMax(static_cast<double>(SHRT_MIN), srcRect->left), static_cast<double>(SHRT_MAX));
        destRect->top = CSMin(CSMax(static_cast<double>(SHRT_MIN), srcRect->top), static_cast<double>(SHRT_MAX));
        destRect->right = CSMin(CSMax(static_cast<double>(SHRT_MIN), srcRect->right), static_cast<double>(SHRT_MAX));
        destRect->bottom = CSMin(CSMax(static_cast<double>(SHRT_MIN), srcRect->bottom), static_cast<double>(SHRT_MAX));
    }

    /* DoubleRectToLongRect - Convert aDoubleRect to a Rect. Values are clipped to 16 bit QuickDraw space.*/
    void DoubleToLongRect(register DoubleRect* srcRect, register LongRect* destRect)
    {
        destRect->left = CSMin(CSMax(static_cast<double>(LONG_MIN), srcRect->left), static_cast<double>(LONG_MAX));
        destRect->top = CSMin(CSMax(static_cast<double>(LONG_MIN), srcRect->top), static_cast<double>(LONG_MAX));
        destRect->right = CSMin(CSMax(static_cast<double>(LONG_MIN), srcRect->right), static_cast<double>(LONG_MAX));
        destRect->bottom = CSMin(CSMax(static_cast<double>(LONG_MIN), srcRect->bottom), static_cast<double>(LONG_MAX));
    }

    /* SetDoubleRect - Fill in the members of a DoubleRect.*/
    void SetDoubleRect(register DoubleRect* r, double left, double top, double right, double bottom)
    {
        r->left = left;
        r->right = right;
        r->top = top;
        r->bottom = bottom;
    }

    /* OffsetDoubleRect - Translates a DoubleRect. Positive values are to the right and down.*/
    void OffsetDoubleRect(register DoubleRect* r, double dh, double dv)
    {
        r->left += dh;
        r->right += dh;
        r->top += dv;
        r->bottom += dv;
    }

    /* InsetDoubleRect - Insets the sides of a DoubleRect. Positive values move the sizes inward.*/
    void InsetDoubleRect(register DoubleRect* r, double dh, double dv)
    {
        r->left += dh;
        r->right -= dh;
        r->top += dv;
        r->bottom -= dv;
    }

    /* SectDoubleRect
    Calculates the intersection of two DoubleRects and returns the result in destRect. destRect may
    be the same as either src1 or src2. Returns TRUE if the result is non-empty.*/
    bool SectDoubleRect(register DoubleRect* src1, register DoubleRect* src2,
        register DoubleRect* destRect)
    {
        destRect->left = CSMax(src1->left, src2->left);
        destRect->right = CSMin(src1->right, src2->right);
        if (src1->top < src1->bottom)
        { // normal vFront, vSide behaviour
            destRect->top = CSMax(src1->top, src2->top);
            destRect->bottom = CSMin(src1->bottom, src2->bottom);
        }
        else
        { // vTop
            destRect->top = CSMin(src1->top, src2->top);
            destRect->bottom = CSMax(src1->bottom, src2->bottom);
        }

        return !EmptyDoubleRect(destRect);
    }

    /*UnionDoubleRect
    Calculates the union of two DoubleRects and returns the result in destRect.
    destRect may be the same as either src1 or src2.*/
    void UnionDoubleRect(DoubleRect* src1, DoubleRect* src2, DoubleRect* destRect)
    {
        destRect->left = CSMin(src1->left, src2->left);
        destRect->right = CSMax(src1->right, src2->right);
        destRect->top = CSMin(src1->top, src2->top);
        destRect->bottom = CSMax(src1->bottom, src2->bottom);
    }

    /* PtInDoubleRect - Returns TRUE if pt lies within r.*/
    bool PtInDoubleRect(register DoublePt* pt, register DoubleRect* r)
    {
        return ((pt->h >= r->left) && (pt->h < r->right) &&
            (pt->v >= r->top) && (pt->v < r->bottom));
    }

    /* Pt2DoubleRect - Calculates the minimal rect enclose the two given points.*/
    void Pt2DoubleRect(register DoublePt* pt1, register DoublePt* pt2,
        register DoubleRect* r)
    {
        r->left = CSMin(pt1->h, pt2->h);
        r->top = CSMin(pt1->v, pt2->v);
        r->right = CSMax(pt1->h, pt2->h);
        r->bottom = CSMax(pt1->v, pt2->v);
    }

    /* EqualDoubleRect - Returns true if r1 and r2 are equal.*/
    bool EqualDoubleRect(register DoubleRect* r1, register DoubleRect* r2)
    {
        return ((r1->left == r2->left) && (r1->top == r2->top) &&
            (r1->right == r2->right) && (r1->bottom == r2->bottom));
    }

    /* EmptyDoubleRect - Returns TRUE if r encloses no points.*/
    bool EmptyDoubleRect(register DoubleRect* r)
    {
        return ((r->top >= r->bottom) || (r->left >= r->right));
    }

    /* DoubleRectInQDSpace - Returns TRUE if r is entirely within QD space.*/
    bool DoubleRectInQDSpace(DoubleRect* r)
    {
        return DoublePtInQDSpace(&topLeftD(*r)) && DoublePtInQDSpace(&botRightD(*r));
    }

    /* DoubleRectInLongSpace - Returns TRUE if r is entirely within Long space.*/
    bool DoubleRectInLongSpace(DoubleRect* r)
    {
        return DoublePtInLongSpace(&topLeftD(*r)) && DoublePtInLongSpace(&botRightD(*r));
    }

    /* EqualDoubleCoordinate - Returns TRUE i1 and i2 are identical.*/
    bool EqualDoubleCoordinate(DoubleCoordinate* i1, DoubleCoordinate* i2)
    {
        return ((i1->x == i2->x) && (i1->y == i2->y) && (i1->z == i2->z));
    }

    void SetDoubleBox(DoubleBox* box, double top, double left, double bottom, double right, double front, double back)
    {
        box->left = left;
        box->right = right;
        box->bottom = bottom;
        box->top = top;
        box->front = front;
        box->back = back;
    }

    /* DoubleCoordinateToPt */
    void DoubleCoordinateToPt(const DoubleCoordinate* findex, DoublePt* fpt, ViewDirection dirn)
    {
        switch (dirn)
        {
        case vFront:
            fpt->h = findex->x;
            fpt->v = findex->z;
            break;

        case vSide:
            fpt->h = findex->y;
            fpt->v = findex->z;
            break;

        case vTop:
            fpt->h = findex->x;
            fpt->v = findex->y;
            break;

        case vNONE:
        default:
            fpt->h = 0;
            fpt->v = 0;
            break;
        } // end of ‘switch (dirn)’
    } // end of ‘DoubleCoordinateToPt()’

    /* DoubleCoordinateToPt */
    void DoubleCoordinateToPt(const DoubleCoordinate* findex, double& hpt, double& vpt, ViewDirection dirn)
    {
        switch (dirn)
        {
        case vFront:
            hpt = findex->x;
            vpt = findex->z;
            break;

        case vSide:
            hpt = findex->y;
            vpt = findex->z;
            break;

        case vTop:
            hpt = findex->x;
            vpt = findex->y;
            break;

        case vNONE:
        default:
            hpt = 0;
            vpt = 0;
            break;
        } // end of ‘switch (dirn)’
    } // end of ‘DoubleCoordinateToPt()’

    /* DoublePtToCoordinate
    Similar comment to DoubleCoordinatetoDoublePt */
    void DoublePtToCoordinate(DoublePt* fpt, DoubleCoordinate* findex, ViewDirection dirn)
    {
        switch (dirn)
        {
        case vFront:
            findex->x = fpt->h;
            findex->z = fpt->v;
            break;

        case vSide:
            findex->y = fpt->h;
            findex->z = fpt->v;
            break;

        case vTop:
            findex->x = fpt->h;
            findex->y = fpt->v;
            break;

        case vNONE:
        default:
            break;
        } // end of ‘switch (dirn)’
    } // end of ‘DoublePtToCoordinate()’

    /* DoubleBoxToRect
    Similar comment to DoubleCoordinatetoDoublePt */
    void DoubleBoxToRect(const DoubleBox& fbox, DoubleRect* frect, ViewDirection dirn)
    {
        switch (dirn)
        {
        case vFront:
            frect->top = fbox.top;
            frect->left = fbox.left;
            frect->bottom = fbox.bottom;
            frect->right = fbox.right;
            break;
        case vSide:
            frect->top = fbox.top;
            frect->left = fbox.front;
            frect->bottom = fbox.bottom;
            frect->right = fbox.back;
            break;
        case vTop:
            frect->top = fbox.back;
            frect->left = fbox.left;
            frect->bottom = fbox.top;
            frect->right = fbox.right;
            break;
        case vNONE:
        default:
            break;
        }
    }
} /* namespace rootmap */
