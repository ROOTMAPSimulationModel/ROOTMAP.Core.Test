#ifndef ScoreboardDataDAI_H
#define ScoreboardDataDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataDAI.h
// Purpose:     Declaration of the ScoreboardDataDAI and support
//              classes for 3, 2, 1 and 0-dimensional data.
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
// #include "core/scoreboard/ScoreboardBox.h"
#include "core/scoreboard/Dimension.h"


#include <map>
#include <list>

namespace rootmap
{
    enum SchemeContentElementType
    {
        sceScoreboard,
        scePlane,
        sceRowColumn,
        sceBoxes,
        sceBox,
        sceInvalid
    };

    struct DimensionArray
    {
        int x, y, z;

        DimensionArray(int x_, int y_, int z_) : x(x_), y(y_), z(z_)
        {
        }
    };

    struct DimensionValuePair
    {
        Dimension dimension;
        long dimension_value;

        DimensionValuePair(Dimension d, long v) : dimension(d), dimension_value(v)
        {
        }
    };

    struct SchemeContentElement
    {
        SchemeContentElementType m_elementType;
        std::list<DimensionValuePair> m_dimensions;
        std::list<DimensionArray> m_arrays;
        double m_value;
    };

    typedef std::list<SchemeContentElement> SchemeContentElementList;


    class ScoreboardDataDAI
    {
    public:
        virtual ~ScoreboardDataDAI()
        {
        }

        virtual const std::string& getProcessName() const = 0;

        virtual const std::string& getCharacteristicName() const = 0;

        /**
         *
         * @param
         * @return
         */
        virtual const SchemeContentElementList& getElementList() const = 0;


    protected:
        ScoreboardDataDAI()
        {
        }

    private:

        ///
        /// member declaration
    }; // class ScoreboardDataDAI

    std::ostream& operator<<(std::ostream& ostr, SchemeContentElementType elementType);
    std::ostream& operator<<(std::ostream& ostr, SchemeContentElement element);
} /* namespace rootmap */

#endif // #ifndef ScoreboardDataDAI_H
