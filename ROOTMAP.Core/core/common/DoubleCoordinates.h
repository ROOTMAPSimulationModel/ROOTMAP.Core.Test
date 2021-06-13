#ifndef DoubleCoordinates_H
#define DoubleCoordinates_H
/******************************************************************************
DoubleCoordinates.h
 ******************************************************************************/
 //#include <Types.h>
#include "core/macos_compatibility/macos_compatibility.h"
#include "core/macos_compatibility/LongCoordinates.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "core/utility/Vec3d.h"
#include "core/common/RmAssert.h"


// We use the fastest double for the platform as the basis for our double:
//#define cs_double double

// Forward declaration of DoublePt for use by DoubleCoordinate
class DoublePt;

// MSA 10.07.26 DoubleCoordinate is defined in the default namespace to allow integration with boost::geometry
class DoubleCoordinate
{
public:
    double x;
    double y;
    double z;

    DoubleCoordinate();
    DoubleCoordinate(double x_, double y_, double z_);
    void set(double x_, double y_, double z_);
    //void set(const DoublePt & pt, rootmap::ViewDirection viewdir);
    bool operator==(const DoubleCoordinate& other) const;
    bool operator!=(const DoubleCoordinate& other) const;
    DoubleCoordinate operator+(const DoubleCoordinate& rhs) const;
    DoubleCoordinate operator-(const DoubleCoordinate& rhs) const;
    DoubleCoordinate operator+(const rootmap::Vec3d& rhs) const;
    DoubleCoordinate& operator*(const double& multiplier);
};

inline DoubleCoordinate::DoubleCoordinate()
    : x(0), y(0), z(0)
{
}

inline DoubleCoordinate::DoubleCoordinate(double x_, double y_, double z_)
    : x(x_), y(y_), z(z_)
{
}

inline void DoubleCoordinate::set(double x_, double y_, double z_)
{
    x = x_;
    y = y_;
    z = z_;
}

