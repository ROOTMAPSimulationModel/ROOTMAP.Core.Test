#ifndef Scoreboard_H
#define Scoreboard_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Scoreboard.h
// Purpose:     Declaration of the Scoreboard class
// Created:     31-07-2002 (Wednesday July 31, 2002)
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"
#include "core/macos_compatibility/CCollaborator.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/scoreboard/BoundaryArray.h"
#include "core/log/Logger.h"
#include <vector>
#include <list>
#include <bitset>


#define ROOTMAP_SCOREBOARD_GETCHARACTERISTICVALUE_INLINE


namespace rootmap
{
    //
    // FORWARD declarations
    struct CharacteristicDescriptor;
    class SpecialScoreboardData;
    class VolumeObjectCoordinator;
    class VolumeObject;

    // Logically, an TransferRestrictedDimension is a (Dimension, bool) pair which defines
    // a plane in which the VO surface contiguously divides the box, 
    // and the direction in which the plane is restricted inside the VO (true == positive).
    // It is implemented as an enum for efficiency.
    enum TransferRestrictedDimension
    {
        XNegative, // ==0
        XPositive, // ==1, etc
        YNegative,
        YPositive,
        ZNegative,
        ZPositive
    };

    typedef std::bitset<6> TransferRestrictedDimensionBitset;
    // This map is used for nonzero permeabilities.
    typedef std::map<TransferRestrictedDimension, double> TransferRestrictedDimensionPermeabilities;

    struct VolumeObjectScoreboardBoxInteraction
    {
        double coincidentProportion;
        BoxIndex boxIndex;
        VolumeObject* volumeObject;
        VolumeObject* backgroundVolumeObject;
        // Lists the ObstructedDimensions in enumerated order, and whether they are obstructed or not.
        TransferRestrictedDimensionBitset transferRestrictedDimensions;
        TransferRestrictedDimensionPermeabilities transferRestrictedDimensionPermeabilities;
        const bool operator==(const VolumeObjectScoreboardBoxInteraction& rhs) const;
    };

