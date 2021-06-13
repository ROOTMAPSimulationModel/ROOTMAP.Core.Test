/*
    BoundaryArray.cpp

    Author:            Robert van Hugten
    Description:    <describe the BoundaryArray class here>
*/

#include "core/scoreboard/BoundaryArray.h"
#include "core/common/Constants.h"
#include "core/common/Exceptions.h"
#include "core/utility/Utility.h"


namespace rootmap
{
    BoundaryArray::BoundaryArray
    (const ScoreboardStratum& ss,
        const Dimension& d,
        long int number_of_boundaries,
        const std::string& name)
        : myScoreboardStratum(ss)
        , myDimension(d)
        , myName(name)
        , myNumberOfBoundaries(number_of_boundaries)
        , myFirstBoundary(0.0)
        , myLastBoundary(0.0)
        , myOverallThickness(0.0)
    {
    }

    BoundaryArray::BoundaryArray
    (const ScoreboardStratum& ss,
        const Dimension& d,
        const BoundaryValueVector& boundaries,
        const std::string& name)
        : myScoreboardStratum(ss)
        , myDimension(d)
        , myName(name)
        , myNumberOfBoundaries(0)
        , myFirstBoundary(0.0)
        , myLastBoundary(0.0)
        , myOverallThickness(0.0)
    {
        myNumberOfBoundaries = boundaries.size();
        BoundaryValueVector::const_iterator it = boundaries.begin();
        while (it != boundaries.end())
        {
            AddBoundary(*it);
            ++it;
        }
    }

    BoundaryArray::~BoundaryArray()
    {
    }


    void BoundaryArray::AddBoundary(double position)
    {
        bool isWithinCurrent = false;
        if ((position > myFirstBoundary) && (position < myLastBoundary))
        {
            isWithinCurrent = true;
        }

        if (isWithinCurrent)
        {
            throw UnimplementedFunctionException("BoundaryArray::AddBoundary does not support inserting boundaries (YET)");
        }
        else
        {
            myBoundaries.push_back(position);
        }

        myFirstBoundary = myBoundaries[0];
        myNumberOfBoundaries = myBoundaries.size();
        myLastBoundary = myBoundaries[myNumberOfBoundaries - 1];
        myOverallThickness = Utility::CSAbs(myLastBoundary - myFirstBoundary);
    }

    long BoundaryArray::FindLayerWith(double position) const
    {
        unsigned long layer_index = 1;
        //unsigned long layer_maximum = GetNumLayers();

        // prepare for the search by ensuring that the position being searched
        // for is within the scoreboard bounds
        while (position < myFirstBoundary)
        {
            position += myOverallThickness;
        }
        while (position > myLastBoundary)
        {
            position -= myOverallThickness;
        }

        // MSA 11.02.16 Kludge in case of a position located on the last boundary exactly
        if (position == myLastBoundary)
        {
            position -= 1e-12;
        }

        // WHILE (the next boundary's position is less than the requested position)
        //       AND
        //       (there are more layers)
        while (GetBoundary(layer_index) <= position)
        {
            ++layer_index;
        }

        // otherwise, we had found the right layer - now we need to decrement the layer counter
        return (--layer_index);
    }

    long BoundaryArray::FindLayerWith(double position, WrapDirection* wrap_dir) const
    {
        unsigned long layer_index = 1;
        //unsigned long layer_maximum = GetNumLayers();

        // prepare for the search by ensuring that the position being searched
        // for is within the scoreboard bounds, noting that we have needed to
        // "wrap" if not.
        while (position < myFirstBoundary)
        {
            position += myOverallThickness;
            *wrap_dir = -1;
        }
        while (position > myLastBoundary)
        {
            position -= myOverallThickness;
            *wrap_dir = -1;
        }

        // MSA 11.02.16 Kludge in case of a position located on the last boundary exactly
        if (position == myLastBoundary)
        {
            position -= 1e-12;
        }

        // WHILE (the next boundary's position is less than the requested position)
        //       AND
        //       (there are more layers)
        while (GetBoundary(layer_index) <= position)
        {
            ++layer_index;
        }

        // otherwise, we had found the right layer - now we need to decrement the layer counter
        return (--layer_index);
    }
} /* namespace rootmap */
