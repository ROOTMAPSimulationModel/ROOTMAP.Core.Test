// 
//    BoundaryArrayExceptions.h
//
//  Author        : Robert van Hugten
//  Date Created  : 200207017 (Wednesday 17th July 2002)
//  Description   : Contains exception classes used by the BoundaryArray
//                  class.

#ifndef BoundaryArrayExceptions_H
#define BoundaryArrayExceptions_H

#include <exception>

namespace rootmap
{
    class BoundaryIndexOutOfBounds : public std::exception
    {
    public:
        BoundaryIndexOutOfBounds(long index, long max) : m_index(index), m_max(max)
        {
        }

        BoundaryIndexOutOfBounds(const BoundaryIndexOutOfBounds&)
        {
        }

        BoundaryIndexOutOfBounds& operator=(const BoundaryIndexOutOfBounds&) { return *this; }

        virtual ~BoundaryIndexOutOfBounds()
        {
        };
        virtual const char* what() const { return "BoundaryIndexOutOfBounds"; };

        long GetIndex() const { return m_index; }
        long GetMax() const { return m_max; }
    private:
        long m_index;
        long m_max;
    };

    class BoundaryArrayNull : public std::exception
    {
    public:
        BoundaryArrayNull()
        {
        }

        BoundaryArrayNull(const BoundaryArrayNull&)
        {
        }

        BoundaryArrayNull& operator=(const BoundaryArrayNull&) { return *this; }

        virtual ~BoundaryArrayNull()
        {
        };
        virtual const char* what() const { return "BoundaryArrayNull"; };
    };
} /* namespace rootmap */

#endif // #ifndef BoundaryArrayExceptions_H