    inline const bool VolumeObjectScoreboardBoxInteraction::operator==(const VolumeObjectScoreboardBoxInteraction& rhs) const
    {
        if (boxIndex == rhs.boxIndex
            && coincidentProportion == rhs.coincidentProportion
            && volumeObject == rhs.volumeObject
            && backgroundVolumeObject == rhs.backgroundVolumeObject
            && transferRestrictedDimensions == rhs.transferRestrictedDimensions
            && transferRestrictedDimensionPermeabilities.size() == rhs.transferRestrictedDimensionPermeabilities.size())
        {
            TransferRestrictedDimensionPermeabilities::const_iterator i1 = transferRestrictedDimensionPermeabilities.begin();
            TransferRestrictedDimensionPermeabilities::const_iterator i2 = rhs.transferRestrictedDimensionPermeabilities.begin();
            for (; i1 != transferRestrictedDimensionPermeabilities.end() && i2 != rhs.transferRestrictedDimensionPermeabilities.end(); ++i1, ++i2)
            {
                if (i1 != i2)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    /**
     *
     * The Scoreboard is is the structure which Processes use to share data about
     * soil characteristics.  They do that by both by summarising their internal
     * data into the Scoreboard for external consumption, and by obtaining
     * characteristic data from it for determining their behaviour.
     *
     *
     * This version of the Scoreboard is organised very differently to its
     * predecessor/precursor, LScoreboard :
     * - it "has a" list of ScoreboardBoxes, rather than "is a".  Thus, it
     *   is a base class, not derived from CVoidPtrArray.  Less virtual functions
     *   can only be good (in fact, it now has None).
     * - it tracks box boundaries by using the class BoundaryArray, rather
     *   than LHorizonArray.
     * - It owns the the box BoundaryArray's, not the Simulation.
     * - through virtue of the previous point, it acts as a fascade for the
     *   BoundaryArray objects it owns.
     * - it allocates memory per characteristic/summary, rather than by per
     *   ScoreboardBox.
     * - LBinaryTree is no longer used. All summaries are registered as they
     *   are required.
     * - the floating point values are now "double", which is in turn
     *    typedef'd to double_t (which in turn is #defined to double).
     *
     * Most of these differences come about through new requirements:
     * - No more lingering elusive infrequent bugs relating to use of LBinaryTree
     *   (combined with...)
     * - dynamic addition of characteristics, rather than dynamic adding and/or
     *   editting of boundaries.
     * - greater overall performance
     * - better design and maintainability
     *
     *
     * Usage:
     *
     * looping through the scoreboard by x/y/z is ok as long as the first
     * step you take is :
     *
     *      const BoxIndex & box_index = scoreboard->GetBoxIndex(x,y,z);
     *
     * then use that in calls to scoreboard functions.
     *
     * The preferred alternative is:
     *
     *     for ( const BoxIndex & box_index = scoreboard->begin() ;
     *           box_index < scoreboard->end() ;
     *           ++box_index
     *         )
     *     {
     *         Get_Concentration_Of_BoxIndex(nitrate_uptake_index, box_index);
     *     }
     *
     */
    class Scoreboard : public ::CCollaborator
    {
    public:
        // constructors and destructor
        Scoreboard(const ScoreboardStratum& ss,
            const BoundaryArray& ba_x,
            const BoundaryArray& ba_y,
            const BoundaryArray& ba_z);

        ~Scoreboard();

        // destruction assistance
        void DisposeCharacteristicValues(void);

        // accessors
        const ScoreboardStratum& GetScoreboardStratum() const;

        void ProviderChanged(CCollaborator* aProvider, long reason, CCollaboratorInfo* info);

        //
        const long GetSizeOfLargestSlice(void) const;

        const long GetNumBoundaries(const Dimension& d) const;
        const long GetNumLayers(const Dimension& d) const;

        void GetNumBoundaries(long& x, long& y, long& z) const;
        void GetNumLayers(long& x, long& y, long& z) const;

        const double GetThickness(const Dimension& d) const;
        void GetThickness(double& x, double& y, double& z) const;

        const double GetTop(const Dimension& d) const;
        const double GetBottom(const Dimension& d) const;

        const long GetNumberOfBoxes() const;

        const BoundaryArray& GetBoundaryArray(const Dimension& d) const;

        const size_t GetNumberOfCharacteristics() const;

        const TransferRestrictedDimension ReverseDirection(const TransferRestrictedDimension& original) const;

        // MSA 10.12.03 A useful utility method. Assigns the Dimensional plane in which the two Boxes are colinear,
        // and assigns the bool inPositiveDirection (true if positive direction, false if negative).
        // Returns true for a successful operation.
        // If the Boxes are not colinear, returns false.
        virtual const bool GetDirectionBetweenColinearBoxes(const BoxIndex& fromBox, const BoxIndex& toBox, Dimension& plane, bool& inPositiveDirection) const;

        //
        // Scoreboard Box Index arithmetic
        //
        const BoxIndex GetBoxIndex(const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const;
        const BoxIndex GetBoxIndex(const BoxCoordinate& box_coordinate) const;
        const BoxIndex GetBoxIndexWrapped(BoxCoordinateIndex x, BoxCoordinateIndex y, BoxCoordinateIndex z) const;
        const BoxIndex GetBoxIndexWrapped(const BoxCoordinate& box_coordinate) const;

        //
        // Scoreboard Box Index reverse arithmetic
        //
        const BoxCoordinateIndex GetX(const BoxIndex& index) const;
        const BoxCoordinateIndex GetY(const BoxIndex& index) const;
        const BoxCoordinateIndex GetZ(const BoxIndex& index) const;
        void GetXYZ(const BoxIndex& boxIndex, BoxCoordinateIndex& x, BoxCoordinateIndex& y, BoxCoordinateIndex& z) const;
        void GetXYZ(const BoxIndex& boxIndex, BoxCoordinate& box_coord) const;

        // Equivalent to ScoreboardCoordinator::GetBoxSoil()
        void GetBoxDimensions(const BoxIndex& index, DoubleBox& box) const;

        //
        // Scoreboard Iterating
        BoxIndex begin() const;
        BoxIndex end() const;

        /**
        *
        */
        void FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate, WrapDirection* wrap_direction);

        /**
         *
         */
        void FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate);

        // MSA This overload of FindBoxBySoilCoordinate is long overdue
        const BoxIndex FindBoxBySoilCoordinate(const DoubleCoordinate* soil_coordinate) const;

        //    
        // Accessors and Mutators of Characteristic values
        double GetCharacteristicValue(const CharacteristicIndex& characteristic_index, const BoxIndex& box) const;
        double GetCharacteristicValue(const CharacteristicIndex& characteristic_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const;
        double GetCharacteristicValue(const CharacteristicIndex& characteristic_index, const BoxCoordinate& box_coordinate) const;

        void SetCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box);
        void SetCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z);
        void SetCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinate& box_coordinate);

        // MSA 09.10.12 New rangewise mutators
        void SetCharacteristicValueRange(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& first, const BoxIndex& last);
        void SetCharacteristicValueRange(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinateIndex& firstx, const BoxCoordinateIndex& firsty, const BoxCoordinateIndex& firstz, const BoxCoordinateIndex& lastx, const BoxCoordinateIndex& lasty, const BoxCoordinateIndex& lastz);
        void SetCharacteristicValueRange(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinate& first_box_coordinate, const BoxCoordinate& last_box_coordinate);


        void AddCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box);
        void AddCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z);
        void AddCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, BoxCoordinate& box_coordinate);

        void SetCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, BoxCoordinate& box_coordinate);
        void SetCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, const double& x, const double& y, const double& z);

        void AddCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, BoxCoordinate& box_coordinate);
        void AddCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, const double& x, const double& y, const double& z);

        void SetPlaneCharacteristicValues(const CharacteristicIndex& characteristic_index, Dimension dimension, long plane_index, const double& value);
        void SetRowColumnCharacteristicValues(const CharacteristicIndex& characteristic_index, BoxCoordinateIndex x_index, BoxCoordinateIndex y_index, BoxCoordinateIndex z_index, const double& value);
        void SetArbitraryBoxesCharacteristicValues(const CharacteristicIndex& characteristic_index, BoxCoordinate& bc1, BoxCoordinate& bc2, const double& value);

        BoxIndex AddCharacteristicLength(const CharacteristicIndex& characteristic_index,
            double value,
            const DoubleCoordinate* start_point,
            const BoxCoordinate* start_box_coord,
            const BoxIndex& start_box,
            const size_t volumeObjectIndex,
            const DoubleCoordinate* end_point,
            const bool& do_density);

        const SpecialScoreboardData* GetSpecialPointer(const CharacteristicIndex& custom_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const;
        const SpecialScoreboardData* GetSpecialPointer(const CharacteristicIndex& custom_index, const BoxIndex& box_index) const;

        void SetSpecialPointer(const CharacteristicIndex& custom_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z, SpecialScoreboardData* p);
        void SetSpecialPointer(const CharacteristicIndex& custom_index, const BoxIndex& box_index, SpecialScoreboardData* p);

        const double GetBoxVolume(const BoxIndex& box) const;
        const double GetBoxVolume(const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const;

        // 
        // Registering Characteristics (and Summaries)
        void RegisterCharacteristic(CharacteristicDescriptor* cd);

        void IncrementVOCount();

        // The inlined, quick version
        const double GetCoincidentProportion(const BoxIndex& boxIndex, const size_t& volumeObjectIndex);
    private:
        // The slow version, which is called for the first reference to each BoxIndex/VolumeObject index pair
        const double RetrieveCoincidentProportion(const BoxIndex& boxIndex, const size_t& volumeObjectIndex);

    public:
#if defined ROOTMAP_TODO_GUI
        //
        // Scoreboard Table Collaboration
        void SetEditor(TEditCell *aProvider);
        void ReleaseEditor(TEditCell *aProvider);
        void ProviderChanged(CCollaborator *aProvider, long reason, void *info);
#endif // #if defined ROOTMAP_TODO_GUI

        //
        // For processes to use, to stuff the whole scoreboard with one value (usually default)
        void FillCharacteristicWithValue(const CharacteristicIndex& characteristic_index, const double& value);
        void FillSpecialDataWithNull(CharacteristicIndex characteristic_index);

        // Used by "Process::DoReceiverDelayedReaction()"
        void TransferAndClearCharacteristicValue(const CharacteristicIndex& from_index, const CharacteristicIndex& to_index, const BoxIndex& box);
        void TransferAndClearCharacteristicValue(CharacteristicIndex from_index, CharacteristicIndex to_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z);

        // i wish this could be restricted to the ScoreboardCoordinator
        // RvH 20060601 Huh? what does that mean ?
        void CalculateBoxVolumes();

        double GetMean(int hi, int vi, ViewDirection viewdir, int characteristicindex);

        const TransferRestrictionData& GetTransferRestrictionData(const BoxIndex& fromBoxIndex, const BoxIndex& toBoxIndex, const TransferRestrictedDimension& obstructionDimension);
        const TransferRestrictedDimension GetTransferRestrictedDimension(const Dimension& plane, const bool& inPositiveDirection) const;

        // ScoreboardBox-wise map of VolumeObject interactions located within each ScoreboardBox.
        // Each VOSBI pointer is associated with a proportion of the box's volume that it shares, in the range (0,1],
        // and a list of Dimensions which are fully obstructed by the VolumeObject surface.
        // See ScoreboardBox.h for VOSBI declaration and
        // ScoreboardCoordinator::RegisterVolumeObject() for usage.
        // Key: BoxIndex. Value: VolumeObjectScoreboardBoxInteraction
        // Pointers are NOT owned by this map, they are owned by ScoreboardCoordinator::m_volumeObjects.
        std::map<BoxIndex, VolumeObjectScoreboardBoxInteraction> m_VOSBIMap;

        static std::string sBoxVolumeCharacteristicName;

        static std::string sConfigName;

    private:
        const TransferRestrictionData& CalculateTransferRestrictionData(const BoxIndex& fromBoxIndex, const BoxIndex& toBoxIndex, const TransferRestrictedDimension& obstructionDimension);

        // accessing of characteristics in a box
        double GetBoxValue(const CharacteristicIndex& characteristic_index, const BoxIndex& box) const;
        void SetBoxValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box);
        void AddBoxValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box);

        const SpecialScoreboardData* GetBoxPointer(const CharacteristicIndex& characteristic_index, const BoxIndex& box) const;
        void SetBoxPointer(const CharacteristicIndex& characteristic_index, const BoxIndex& box, SpecialScoreboardData* p);

        RootMapLoggerDeclaration();

        TransferRestrictionData myDefaultTRD;

        //
        // Boundary Array members.  These were previously owned by the Simulation.
        BoundaryArray myBoundaries_X;
        BoundaryArray myBoundaries_Y;
        BoundaryArray myBoundaries_Z;
        BoundaryArray* myBoundariesArray[NumberOfDimensions];
        long int myNumberOfLayers_X;
        long int myNumberOfLayers_Y;
        long int myNumberOfLayers_Z;
        long int myNumberOfLayersArray[NumberOfDimensions];

        long int myNumberOfBoxes;

        long int myVOCount;

        //
        // The vector of characteristics.  Each characteristic is an array of
        // double values.
        std::vector<CharacteristicValueArray> myCharacteristicValues;

        //
        // The vector of special values.  Each "special value" is an array of
        // void pointers.
        std::vector<SpecialScoreboardDataArray> mySpecialValues;
        //
        // Really really private.
        CharacteristicIndex myBoxVolumeCharacteristicIndex;

        // The volume/stratum of this Scoreboard.
        ScoreboardStratum myScoreboardStratum;

        CharacteristicDescriptor* myBoxVolumeCharacteristicDescriptor;


        //
        //
        std::vector<CharacteristicDescriptor *> myCharacteristicDescriptors;

        std::map<std::pair<BoxIndex, BoxIndex>, TransferRestrictionData> m_transferRestrictionMap;

        std::vector<double> m_coincidentProportions;
    }; // class Scoreboard


    //
    //
    //
    //
    //
    //
    //

    inline const ScoreboardStratum& Scoreboard::GetScoreboardStratum() const
    {
        return myScoreboardStratum;
    }

    inline void Scoreboard::GetNumBoundaries(long& x, long& y, long& z) const
    {
        x = static_cast<long>(myBoundaries_X.GetNumBoundaries());
        y = static_cast<long>(myBoundaries_Y.GetNumBoundaries());
        z = static_cast<long>(myBoundaries_Z.GetNumBoundaries());
    }

    inline void Scoreboard::GetNumLayers(long& x, long& y, long& z) const
    {
        x = myNumberOfLayers_X;
        y = myNumberOfLayers_Y;
        z = myNumberOfLayers_Z;
    }

    inline const long Scoreboard::GetNumLayers(const Dimension& d) const
    {
        return myNumberOfLayersArray[d];
    }

    inline const long Scoreboard::GetNumberOfBoxes() const
    {
        //    return (myNumberOfLayers_X * myNumberOfLayers_Y * myNumberOfLayers_Z);
        return myNumberOfBoxes;
    }

    /**
     *    Takes 1-based BoxCoordinateIndex arguments and returns the 0-based BoxIndex of this ScoreboardBox.
     *    For a Scoreboard of a by b by c Boxes, BoxIndices range from 0 to (abc-1).
     */
    inline const BoxIndex Scoreboard::GetBoxIndex(const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const
    {
        return ((x - 1) + (myNumberOfLayers_X * (y - 1)) + (myNumberOfLayers_X * myNumberOfLayers_Y * (z - 1)));
    }

    inline const BoxIndex Scoreboard::GetBoxIndex(const BoxCoordinate& box_coordinate) const
    {
        return GetBoxIndex(box_coordinate.x, box_coordinate.y, box_coordinate.z);
    }

    inline const BoxIndex Scoreboard::GetBoxIndexWrapped(const BoxCoordinate& box_coordinate) const
    {
        return GetBoxIndexWrapped(box_coordinate.x, box_coordinate.y, box_coordinate.z);
    }

    /**
     *    Returns the 1-based BoxCoordinateIndex of this ScoreboardBox's X coordinate.
     *    For a Scoreboard of a Boxes in the X direction, BoxCoordinateIndices range from 1 to a.
     */
    inline const BoxCoordinateIndex Scoreboard::GetX(const BoxIndex& box_index) const
    {
        // MSA  09.10.08 Modified: BoxIndex is 0-based; should not be decremented before modulus operation
        return (((box_index) % myNumberOfLayers_X) + 1);
    }

    /**
     *    Returns the 1-based BoxCoordinateIndex of this ScoreboardBox's Y coordinate.
     *    For a Scoreboard of b Boxes in the Y direction, BoxCoordinateIndices range from 1 to b.
     */
    inline const BoxCoordinateIndex Scoreboard::GetY(const BoxIndex& box_index) const
    {
        // MSA  09.10.08 Modified: BoxIndex is 0-based; should not be decremented before modulus operation
        return ((((box_index) / myNumberOfLayers_X) % myNumberOfLayers_Y) + 1);
    }

    /**
     *    Returns the 1-based BoxCoordinateIndex of this ScoreboardBox's Z coordinate.
     *    For a Scoreboard of c Boxes in the Z direction, BoxCoordinateIndices range from 1 to c.
     */
    inline const BoxCoordinateIndex Scoreboard::GetZ(const BoxIndex& box_index) const
    {
        // MSA  09.10.08 Modified: BoxIndex is 0-based; should not be decremented before modulus operation
        return ((((box_index) / (myNumberOfLayers_X * myNumberOfLayers_Y))) + 1);
    }

    inline void Scoreboard::GetXYZ(const BoxIndex& box_index, long& x, long& y, long& z) const
    {
        x = GetX(box_index);
        y = GetY(box_index);
        z = GetZ(box_index);
    }

    inline void Scoreboard::GetXYZ(const BoxIndex& box_index, BoxCoordinate& box_coord) const
    {
        box_coord.x = GetX(box_index);
        box_coord.y = GetY(box_index);
        box_coord.z = GetZ(box_index);
    }

    inline BoxIndex Scoreboard::begin() const
    {
        return 0;
    }

    inline BoxIndex Scoreboard::end() const
    {
        return myNumberOfBoxes - 1;
    }


    inline double Scoreboard::GetBoxValue(const CharacteristicIndex& characteristic_index, const BoxIndex& box) const
    {
        return ((myCharacteristicValues[characteristic_index])[box]);
    }

    inline void Scoreboard::SetBoxValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box)
    {
        (myCharacteristicValues[characteristic_index])[box] = value;
    }

    inline void Scoreboard::AddBoxValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box)
    {
        (myCharacteristicValues[characteristic_index])[box] += value;
    }