inline bool DoubleCoordinate::operator==(const DoubleCoordinate& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

inline bool DoubleCoordinate::operator!=(const DoubleCoordinate& other) const
{
    return x != other.x || y != other.y || z != other.z;
}

inline DoubleCoordinate DoubleCoordinate::operator+(const DoubleCoordinate& rhs) const
{
    return DoubleCoordinate(x + rhs.x, y + rhs.y, z + rhs.z);
}

inline DoubleCoordinate DoubleCoordinate::operator-(const DoubleCoordinate& rhs) const
{
    return DoubleCoordinate(x - rhs.x, y - rhs.y, z - rhs.z);
}

inline DoubleCoordinate DoubleCoordinate::operator+(const rootmap::Vec3d& rhs) const
{
    return DoubleCoordinate(x + rhs.x, y + rhs.y, z + rhs.z);
}

inline DoubleCoordinate& DoubleCoordinate::operator*(const double& multiplier)
{
    x *= multiplier;
    y *= multiplier;
    z *= multiplier;
    return *this;
}

// Register DoubleCoordinate with boost::geometry
//BOOST_GEOMETRY_REGISTER_POINT_3D(DoubleCoordinate, double, cs::cartesian, x, y, z)

namespace rootmap
{
    class DoublePt
    {
    public:
        double v;
        double h;

        DoublePt();
        DoublePt(double h_, double v_);
        DoublePt(const DoubleCoordinate& coord, ViewDirection viewDir);
        void Set(double h_, double v_);
        // returns an Abs'd version of this
        DoublePt Abs() const;
        DoublePt& Abs();
        // sets this to the Abs'd version of "unabs" and returns this
        DoublePt& Abs(const DoublePt& unabs);

        DoublePt& operator-(const DoublePt& other);

        //DoublePt & operator-=(const DoublePt & other);
        void Offset(const DoublePt& offset);
    };

    typedef DoublePt* DoublePtPtr;

    inline DoublePt::DoublePt() : v(0.0), h(0.0)
    {
    }

    inline DoublePt::DoublePt(double h_, double v_) : v(v_), h(h_)
    {
    }

    inline void DoublePt::Set(double h_, double v_)
    {
        v = v_;
        h = h_;
    }

    inline void DoublePt::Offset(const DoublePt& offset)
    {
        v += offset.v;
        h += offset.h;
    }

    struct DoubleRect
    {
        double top, left, bottom, right;
        DoubleRect();
        DoubleRect(double top_, double left_, double bottom_, double right_);

        /** convenience function */
        void Set(double top_, double left_, double bottom_, double right_);

        /**  */
        void Offset(const DoublePt& offset);
        void SetPosition(double v, double h);
        void SetPosition(const DoublePt& position);

        double GetWidth() const;
        double GetHeight() const;
    };

    typedef struct DoubleRect* DoubleRectPtr;

    inline DoubleRect::DoubleRect()
        : top(0.0), left(0.0), bottom(0.0), right(0.0)
    {
    }

    inline DoubleRect::DoubleRect(double top_, double left_, double bottom_, double right_)
        : top(top_), left(left_), bottom(bottom_), right(right_)
    {
    }

    inline void DoubleRect::Set(double top_, double left_, double bottom_, double right_)
    {
        top = top_;
        left = left_;
        bottom = bottom_;
        right = right_;
    }

    inline void DoubleRect::Offset(const DoublePt& offset)
    {
        top += offset.v;
        left += offset.h;
        bottom += offset.v;
        right += offset.h;
    }

    // access top left and bottom right DoublePt of a DoubleRect
#define topLeftD(r) (((DoublePt *) &(r))[0])
#define botRightD(r) (((DoublePt *) &(r))[1])

    class DoubleBox
    {
    public:

        // Default constructor for legacy usage
        DoubleBox()
        {
        };

        DoubleBox(const double& l, const double& f, const double& t,
            const double& r, const double& ba, const double& bo)
            : left(l)
            , front(f)
            , top(t)
            , right(r)
            , back(ba)
            , bottom(bo)
        {
            RmAssert(l <= r && f <= ba && t <= bo, "Improper DoubleBox construction. Left must be <= Right; Front must be <= Back and Top must be <= Bottom.");
        }

        const bool operator==(const DoubleBox& rhs) const
        {
            return left == rhs.left && right == rhs.right
                && top == rhs.top && bottom == rhs.bottom
                && front == rhs.front && back == rhs.back;
        }

        const bool operator!=(const DoubleBox& rhs) const
        {
            return !this->operator==(rhs);
        }

        const double GetVolume() const
        {
            return (abs(bottom - top) * abs(right - left) * abs(back - front));
        }

        double top;
        double left;
        double bottom;
        double right;
        double front;
        double back;
    };

    typedef double DoubleBoxArray[6];

    // enumerated types for use with DoubleBoxArray, making it point-compatible with DoubleBox (but not SoilDoubleBox, i think)
    enum EBoxSide
    {
        boxTop = 0,
        boxLeft,
        boxBottom,
        boxRight,
        boxFront,
        boxBack
    };

#define DOUBLEBOXPTR_2_ARRAY(fboxptr) ((DoubleBoxArray)((void*)fboxptr))
#define DOUBLEBOX_2_ARRAY(fbox) ((DoubleBoxArray)&(fbox))
#define DOUBLEBOX_OFFSET(offset) (((DoubleBoxArray)&(fbox))[offset])
#define BoxSidesFirst boxTop
#define BoxSidesLast boxBack

    /*
     *    DoublePt utilities
     *
     */
    void QDToDoublePt(Point srcPt, DoublePt* destPt);
    void DoubleToQDPt(DoublePt* srcPt, Point* destPt);
    void SetDoublePt(DoublePt* pt, double h, double v);
    void AddDoublePt(DoublePt* srcPt, DoublePt* destPt);
    void SubDoublePt(DoublePt* srcPt, DoublePt* destPt);
    bool EqualDoublePt(DoublePt* pt1, DoublePt* pt2);
    bool DoublePtInQDSpace(DoublePt* pt);

    void LongToDoublePt(LongPt srcPt, DoublePt* destPt);
    void DoubleToLongPt(DoublePt* srcPt, LongPt* destPt);
    bool DoublePtInLongSpace(DoublePt* pt);


    /*
     *    DoubleRect utilities
     *
     */
    void QDToDoubleRect(Rect* srcRect, DoubleRect* destRect);
    void DoubleToQDRect(DoubleRect* srcRect, Rect* destRect);
    void SetDoubleRect(DoubleRect* r, double left, double top, double right, double bottom);
    void OffsetDoubleRect(DoubleRect* r, double dh, double dv);
    void InsetDoubleRect(DoubleRect* r, double dh, double dv);
    bool SectDoubleRect(DoubleRect* src1, DoubleRect* src2, DoubleRect* destRect);
    void UnionDoubleRect(DoubleRect* src1, DoubleRect* src2, DoubleRect* destRect);
    bool PtInDoubleRect(DoublePt* pt, DoubleRect* r);
    void Pt2DoubleRect(DoublePt* pt1, DoublePt* pt2, DoubleRect* r);
    bool EqualDoubleRect(DoubleRect* r1, DoubleRect* r2);
    bool EmptyDoubleRect(DoubleRect* r);
    bool DoubleRectInQDSpace(DoubleRect* r);

    void LongToDoubleRect(LongRect* srcRect, DoubleRect* destRect);
    void DoubleToLongRect(DoubleRect* srcRect, LongRect* destRect);
    bool DoubleRectInLongSpace(DoubleRect* r);

    bool EqualDoubleCoordinate(DoubleCoordinate* i1, DoubleCoordinate* i2);

    void SetDoubleBox(DoubleBox* box, double top, double left, double bottom, double right, double front, double back);


    void DoubleCoordinateToPt(const DoubleCoordinate* double_index, DoublePt* double_pt, ViewDirection viewdir);
    void DoubleCoordinateToPt(const DoubleCoordinate* findex, double& hpt, double& vpt, ViewDirection dirn);

    // Point 2 Coordinate
    void DoublePtToCoordinate(DoublePt* double_pt, DoubleCoordinate* double_index, ViewDirection viewdir);

    void DoubleBoxToRect(const DoubleBox& double_box, DoubleRect* double_rect, ViewDirection viewdir);
    double Distance3D(const DoubleCoordinate* p1, const DoubleCoordinate* p2);
    double DistanceSquared3D(const DoubleCoordinate* p1, const DoubleCoordinate* p2);
    double VectorSize(const DoubleCoordinate* vector);

    // MSA 11.05.02 Newly inlined functions

    /**
     * The distance between 2 3D points, being determined by the formula
     * dist = sqrt( (X2-X1)^2 + (Y2-Y1)^2 + (Z2-Z1)^2 )
     */
    inline double Distance3D(const DoubleCoordinate* p1, const DoubleCoordinate* p2)
    {
        return (sqrt(((p2->x - p1->x) * (p2->x - p1->x)) + ((p2->y - p1->y) * (p2->y - p1->y)) + ((p2->z - p1->z) * (p2->z - p1->z))));
    }

    /**
     *    Use this function when you only need relative differences, to avoid the cost of a square root calculation.
     *
     */
    inline double DistanceSquared3D(const DoubleCoordinate* p1, const DoubleCoordinate* p2)
    {
        return (p2->x - p1->x) * (p2->x - p1->x) + (p2->y - p1->y) * (p2->y - p1->y) + (p2->z - p1->z) * (p2->z - p1->z);
    }

    // similar to Distance3D, assumes p2 is <0,0,0>
    // good for unit-vectors
    inline double VectorSize(const DoubleCoordinate* vector)
    {
        return (sqrt((vector->x * vector->x) + (vector->y * vector->y) + (vector->z * vector->z)));
    }
} /* namespace rootmap */


#endif // #ifndef DoubleCoordinates_H
