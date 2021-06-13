#ifndef Vec3d_H
#define Vec3d_H

#include <cmath>
#include <limits>
#include "core/common/Types.h"
#include "core/common/Constants.h"

namespace rootmap
{
    class Vec3d
    {
    public:

        Vec3d() : x(0.0), y(0.0), z(0.0)
        {
        }

        Vec3d(double xx, double yy, double zz) : x(xx), y(yy), z(zz)
        {
        }

        void crossProd(const Vec3d& v1, const Vec3d& v2);
        double dotProd(const Vec3d& v1) const;
        // Returns the angle between two vectors in radians
        double angleBetween(const Vec3d& rhs) const;
        bool parallelTo(const Vec3d& rhs) const;
        bool orthogonalTo(const Vec3d& rhs) const;
        double Length() const;
        void Normalize(const bool convertZeroVectorToXUnitVector = false);

        Vec3d operator*(double scale) const;
        Vec3d& operator*=(double scale);

        Vec3d operator+(const Vec3d& v) const;
        Vec3d& operator+=(const Vec3d& v);

        Vec3d operator-(const Vec3d& v) const;
        Vec3d& operator-=(const Vec3d& v);

        double x;
        double y;
        double z;
    };

    inline Vec3d operator*(double scale, const Vec3d& v)
    {
        return Vec3d(scale * v.x, scale * v.y, scale * v.z);
    }

    inline void Vec3d::crossProd(const Vec3d& v1, const Vec3d& v2)
    {
        x = v1.y * v2.z - v1.z * v2.y;
        y = v1.z * v2.x - v1.x * v2.z;
        z = v1.x * v2.y - v1.y * v2.x;
        Normalize();
    }

    inline double Vec3d::dotProd(const Vec3d& v1) const
    {
        return x * v1.x + y * v1.y + z * v1.z;
    }

    inline double Vec3d::angleBetween(const Vec3d& rhs) const
    {
        const double dotProduct = dotProd(rhs);
        const double cosTheta = dotProduct / (Length() * rhs.Length());
        return acos(cosTheta);
    }

    inline bool Vec3d::parallelTo(const Vec3d& rhs) const
    {
        double ang = angleBetween(rhs);
        return ang == PI || ang == 0;
    }

    inline bool Vec3d::orthogonalTo(const Vec3d& rhs) const
    {
        return angleBetween(rhs) == (0.5 * PI);
    }

    inline double Vec3d::Length() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    inline void Vec3d::Normalize(const bool convertZeroVectorToXUnitVector)
    {
        const double length = Length();

        if (convertZeroVectorToXUnitVector && abs(length) < std::numeric_limits<double>::epsilon())
        {
            x = 1;
            y = 0;
            z = 0;
        }
        else
        {
            x /= length;
            y /= length;
            z /= length;
        }
    }

    inline Vec3d Vec3d::operator*(const double scale) const
    {
        return Vec3d(x * scale, y * scale, z * scale);
    }

    inline Vec3d Vec3d::operator+(const Vec3d& v) const
    {
        return Vec3d(x + v.x, y + v.y, z + v.z);
    }

    inline Vec3d Vec3d::operator-(const Vec3d& v) const
    {
        return Vec3d(x - v.x, y - v.y, z - v.z);
    }

    inline Vec3d& Vec3d::operator*=(const double scale)
    {
        x *= scale;
        y *= scale;
        z *= scale;
        return *this;
    }

    inline Vec3d& Vec3d::operator+=(const Vec3d& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    inline Vec3d& Vec3d::operator-=(const Vec3d& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
}; /* namespace rootmap */

#endif // #ifndef Vec3d_H
