#ifndef ScoreboardInitialisationDAI_H
#define ScoreboardInitialisationDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardInitialisationDAI.h
// Purpose:     Declaration of the ScoreboardInitialisationDAI and support
//              classes for 3, 2, 1 and 0-dimensional data.
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/scoreboard/ScoreboardBox.h"
#include "core/scoreboard/Dimension.h"
#include "core/common/Structures.h"

#include <list>

namespace rootmap
{
    /** Defines the interface available for retrieving the values from the
     * "three_dimensional_data" tagged section of a Scoreboard initialisation
     * file
     */
    class ScoreboardThreeDimensionalDAI
    {
    public:
        virtual void getBoxCoordinates(BoxCoordinate & start_box, BoxCoordinate & end_box) const = 0;

        virtual bool hasFillWith() = 0;
        virtual double_cs getFillWith() = 0;

        virtual void getDimensionOrder(Dimension & first, Dimension & second, Dimension & third) const = 0;
        virtual Dimension get1stDimensionOrder() const = 0;
        virtual Dimension get2ndDimensionOrder() const = 0;
        virtual Dimension get3rdDimensionOrder() const = 0;
        virtual const RmString & getData() const = 0;

        virtual ~ScoreboardThreeDimensionalDAI() {}

    protected:
        ScoreboardThreeDimensionalDAI() {}
    }; // class ScoreboardThreeDimensionalDAI

    ///
    /// Type definition for a list of this data interface
    typedef std::list< ScoreboardThreeDimensionalDAI * > ThreeDimensionalDAI;


    /** Defines the interface available for retrieving the values from the
     * "two_dimensional_data" tagged section of a Scoreboard initialisation
     * file
     */
    class ScoreboardTwoDimensionalDAI
    {
    public:
        virtual Plane getPlane() const = 0;
        virtual long getNumber() const = 0;
        virtual double_cs getValue() const = 0;

        virtual ~ScoreboardTwoDimensionalDAI() {}

    protected:
        ScoreboardTwoDimensionalDAI() {}
    }; // class ScoreboardTwoDimensionalDAI

    ///
    /// Type definition for a list of this data interface
    typedef std::list< ScoreboardTwoDimensionalDAI * > TwoDimensionalDAI;


    /** Defines the interface available for retrieving the values from the
     * "one_dimensional_data" tagged section of a Scoreboard initialisation
     * file
     */
    class ScoreboardOneDimensionalDAI
    {
    public:
        virtual Dimension getDimension() const = 0;
        virtual long getDimensionValue(Dimension dim) const = 0;
        virtual double_cs getValue() const = 0;

        virtual ~ScoreboardOneDimensionalDAI() {}

    protected:
        ScoreboardOneDimensionalDAI() {}
    }; // class ScoreboardOneDimensionalDAI

    ///
    /// Type definition for a list of this data interface
    typedef std::list< ScoreboardOneDimensionalDAI * > OneDimensionalDAI;


    /** Defines the interface available for retrieving the values from the
     * "zero_dimensional_data" tagged section of a Scoreboard initialisation
     * file
     */
    class ScoreboardZeroDimensionalDAI
    {
    public:
        virtual BoxCoordinate getBox() const = 0;
        virtual double_cs getValue() const = 0;

        virtual ~ScoreboardZeroDimensionalDAI() {}

    protected:
        ScoreboardZeroDimensionalDAI() {}
    }; // class ScoreboardZeroDimensionalDAI

    ///
    /// Type definition for a list of this data interface
    typedef std::list< ScoreboardZeroDimensionalDAI * > ZeroDimensionalDAI;


    class ScoreboardInitialisationDAI
    {
    public:
        virtual ~ScoreboardInitialisationDAI() {}
        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */

        virtual RmString getCharacteristicName() = 0;
        virtual BoxCoordinate getScoreboardSize() = 0;

        virtual ThreeDimensionalDAI get3DimensionalData() = 0;
        virtual TwoDimensionalDAI   get2DimensionalData() = 0;
        virtual OneDimensionalDAI   get1DimensionalData() = 0;
        virtual ZeroDimensionalDAI  get0DimensionalData() = 0;

    protected:
        ScoreboardInitialisationDAI() {}
        
    private:

        ///
        /// member declaration


    }; // class ScoreboardInitialisationDAI

} // namespace rootmap

#endif // #ifndef ScoreboardInitialisationDAI_H