#if defined ROOTMAP_SCOREBOARD_GETCHARACTERISTICVALUE_INLINE
    inline double Scoreboard::GetCharacteristicValue(const CharacteristicIndex& characteristic_index, const BoxIndex& box_index) const
    {
        return (Scoreboard::GetBoxValue(characteristic_index, box_index));
    }
#endif // #if defined ROOTMAP_SCOREBOARD_GETCHARACTERISTICVALUE_INLINE

    inline double Scoreboard::GetCharacteristicValue(const CharacteristicIndex& characteristic_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const
    {
        return (GetCharacteristicValue(characteristic_index, GetBoxIndex(x, y, z)));
    }

    inline double Scoreboard::GetCharacteristicValue(const CharacteristicIndex& characteristic_index, const BoxCoordinate& box) const
    {
        return (GetCharacteristicValue(characteristic_index, GetBoxIndex(box)));
    }

    inline void Scoreboard::SetCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box_index)
    {
        //    BoxCoordinateIndex x,y,z;
        //    GetNumLayers(x,y,z);
        //    BoxIndex bi = GetBoxIndex(x,y,z);
        //    long numboxes = GetNumberOfBoxes();
        //    if ( ( characteristic_index == 134 ) &&
        //         ( box_index == bi )
        //       )
        //    {
        //        Debugger();
        //    }
        Scoreboard::SetBoxValue(characteristic_index, value, box_index);
    }

    inline void Scoreboard::SetCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z)
    {
        SetCharacteristicValue(characteristic_index, value, GetBoxIndex(x, y, z));
    }

    inline void Scoreboard::SetCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinate& box)
    {
        SetCharacteristicValue(characteristic_index, value, GetBoxIndex(box));
    }

    // MSA 09.10.12 New rangewise mutators
    /**
     *    Uses std::fill to set box range [first, last) to value. See http://www.cplusplus.com/reference/algorithm/fill/
     */
    inline void Scoreboard::SetCharacteristicValueRange(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& first, const BoxIndex& last)
    {
        std::fill(myCharacteristicValues[characteristic_index] + first, myCharacteristicValues[characteristic_index] + last, value);
    }

    /**
     *    IMPORTANT: This variant fills from (firstx,firsty,firstz) to (lastx,lasty,lastz) INCLUSIVELY.
     *    This is for user-friendliness, so callers can refer to the volume they wish to set directly
     *  (e.g. the "cube" made up of Boxes (1,1,1) to (3,3,3)).
     *
     *    Callers of SetCharacteristicValueRange(..., const BoxIndex & first, const BoxIndex & last) [see above]
     *  are more likely to be simply resetting the whole Scoreboard to a certain value, hence its usage of std::fill-style arguments.
     */
    inline void Scoreboard::SetCharacteristicValueRange(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinateIndex& firstx, const BoxCoordinateIndex& firsty, const BoxCoordinateIndex& firstz, const BoxCoordinateIndex& lastx, const BoxCoordinateIndex& lasty, const BoxCoordinateIndex& lastz)
    {
        for (BoxCoordinateIndex z = firstz; z <= lastz; ++z)
        {
            for (BoxCoordinateIndex y = firsty; y <= lasty; ++y)
            {
                // x varies fastest --> contiguous series of x-values are contiguous in myCharacteristicValues
                // e.g. Take a hypothetical 3x3x3 Scoreboard; the vector (as an array) would look like this:
                // [x0y0z0,x1y0z0,x2y0z0,x0y1z0,x1y1z0,x2y1z0,x0y2z0,x1y2z0,x2y2z0,x0y0z1,x1y0z1,x2y0z1,...]
                std::fill(myCharacteristicValues[characteristic_index] + GetBoxIndex(firstx, y, z), myCharacteristicValues[characteristic_index] + GetBoxIndex(lastx, y, z) + 1, value);
            }
        }
    }

    /**
     *    IMPORTANT: This variant fills from first_box_coordinate to last_box_coordinate INCLUSIVELY.
     *    This is for user-friendliness, so callers can refer to the volume they wish to set directly
     *  (e.g. the "cube" made up of Boxes (1,1,1) to (3,3,3)).
     *
     *    Callers of SetCharacteristicValueRange(..., const BoxIndex & first, const BoxIndex & last) [see above]
     *  are more likely to be simply resetting the whole Scoreboard to a certain value, hence its usage of std::fill-style arguments.
     */
    inline void Scoreboard::SetCharacteristicValueRange(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinate& first_box_coordinate, const BoxCoordinate& last_box_coordinate)
    {
        Scoreboard::SetCharacteristicValueRange(characteristic_index, value, first_box_coordinate.x, first_box_coordinate.y, first_box_coordinate.z,
            last_box_coordinate.x, last_box_coordinate.y, last_box_coordinate.z);
    }


    inline void Scoreboard::AddCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxIndex& box_index)
    {
        Scoreboard::AddBoxValue(characteristic_index, value, box_index);
    }

    inline void Scoreboard::AddCharacteristicValue(const CharacteristicIndex& characteristic_index, const double& value, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z)
    {
        AddCharacteristicValue(characteristic_index, value, GetBoxIndex(x, y, z));
    }

    inline void Scoreboard::TransferAndClearCharacteristicValue(CharacteristicIndex from_index, CharacteristicIndex to_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z)
    {
        Scoreboard::TransferAndClearCharacteristicValue(from_index, to_index, GetBoxIndex(x, y, z));
    }


    inline const SpecialScoreboardData* Scoreboard::GetBoxPointer(const CharacteristicIndex& characteristic_index, const BoxIndex& box) const
    {
        return ((mySpecialValues[characteristic_index])[box]);
    }

    inline void Scoreboard::SetBoxPointer(const CharacteristicIndex& characteristic_index, const BoxIndex& box, SpecialScoreboardData* p)
    {
        (mySpecialValues[characteristic_index])[box] = p;
    }

    inline const SpecialScoreboardData* Scoreboard::GetSpecialPointer(const CharacteristicIndex& custom_index, const BoxIndex& box) const
    {
        //void * p = GetBoxPointer(custom_index, box);
        //return (reinterpret_cast<SpecialScoreboardData *>(p));
        return GetBoxPointer(custom_index, box);
    }

    inline const SpecialScoreboardData* Scoreboard::GetSpecialPointer(const CharacteristicIndex& custom_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const
    {
        return GetSpecialPointer(custom_index, GetBoxIndex(x, y, z));
    }

    inline void Scoreboard::SetSpecialPointer(const CharacteristicIndex& custom_index, const BoxIndex& box, SpecialScoreboardData* p)
    {
        SetBoxPointer(custom_index, box, p);
    }

    inline void Scoreboard::SetSpecialPointer(const CharacteristicIndex& custom_index, const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z, SpecialScoreboardData* p)
    {
        SetSpecialPointer(custom_index, GetBoxIndex(x, y, z), p);
    }

    inline const double Scoreboard::GetBoxVolume(const BoxIndex& box_index) const
    {
        return (GetCharacteristicValue(myBoxVolumeCharacteristicIndex, box_index));
    }

    inline const double Scoreboard::GetBoxVolume(const BoxCoordinateIndex& x, const BoxCoordinateIndex& y, const BoxCoordinateIndex& z) const
    {
        return (GetBoxVolume(GetBoxIndex(x, y, z)));
    }

    inline const BoundaryArray& Scoreboard::GetBoundaryArray(const Dimension& d) const
    {
        return (*(myBoundariesArray[d]));
    }

    inline const size_t Scoreboard::GetNumberOfCharacteristics() const
    {
        return (myCharacteristicValues.size());
    }

    inline void Scoreboard::GetBoxDimensions(const BoxIndex& index, DoubleBox& box) const
    {
        BoxCoordinate bc;
        GetXYZ(index, bc);

        box.left = myBoundaries_X.GetLayerTop(bc.x);
        box.right = myBoundaries_X.GetLayerBottom(bc.x);

        box.front = myBoundaries_Y.GetLayerTop(bc.y);
        box.back = myBoundaries_Y.GetLayerBottom(bc.y);

        box.top = myBoundaries_Z.GetLayerTop(bc.z);
        box.bottom = myBoundaries_Z.GetLayerBottom(bc.z);
    }

    inline const TransferRestrictedDimension Scoreboard::GetTransferRestrictedDimension(const Dimension& plane, const bool& inPositiveDirection) const
    {
        if (plane == X)
        {
            if (inPositiveDirection) return XPositive;
            else return XNegative;
        }
        else if (plane == Y)
        {
            if (inPositiveDirection) return YPositive;
            else return YNegative;
        }
        else
        {
            if (inPositiveDirection) return ZPositive;
            else return ZNegative;
        }
    }

    // MSA 11.04.20 Recently inlined functions

    inline const long Scoreboard::GetSizeOfLargestSlice(void) const
    {
        size_t xysize = myNumberOfLayers_X * myNumberOfLayers_Y;
        size_t xzsize = myNumberOfLayers_X * myNumberOfLayers_Z;
        size_t yzsize = myNumberOfLayers_Y * myNumberOfLayers_Z;

        return (Utility::CSMax(yzsize, Utility::CSMax(xysize, xzsize)));
    }


    inline const long Scoreboard::GetNumBoundaries(const Dimension& d) const
    {
        return (myBoundariesArray[d])->GetNumBoundaries();
    }

    inline const double Scoreboard::GetThickness(const Dimension& d) const
    {
        return (myBoundariesArray[d])->GetOverallThickness();
    }


    inline void Scoreboard::GetThickness(double& x, double& y, double& z) const
    {
        x = (myBoundariesArray[X])->GetOverallThickness();
        y = (myBoundariesArray[Y])->GetOverallThickness();
        z = (myBoundariesArray[Z])->GetOverallThickness();
    }

    inline const double Scoreboard::GetTop(const Dimension& d) const
    {
        return ((myBoundariesArray[d])->GetTop());
    }

    inline const double Scoreboard::GetBottom(const Dimension& d) const
    {
        return ((myBoundariesArray[d])->GetBottom());
    }

    inline void Scoreboard::FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate)
    {
        //long int stratumval = myScoreboardStratum.value(); // MSA 09.10.07 Unused
        box_coordinate->x = myBoundaries_X.FindLayerWith(soil_coordinate->x);
        box_coordinate->y = myBoundaries_Y.FindLayerWith(soil_coordinate->y);
        box_coordinate->z = myBoundaries_Z.FindLayerWith(soil_coordinate->z);
    }

    inline const BoxIndex Scoreboard::FindBoxBySoilCoordinate(const DoubleCoordinate* soil_coordinate) const
    {
        return GetBoxIndex(myBoundaries_X.FindLayerWith(soil_coordinate->x), myBoundaries_Y.FindLayerWith(soil_coordinate->y), myBoundaries_Z.FindLayerWith(soil_coordinate->z));
    }

    inline void Scoreboard::TransferAndClearCharacteristicValue(const CharacteristicIndex& from_index, const CharacteristicIndex& to_index, const BoxIndex& box)
    {
        AddCharacteristicValue(to_index, GetCharacteristicValue(from_index, box), box);
        SetCharacteristicValue(from_index, 0.0, box);
    }

    /* --××--××--××--××--××--××--××--××--××--××--××--××--××--××--
    FillWithValue
    Places the prescribed value at the requested offset in ALL of the boxes
    being managed by this object.
    --××--××--××--××--××--××--××--××--××--××--××--××--××--××-- */
    inline void Scoreboard::FillCharacteristicWithValue(const CharacteristicIndex& characteristic_index, const double& value)
    {
        //wxLogMessage(wxT("FillCharacteristicWithValue index %d value %f"), characteristic_index, value);

        // MSA 09.10.12 Optimisation attempt: rewriting this function to use STL fill
        std::fill(myCharacteristicValues[characteristic_index], myCharacteristicValues[characteristic_index] + myNumberOfBoxes, value);

        /*
        const BoxIndex box_max = GetNumberOfBoxes();
        for ( const BoxIndex & box_index = 0 ;
              box_index < box_max ;
              box_index++
            )
        {
            SetCharacteristicValue(characteristic_index, value, box_index);
        }
        */
    }

    inline const double Scoreboard::GetCoincidentProportion(const BoxIndex& boxIndex, const size_t& volumeObjectIndex)
    {
        if (m_coincidentProportions.empty())
        {
            m_coincidentProportions.resize(myNumberOfBoxes * (myVOCount + 1));
            std::fill(m_coincidentProportions.begin(), m_coincidentProportions.end(), -1);
        }
        const double possibleResult = m_coincidentProportions[boxIndex * (myVOCount + 1) + volumeObjectIndex];
        if (possibleResult < 0) return RetrieveCoincidentProportion(boxIndex, volumeObjectIndex); // Has not yet been retrieved; do so now
        return possibleResult;
    }

    inline void Scoreboard::IncrementVOCount()
    {
        ++myVOCount;
    }

    inline const TransferRestrictionData& Scoreboard::GetTransferRestrictionData(const BoxIndex& fromBoxIndex, const BoxIndex& toBoxIndex, const TransferRestrictedDimension& obstructedDimension)
    {
        if (m_VOSBIMap.empty()) return myDefaultTRD; // If there are no VolumeObjects, skip out early.

        std::map<std::pair<BoxIndex, BoxIndex>, TransferRestrictionData>::const_iterator iter = m_transferRestrictionMap.find(std::pair<BoxIndex, BoxIndex>(fromBoxIndex, toBoxIndex));
        if (iter == m_transferRestrictionMap.end()) return CalculateTransferRestrictionData(fromBoxIndex, toBoxIndex, obstructedDimension);

        return iter->second;
    }

    inline const TransferRestrictedDimension Scoreboard::ReverseDirection(const TransferRestrictedDimension& original) const
    {
        const int temp = static_cast<int>(original);
        return static_cast<TransferRestrictedDimension>((temp % 2 == 0) ? (temp + 1) : (temp - 1));
    }
} /* namespace rootmap */

#endif // #ifndef Scoreboard_H
