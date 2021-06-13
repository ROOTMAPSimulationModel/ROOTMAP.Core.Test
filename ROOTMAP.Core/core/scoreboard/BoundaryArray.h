/////////////////////////////////////////////////////////////////////////////
// Name:        BoundaryArray.h
// Purpose:     Essentially a specialised wrapper of an array of doubles.
//                Intended as a higher performance replacement of LHorizonArray
// Created:     17/07/2002
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////// 
#ifndef BoundaryArray_H
#define BoundaryArray_H

#include "core/common/Types.h"
#include "core/common/Structures.h"
//#include "core/common/DoubleCoordinates.h"

#include "core/scoreboard/ScoreboardStratum.h"
#include "core/scoreboard/Dimension.h"
#include "core/scoreboard/BoundaryArrayExceptions.h"
#include <vector>

namespace rootmap
{
#define BOUNDARYARRAY_MYBOUNDARIES_STDVECTOR 1
#define BOUNDARYARRAY_MYBOUNDARIES_DOUBLEARRAY 2
#define BOUNDARYARRAY_MYBOUNDARIES BOUNDARYARRAY_MYBOUNDARIES_STDVECTOR

    //
    // Uncomment the following to enable exceptions to be thrown in cases
    // where that might be expected.
    // #define ROOTMAP_BOUNDARY_EXCEPTIONS

    typedef std::vector<double> BoundaryValueVector;

    /**
     * Note that Boundary indices are 1-based... ?
     */
    class BoundaryArray // : public CCollection
    {
    public:
        BoundaryArray(const ScoreboardStratum& ss, const Dimension& d, long int number_of_boundaries, const std::string& name);
        BoundaryArray(const ScoreboardStratum& ss, const Dimension& d, const BoundaryValueVector& boundaries, const std::string& name);
        virtual ~BoundaryArray();

        // Layers
        size_t GetNumBoundaries(void) const;
        size_t GetNumLayers(void) const;

        // Measurements
        double GetBoundary(const unsigned long& boundaryIndex) const;

        double GetFirst(void) const;
        double GetLast(void) const;
        double GetTop(void) const;
        double GetBottom(void) const;
        double GetOverallThickness(void) const;

        //
        void AddBoundary(double position);

        //
        double GetLayerThickness(const unsigned long& layerIndex) const;

        long FindLayerWith(double position) const;
        long FindLayerWith(double position, WrapDirection* wrap_dir) const;

#ifdef BOUNDARYARRAY_MENU
        // Macintosh menu stuff
        void GetLayerTopString(long layerIndex, StringPtr theString);
        void GetLayerBottomString(long layerIndex, StringPtr theString);

        MenuHandle GetMenuHandle(void);
        short int GetMenuID(void);
        long GetLayersMenuCommand(void);
        // internal manipulation of things
        void BuildMenu(void);
#endif // #ifdef BOUNDARYARRAY_MENU

#ifdef BOUNDARY_ARRAY_FILE_ACCESS
        void ReadFromFile(JSimulationFile * sFile, long numberOf);
        void SaveToFile(JSimulationFile * sFile);
#endif // #ifdef BOUNDARY_ARRAY_FILE_ACCESS

        double GetLayerTop(const unsigned long& layerIndex) const;
        double GetLayerBottom(const unsigned long& layerIndex) const;

        /** Synonym for GetLayerTop */
        double GetLayerStart(const unsigned long& layerIndex) const;
        /** Synonym for GetLayerBottom */
        double GetLayerEnd(const unsigned long& layerIndex) const;

    private:
        ScoreboardStratum myScoreboardStratum;
        std::string myName;
        size_t myNumberOfBoundaries;

        double myFirstBoundary;
        double myLastBoundary;
        //
        // Cache this value, as it is used in the Scoreboard
        //
        double myOverallThickness;

        Dimension myDimension;

        //
        // The array of double values
        //    
#if BOUNDARYARRAY_MYBOUNDARIES == BOUNDARYARRAY_MYBOUNDARIES_STDVECTOR
        BoundaryValueVector myBoundaries;
#elif BOUNDARYARRAY_MYBOUNDARIES == BOUNDARYARRAY_MYBOUNDARIES_DOUBLEARRAY
        long int myNumberOfBoundariesInitialised;
        DoubleArray myBoundaries;
#endif

#ifdef BOUNDARYARRAY_MENU
        //
        // The Mac MenuHandle of the "Layers" submenu which is the target
        // of the boundaries in this array
        MenuHandle    myLayersMenuHandle;

        //
        // The Mac Menu ID of the "Layers" submenu which is the target
        // of the boundaries in this array
        short int   myLayersMenuID;

        //
        // The command number of the "Layers" menu item which the target
        // submenu is "attached" to
        long        myLayersMenuCmd;
#endif // #ifdef BOUNDARYARRAY_MENU
    };

    typedef BoundaryArray* BoundaryArrayP;
    typedef BoundaryArrayP BoundaryArrayPerDimension[NumberOfDimensions];

    inline double BoundaryArray::GetBoundary(const unsigned long& boundaryIndex) const
    {
#ifdef ROOTMAP_BOUNDARY_EXCEPTIONS
        if (boundaryIndex < myNumberOfBoundaries)
        {
#endif // #ifdef ROOTMAP_BOUNDARY_EXCEPTIONS
            return myBoundaries[static_cast<unsigned int>(boundaryIndex - 1)];
#ifdef ROOTMAP_BOUNDARY_EXCEPTIONS
        }
        else
        {
            throw BoundaryIndexOutOfBounds(boundaryIndex, myNumberOfBoundaries);
        }
        return 0.0;
#endif
    }

    inline double BoundaryArray::GetLayerTop(const unsigned long& layerIndex) const
    {
        return (GetBoundary(layerIndex));
    }

    inline double BoundaryArray::GetLayerStart(const unsigned long& layerIndex) const
    {
        return (GetLayerTop(layerIndex));
    }

    inline double BoundaryArray::GetLayerBottom(const unsigned long& layerIndex) const
    {
        return (GetBoundary(layerIndex + 1));
    }

    inline double BoundaryArray::GetLayerEnd(const unsigned long& layerIndex) const
    {
        return (GetLayerBottom(layerIndex));
    }

    inline double BoundaryArray::GetFirst(void) const
    {
        return (myFirstBoundary);
    }

    inline double BoundaryArray::GetLast(void) const
    {
        return (myLastBoundary);
    }

    inline double BoundaryArray::GetTop(void) const
    {
        return (GetFirst());
    }

    inline double BoundaryArray::GetBottom(void) const
    {
        return (GetLast());
    }

    inline double BoundaryArray::GetOverallThickness(void) const
    {
        return myOverallThickness;
    }

    inline size_t BoundaryArray::GetNumLayers(void) const
    {
        // -1, because we actually track the boundaries
        return (myNumberOfBoundaries - 1);
    }

    inline size_t BoundaryArray::GetNumBoundaries(void) const
    {
        return (myNumberOfBoundaries);
    }

    // MSA 11.04.20 Newly inlined function

    inline double BoundaryArray::GetLayerThickness(const unsigned long& layerIndex) const
    {
        return (GetLayerBottom(layerIndex) - GetLayerTop(layerIndex));
    }
} /* namespace rootmap */

#endif    // BoundaryArray_H
