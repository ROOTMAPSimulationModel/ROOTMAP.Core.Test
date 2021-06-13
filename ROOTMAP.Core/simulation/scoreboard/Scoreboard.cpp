/////////////////////////////////////////////////////////////////////////////
// Name:        Scoreboard.cpp
// Purpose:     Implementation of the Scoreboard class
// Created:     31-07-2002
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/scoreboard/Scoreboard.h"

#include "core/common/Constants.h"
#include "core/common/Structures.h"
#include "core/common/RmAssert.h"
#include "core/common/Exceptions.h"
#include "core/utility/Utility.h"
#include "core/scoreboard/BoundaryArray.h"

///#include "ScoreboardCollaboration.h"

#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/ScoreboardAttributeSupplier.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "simulation/scoreboard/ScoreboardCollaboration.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/modules/VolumeObject.h"

#include <limits>
#include <cstdlib>
#include <algorithm>


// this static variable needs to be defined while debugging in general,
// not just while debugging AddCharacteristicLength(), because i
// want to know WHENEVER a length goes through 3 dimensions.
#if defined _DEBUG
static Boolean debug_3_dimensions = true;
#endif

namespace rootmap
{
    RootMapLoggerDefinition(Scoreboard);

    /* --××--××--××--××--××--××--××--××--××--××--××--××--××--××--
    Scoreboard
    Constructor with all necessary arguments
    --××--××--××--××--××--××--××--××--××--××--××--××--××--××-- */
    Scoreboard::Scoreboard
    (const ScoreboardStratum& ss,
        const BoundaryArray& ba_x,
        const BoundaryArray& ba_y,
        const BoundaryArray& ba_z)
        : CCollaborator()
        , myScoreboardStratum(ss)
        , myBoundaries_X(ba_x)
        , myBoundaries_Y(ba_y)
        , myBoundaries_Z(ba_z)
        , myNumberOfLayers_X(ba_x.GetNumLayers())
        , myNumberOfLayers_Y(ba_y.GetNumLayers())
        , myNumberOfLayers_Z(ba_z.GetNumLayers())
        , myNumberOfBoxes(0)
        , myVOCount(0)
        , myBoxVolumeCharacteristicIndex(-1)
        , myBoxVolumeCharacteristicDescriptor(0)
        //, myCharacteristicValues(0)
        //, mySpecialValues(0)
        //, myCharacteristicDescriptors(0)
    {
        RootMapLoggerInitialisation("rootmap.Scoreboard");

        myNumberOfLayersArray[X] = myNumberOfLayers_X;
        myNumberOfLayersArray[Y] = myNumberOfLayers_Y;
        myNumberOfLayersArray[Z] = myNumberOfLayers_Z;

        myBoundariesArray[X] = &myBoundaries_X;
        myBoundariesArray[Y] = &myBoundaries_Y;
        myBoundariesArray[Z] = &myBoundaries_Z;

        myNumberOfBoxes = myNumberOfLayers_X * myNumberOfLayers_Y * myNumberOfLayers_Z;

        myBoxVolumeCharacteristicDescriptor = new CharacteristicDescriptor;
        myBoxVolumeCharacteristicDescriptor->id = -1;
        // This gets set in RegisterCharacteristic()
        //myBoxVolumeCharacteristicDescriptor->ScoreboardIndex = ;
        myBoxVolumeCharacteristicDescriptor->Minimum = 0.0;
        myBoxVolumeCharacteristicDescriptor->Maximum = Utility::infinity();
        myBoxVolumeCharacteristicDescriptor->Default = 0.0;
        myBoxVolumeCharacteristicDescriptor->Name = sBoxVolumeCharacteristicName;
        myBoxVolumeCharacteristicDescriptor->Units = "cm^3";
        myBoxVolumeCharacteristicDescriptor->Visible = false;
        myBoxVolumeCharacteristicDescriptor->Edittable = false;
        myBoxVolumeCharacteristicDescriptor->Savable = false;
        myBoxVolumeCharacteristicDescriptor->SpecialPerBoxInfo = false;
        myBoxVolumeCharacteristicDescriptor->m_ScoreboardStratum = ss;

        RegisterCharacteristic(myBoxVolumeCharacteristicDescriptor);
        myBoxVolumeCharacteristicIndex = myBoxVolumeCharacteristicDescriptor->ScoreboardIndex;

        myDefaultTRD.unrestrictedProportionFrom = 1.0;
        myDefaultTRD.restrictedProportionFrom = 0.0;
        myDefaultTRD.unrestrictedProportionTo = 1.0;
        myDefaultTRD.restrictedProportionTo = 0.0;
        myDefaultTRD.voFromPermeability = 1.0;
        myDefaultTRD.voToPermeability = 1.0;
        myDefaultTRD.volumeObjectFrom = __nullptr;
        myDefaultTRD.volumeObjectTo = __nullptr;
        myDefaultTRD.backgroundVolumeObjectFrom = __nullptr;
        myDefaultTRD.backgroundVolumeObjectTo = __nullptr;
        myDefaultTRD.unrestrictedTransferWithinVO = false;

        ScoreboardAttributeSupplier* s1 = new ScoreboardAttributeSupplier(this);
        SharedAttributeRegistrar::RegisterSupplier(s1);
    }


    //
    // Function:
    //  ~Scoreboard
    //
    //
    // Description:
    //  Dispose of all the BoundaryArrays, CharacteristicValueArrays and SpecialValueArrays
    //
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    Scoreboard::~Scoreboard()
    {
        //
        // Dispose of all the CharacteristicValueArrays
        for (std::vector<CharacteristicValueArray>::iterator cva_iterator = myCharacteristicValues.begin();
            cva_iterator != myCharacteristicValues.end();
            ++cva_iterator)
        {
            CharacteristicValueArray cva = *cva_iterator;
            delete[] cva;
        }


        //
        // Dispose of all the SpecialValueArrays
        for (std::vector<SpecialScoreboardDataArray>::iterator sva_iterator = mySpecialValues.begin();
            sva_iterator != mySpecialValues.end();
            ++sva_iterator)
        {
            SpecialScoreboardDataArray sva = *sva_iterator;
            // TODO: theoretically speaking, there should be a SpecialScoreboardDataPtr for each
            // scoreboard box which will also need delete'ing first...
            delete[] sva;
        }

        delete myBoxVolumeCharacteristicDescriptor;
    }

    void Scoreboard::ProviderChanged(CCollaborator* /* aProvider */, long reason, CCollaboratorInfo* info)
    {
        if (scoreboardEdittedCell == reason)
        {
            EditCellInfo* eci = dynamic_cast<EditCellInfo *>(info);

            RmAssert(0 != eci, "CollaboratorInfo not of type EditCellInfo");

            SetCharacteristicValue(eci->characteristicIndex, eci->value, GetBoxIndex(eci->box));

            // pass the change onto other dependent ScoreboardTableHelpers
            BroadcastChange(reason, info);
        }
    }

    //
    //
    // Function:
    //  GetBoxIndexWrapped
    //
    // Description:
    //  "Wrap"s the scoreboard box coordinates.  That is to say, it brings the
    //  coordinates back "into" the scoreboard bounds if they are "outside".
    //
    // Working Notes:
    //  Used almost exclusively by AddCharacteristicLength(), for when roots or
    //  what-have-you are growing way out of range.
    //
    const BoxIndex Scoreboard::GetBoxIndexWrapped(BoxCoordinateIndex x, BoxCoordinateIndex y, BoxCoordinateIndex z) const
    {
        if (x > myNumberOfLayers_X)
        {
            x %= myNumberOfLayers_X; // modulus 
        }
        else
        {
            while (x < 1)
            {
                x += myNumberOfLayers_X;
            }
        }

        if (y > myNumberOfLayers_Y)
        {
            y %= myNumberOfLayers_Y; // modulus 
        }
        else
        {
            while (y < 1)
            {
                y += myNumberOfLayers_Y;
            }
        }

        return (GetBoxIndex(x, y, z));
    }

    const bool Scoreboard::GetDirectionBetweenColinearBoxes(const BoxIndex& fromBox, const BoxIndex& toBox, Dimension& plane, bool& inPositiveDirection) const
    {
        if (fromBox == toBox) return false;

        BoxCoordinate bcf, bct;
        GetXYZ(fromBox, bcf);
        GetXYZ(toBox, bct);

        // To be colinear, boxes must be coplanar in two Dimensions.
        if (bcf.x == bct.x)
        {
            if (bcf.y == bct.y)
            {
                plane = Z;
                inPositiveDirection = bcf.z < bct.z;
                return true;
            }
            else if (bcf.z == bct.z)
            {
                plane = Y;
                inPositiveDirection = bcf.y < bct.y;
                return true;
            }
        }

        else if (bcf.y == bct.y && bcf.z == bct.z)
        {
            plane = X;
            inPositiveDirection = bcf.x < bct.x;
            return true;
        }

        return false;
    }

    // MSA 11.01.24 Modified this method to initially zero out the parameter wrap_direction.
    // This is because an uninitialised WrapDirection variable is often used as the parameter,
    // and the result is often an invalid WrapDirection.
    void Scoreboard::FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate, WrapDirection* wrap_direction)
    {
        *wrap_direction = wraporderNone;
        WrapDirection temp_wrap = wraporderNone;

        box_coordinate->x = myBoundaries_X.FindLayerWith(soil_coordinate->x, &temp_wrap);
        *wrap_direction += temp_wrap & wraporderX;

        box_coordinate->y = myBoundaries_Y.FindLayerWith(soil_coordinate->y, &temp_wrap);
        *wrap_direction += temp_wrap & wraporderY;

        box_coordinate->z = myBoundaries_Z.FindLayerWith(soil_coordinate->z);
    }


    //
    // Function:
    //  SetCharacteristicPoint
    //
    // Description:
    //  Set the value of a characteristic value CONCENTRATED AT A POINT to the
    //  value of a box. This version of the function is useful when a process
    //  has already found the index of the box in which the Point is to be summarised
    //  coordinate for processes that have already needed to
    //  find the coordinate of the box in which the box coordinate has already
    //  needed to be found
    //
    //
    void Scoreboard::SetCharacteristicPoint
    (const CharacteristicIndex& characteristic_index,
        const double& value,
        BoxCoordinate& box_coordinate)
    {
        // Get the box with the summary values
        const BoxIndex& box_index = GetBoxIndex(box_coordinate.x, box_coordinate.y, box_coordinate.z);
        // Find out how much volume (real soil) it takes up
        double box_volume = GetBoxVolume(box_index);
        // calculate the density of the value
        double average = value / box_volume;
        // set the value in the box
        SetCharacteristicValue(characteristic_index, average, box_index);
    }

    void Scoreboard::SetCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, const double& x, const double& y, const double& z)
    {
        // for passing the point in the soil to the ScoreboardCoordinator's routine
        DoubleCoordinate soil_coordinate(x, y, z);
        // the coordinate of the box in which the soil point will be found
        BoxCoordinate box_coordinate;

        // Find out which box the point is in
        FindBoxBySoilCoordinate(&box_coordinate, &soil_coordinate);
        // pass on to other function to do the work. This is the idea of encapsulation,
        // so that if the real stuff gets changed, it only needs be changed in one function.
        SetCharacteristicPoint(characteristic_index, value, box_coordinate);
    }

    void Scoreboard::SetPlaneCharacteristicValues(const CharacteristicIndex& characteristic_index, Dimension dimension, long plane_index, const double& value)
    {
        BoxCoordinateIndex x_max, y_max, z_max;
        GetNumLayers(x_max, y_max, z_max);
        BoxIndex box_index = InvalidBoxIndex;

        switch (dimension)
        {
        case X:
        {
            LOG_DEBUG << "Setting boxes [" << plane_index << ",*,*] with value " << value;
            for (BoxCoordinateIndex y_index = 1; y_index <= y_max; ++y_index)
            {
                for (BoxCoordinateIndex z_index = 1; z_index <= z_max; ++z_index)
                {
                    box_index = GetBoxIndex(plane_index, y_index, z_index);
                    RmAssert(box_index < myNumberOfBoxes, "Box index exceeds number of boxes");
                    SetCharacteristicValue(characteristic_index, value, box_index);
                }
            }
        }
        break;
        case Y:
        {
            LOG_DEBUG << "Setting boxes [*," << plane_index << ",*] with value " << value;
            for (BoxCoordinateIndex x_index = 1; x_index <= x_max; ++x_index)
            {
                for (BoxCoordinateIndex z_index = 1; z_index <= z_max; ++z_index)
                {
                    box_index = GetBoxIndex(x_index, plane_index, z_index);
                    RmAssert(box_index < myNumberOfBoxes, "Box index exceeds number of boxes");
                    SetCharacteristicValue(characteristic_index, value, box_index);
                }
            }
        }
        break;
        case Z:
        {
            LOG_DEBUG << "Setting boxes [*,*," << plane_index << "] with value " << value;
            for (BoxCoordinateIndex x_index = 1; x_index <= x_max; ++x_index)
            {
                for (BoxCoordinateIndex y_index = 1; y_index <= y_max; ++y_index)
                {
                    box_index = GetBoxIndex(x_index, y_index, plane_index);
                    RmAssert(box_index < myNumberOfBoxes, "Box index exceeds number of boxes");
                    SetCharacteristicValue(characteristic_index, value, box_index);
                }
            }
        }
        break;
        default:
            throw (new RmException("Cannot b sirius, u has a invalid dimension"));
            break;
        }
    }

    void Scoreboard::SetRowColumnCharacteristicValues(const CharacteristicIndex& characteristic_index, BoxCoordinateIndex x_index, BoxCoordinateIndex y_index, BoxCoordinateIndex z_index, const double& value)
    {
        BoxCoordinateIndex x_max, y_max, z_max;
        GetNumLayers(x_max, y_max, z_max);

        if (-1 == x_index)
        {
            LOG_DEBUG << "Setting boxes [*," << y_index << "," << z_index << "] with value " << value;
            for (long iter_index = 1; iter_index <= x_max; ++iter_index)
            {
                SetCharacteristicValue(characteristic_index, value, GetBoxIndex(iter_index, y_index, z_index));
            }
        }
        else if (-1 == y_index)
        {
            LOG_DEBUG << "Setting boxes [" << x_index << ",*," << z_index << "] with value " << value;
            for (long iter_index = 1; iter_index <= y_max; ++iter_index)
            {
                SetCharacteristicValue(characteristic_index, value, GetBoxIndex(x_index, iter_index, z_index));
            }
        }
        else if (-1 == z_index)
        {
            LOG_DEBUG << "Setting boxes [" << x_index << "," << y_index << ",*] with value " << value;
            for (long iter_index = 1; iter_index <= z_max; ++iter_index)
            {
                SetCharacteristicValue(characteristic_index, value, GetBoxIndex(x_index, y_index, iter_index));
            }
        }
    }

    void Scoreboard::SetArbitraryBoxesCharacteristicValues(const CharacteristicIndex& characteristic_index, BoxCoordinate& bc1, BoxCoordinate& bc2, const double& value)
    {
        BoxCoordinateIndex x_max = bc2.x;
        BoxCoordinateIndex y_max = bc2.y;
        BoxCoordinateIndex z_max = bc2.z;

        LOG_DEBUG << "Setting boxes [" << bc1.x << "," << bc1.y << "," << bc1.z << "] to [" << bc2.x << "," << bc2.y << "," << bc2.z << "] with value " << value;

        for (BoxCoordinateIndex x_index = bc1.x; x_index <= x_max; ++x_index)
        {
            for (BoxCoordinateIndex y_index = bc1.y; y_index <= y_max; ++y_index)
            {
                for (BoxCoordinateIndex z_index = bc1.z; z_index <= z_max; ++z_index)
                {
                    SetCharacteristicValue(characteristic_index, value, GetBoxIndex(x_index, y_index, z_index));
                }
            }
        }
    }


    // AddCharacteristicPoint
    //
    // Add the value of a characteristic value CONCENTRATED AT A POINT to the value of a box.
    // This is good for processes that have already needed to find the coordinate of the box in which
    // the box coordinate has already needed to be found
    void Scoreboard::AddCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, BoxCoordinate& box_coordinate)
    {
        /*****
        **    new_average = ((old_average * box_volume) + value) / box_volume
        *****/
        // Get the box with the summary values
        const BoxIndex& box_index = GetBoxIndex(box_coordinate.x, box_coordinate.y, box_coordinate.z);
        // Find out how much volume (real soil) it takes up
        double box_volume = GetBoxVolume(box_index);
        // Get the characteristic from it
        double average = GetCharacteristicValue(characteristic_index, box_index);
        // calculate the overall density of the characteristic in that box
        average = ((average * box_volume) + value) / box_volume;
        // set the value in the box
        SetCharacteristicValue(characteristic_index, average, box_index);
    }

    void Scoreboard::AddCharacteristicPoint(const CharacteristicIndex& characteristic_index, const double& value, const double& x, const double& y, const double& z)
    {
        // for passing the point in the soil to the ScoreboardCoordinator's routine
        DoubleCoordinate soil_coordinate(x, y, z);
        // the coordinate of the box in which the soil point will be found
        BoxCoordinate box_coordinate;

        // Find out which box the point is in
        FindBoxBySoilCoordinate(&box_coordinate, &soil_coordinate);
        // pass on to other function to do the work. This is the idea of encapsulation,
        // so that if the real stuff gets changed, it only needs be changed in one function.
        AddCharacteristicPoint(characteristic_index, value, box_coordinate);
    }

    //  ******
    //    Function:
    //      AddCharacteristicLength()
    //
    //  Description:
    //      Adds the value of the length to the boxes through which it travels.
    //
    //  Working Notes:
    //        This function caters for the possibility that an additional length
    //      might span over any number of boxes in any direction. It is
    //      implemented with the realisation that the possibility is very rare,
    //      however when it IS needed, it needs to work properly. When i say very
    //      rare, of course, that depends on the size of boxes in the scoreboard.
    //      If boxes are very small, the more complicated functionality gets more
    //      use
    //
    //    Parameters:
    //      characteristic_index
    //                      :   the index of the characteristic to which to add
    //                          the length
    //        value           :   the length to add to the appropriate boxes
    //        start_box_coord :   the box coordinates of the box with the start point
    //        start_box       :   the box described ^
    //        end_point       :   the soil coordinates of the end point of the line/length
    //
    //    Returns    :
    //        BoxIndex        :   the index of the new point's box.
    //    
    // ******
#define GET_X_BOUNDARY_LEFT    (myBoundaries_X.GetLayerTop(start_box_coord_x)+x_padding_start)
#define GET_X_BOUNDARY_RIGHT    (myBoundaries_X.GetLayerBottom(start_box_coord_x)+x_padding_start)
#define GET_Y_BOUNDARY_FRONT    (myBoundaries_Y.GetLayerTop(start_box_coord_y)+y_padding_start)
#define GET_Y_BOUNDARY_BACK    (myBoundaries_Y.GetLayerBottom(start_box_coord_y)+y_padding_start)
#define GET_Z_BOUNDARY_TOP        (myBoundaries_Z.GetLayerTop(start_box_coord_z))
#define GET_Z_BOUNDARY_BOTTOM    (myBoundaries_Z.GetLayerBottom(start_box_coord_z))

    // the "cp" parameter stands for "coordinate pointer"
#define GET_NEXT_BOX_LEFT(box, cp)        GetBoxIndexWrapped(cp->x-1, cp->y, cp->z)
#define GET_NEXT_BOX_RIGHT(box, cp)        GetBoxIndexWrapped(cp->x+1, cp->y, cp->z)
#define GET_NEXT_BOX_FRONT(box, cp)        GetBoxIndexWrapped(cp->x, cp->y-1, cp->z)
#define GET_NEXT_BOX_BACK(box, cp)        GetBoxIndexWrapped(cp->x, cp->y+1, cp->z)
#define GET_NEXT_BOX_TOP(box, cp)        GetBoxIndexWrapped(cp->x, cp->y, cp->z-1)
#define GET_NEXT_BOX_BOTTOM(box, cp)    GetBoxIndexWrapped(cp->x, cp->y, cp->z+1)

#ifdef _CS_DEBUG_ADDCHARACTERISTICLENGTH_
#define LENGTH_FAILURE(value) ((value<0) || (value>2.0) || (isnan(value)) || (!isfinite(value)))
#endif
    BoxIndex Scoreboard::AddCharacteristicLength
    (const CharacteristicIndex& characteristic_index,
        double value,
        const DoubleCoordinate* start_point,
        const BoxCoordinate* start_box_coord,
        const BoxIndex& start_box,
        const size_t volumeObjectIndex,
        const DoubleCoordinate* end_point,
        const bool& do_density)
    {
        const double start_point_x = start_point->x;
        const double start_point_y = start_point->y;
        const double start_point_z = start_point->z;
        const double end_point_x = end_point->x;
        const double end_point_y = end_point->y;
        const double end_point_z = end_point->z;

        const long start_box_coord_x = start_box_coord->x;
        const long start_box_coord_y = start_box_coord->y;
        const long start_box_coord_z = start_box_coord->z;
        long end_box_coord_x = start_box_coord_x;
        long end_box_coord_y = start_box_coord_y;
        long end_box_coord_z = start_box_coord_z;

        double x_padding_start = 0.0;
        double x_padding_end = 0.0;
        double y_padding_start = 0.0;
        double y_padding_end = 0.0;
        bool x_is_different = false;
        bool y_is_different = false;
        bool z_is_different = false;

        BoxIndex end_box = start_box;

        long int number_of_interscoreboard_crossings = 0;

        /*****
        * Begin by pre-padding the test widths so that if the start_point was
        * already "wrap"ped, we don't think that the end_point is in a different
        * box just because we had to wrap around to find that box.
        *****/
        while (start_point_x > myBoundaries_X.GetBottom() + x_padding_start)
        {
            x_padding_end = (x_padding_start += myBoundaries_X.GetOverallThickness());
        }
        while (start_point_x < myBoundaries_X.GetTop() + x_padding_start)
        {
            x_padding_end = (x_padding_start -= myBoundaries_X.GetOverallThickness());
        }

        while (start_point_y > myBoundaries_Y.GetBottom() + y_padding_start)
        {
            y_padding_end = (y_padding_start += myBoundaries_Y.GetOverallThickness());
        }
        while (start_point_y < myBoundaries_Y.GetTop() + y_padding_start)
        {
            y_padding_end = (y_padding_start -= myBoundaries_Y.GetOverallThickness());
        }

        //
        // The Z dimension does not wrap. So here, we check that EITHER of the
        // end points are in this scoreboard, by checking that the z coordinates
        // are within.
        //
        // This situation is currently relevant when plant roots grow below the
        // bottom of the bottom-most scoreboard.
        // 
        double z_bottom = myBoundaries_Z.GetBottom();
        double z_top = myBoundaries_Z.GetTop();

        if ((start_point_z > z_bottom) && (end_point_z > z_bottom))
        {
            // sbScoreboardCoordinator->GetScoreboardBelow(this)->AddCharacteristicLength(characteristic_index, value, start_point, start_box_coord, start_box, end_point, do_density);
            return (start_box);
        }

        if ((start_point_z < z_top) && (end_point_z < z_top))
        {
            // sbScoreboardCoordinator->GetScoreboardAbove(this)->AddCharacteristicLength(characteristic_index, value, start_point, start_box_coord, start_box, end_point, do_density);
            return (start_box);
        }

        //
        //  This next section ~looks~ pretty meaty, although unintentionally. If
        //  the length "wrap"s, some special stuff is required to make it look
        //  like the end point is still mathematically on the same side (merely
        //  outside) of the scoreboard.
        //
        while (end_point_x > (myBoundaries_X.GetLayerBottom(end_box_coord_x) + x_padding_end))
        {
            ++end_box_coord_x;
            x_is_different = true;

            if (end_box_coord_x > myNumberOfLayers_X)
            {
                end_box_coord_x = 1;
                x_padding_end += myBoundaries_X.GetOverallThickness();
            }
        }
        while (end_point_x < (myBoundaries_X.GetLayerTop(end_box_coord_x) + x_padding_end))
        {
            --end_box_coord_x;
            x_is_different = true;

            if (end_box_coord_x < 1)
            {
                end_box_coord_x = myNumberOfLayers_X;
                x_padding_end -= myBoundaries_X.GetOverallThickness();
            }
        }

        while (end_point_y > (myBoundaries_Y.GetLayerBottom(end_box_coord_y) + y_padding_end))
        {
            ++end_box_coord_y;
            y_is_different = true;

            if (end_box_coord_y > myNumberOfLayers_Y)
            {
                end_box_coord_y = 1;
                y_padding_end += myBoundaries_Y.GetOverallThickness();
            }
        }
        while (end_point_y < (myBoundaries_Y.GetLayerTop(end_box_coord_y) + y_padding_end))
        {
            --end_box_coord_y;
            y_is_different = true;

            if (end_box_coord_y < 1)
            {
                end_box_coord_y = myNumberOfLayers_Y;
                y_padding_end -= myBoundaries_Y.GetOverallThickness();
            }
        }

        while ((end_box_coord_z <= myNumberOfLayers_Z) &&
            (end_point_z > myBoundaries_Z.GetLayerBottom(end_box_coord_z))
            )
        {
            ++end_box_coord_z;
            z_is_different = true;
        }
        while ((end_box_coord_z >= 1) &&
            (end_point_z < myBoundaries_Z.GetLayerTop(end_box_coord_z))
            )
        {
            --end_box_coord_z;
            z_is_different = true;
        }

        if ((end_box_coord_z < 1) || (end_box_coord_z > myNumberOfLayers_Z))
        {
            LOG_WARN << LOG_LINE << "Invalid End BoxCoordinate Z value {end_box_coord_z:" << end_box_coord_z
                << ", myNumberOfLayers_Z:" << myNumberOfLayers_Z << ", end_point_z:" << end_point_z << "}";
            return start_box;
        }

        end_box = GetBoxIndex(end_box_coord_x, end_box_coord_y, end_box_coord_z);

        //
        // DEBUG
        //
        // That's right, this is actually debug code. Not #ifdef'd out !
        //
        if (((x_padding_start != x_padding_end) || (y_padding_start != y_padding_end)))
        {
            ++number_of_interscoreboard_crossings;
        }


        // MSA 11.06.24 Roots can now penetrate VO surfaces. HOWEVER!
        // Root segments are broken into per-VO subsegments by the root tip growth code,
        // so each root segment passed to this function should be entirely within a single VO.
        const double startBoxVolumeProportion = GetCoincidentProportion(start_box, volumeObjectIndex);
        const double endBoxVolumeProportion = GetCoincidentProportion(end_box, volumeObjectIndex);

        //RmAssert(startBoxVolumeProportion>0.0,"Error: This volume is logically zero, cannot add Characteristic length.");
        //RmAssert(endBoxVolumeProportion>0.0,"Error: This volume is logically zero, cannot add Characteristic length.");

        // MSA TODO 11.01.24 Update this method to work in terms of VolumeObject-wise spatial subsections. Argh.
        // NOTE: Not critical at the moment, as long as VolumeObject surfaces are impassable barriers - 
        // the root deflection code will ensure that roots do not cross VO surfaces.

        // MSA 11.06.24 Roots can now penetrate VO surfaces. HOWEVER!
        // Root segments are broken into per-VO subsegments by the root tip growth code,
        // so each root segmet passed to this function should be entirely within a single VO.

        if (end_box == start_box)
        {
            //
            // This is cinch.  Add the entire new length to the initial box.
            //
            if (do_density) value /= (startBoxVolumeProportion * GetBoxVolume(start_box));
            if (startBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, value, start_box);
            //LOG_DEBUG << "Added value " << value << " to CharacteristicIndex " << characteristic_index << " in box " << start_box << " (do_density=" << do_density << ")";
        }
        else
            /*******
            *    If the end point is in a different box to the start point, we need to
            *    partition the length. between the intervening boxes.
            *    Once again, the implementation works best (is optimised) for the
            *    most common situation - where the end box is adjacent to the start box
            *
            *    If the end point is not in one of the 6 directly adjacent boxes, things
            *    become much trickier, since the "length" will almost certainly
            *    pass through a third, and possibly fourth, box.
            *
            *    NOTE that once i take into consideration that a length might
            *    traverse an entire box and come out the other side, this could
            *    become exponentially more complicated.
            *******/
        {
            double x_boundary = 0, y_boundary = 0, z_boundary = 0;
            double v_same = 0.0, v_next1 = 0.0, v_end = 0.0; //, v_next2=0.0; // MSA 09.10.07 Unused
            long number_of_differing_dimensions = (x_is_different ? 1 : 0) + (y_is_different ? 1 : 0) + (z_is_different ? 1 : 0);

            //
            //  Start by precalculating the boundaries that the "length" crosses.
            //  This is stuff that is required for all calculations that follow
            //
            if (x_is_different)
            {
                if (end_point_x < start_point_x)
                {
                    x_boundary = GET_X_BOUNDARY_LEFT;
                }
                else // if (end_point_x > start_point_x)
                {
                    x_boundary = GET_X_BOUNDARY_RIGHT;
                }
                // else the x coordinates should be equal
            }

            if (y_is_different)
            {
                if (end_point_y < start_point_y)
                {
                    y_boundary = GET_Y_BOUNDARY_FRONT;
                }
                else // if (end_point_y > start_point_y)
                {
                    y_boundary = GET_Y_BOUNDARY_BACK;
                }
                // else the y coordinates should be equal
            }


            if (z_is_different)
            {
                if (end_point_z < start_point_z)
                {
                    z_boundary = GET_Z_BOUNDARY_TOP;
                }
                else // if (end_point_z > start_point_z)
                {
                    z_boundary = GET_Z_BOUNDARY_BOTTOM;
                }
                // else the z coordinates should be equal
            }


            // #pragma mark ("1 differeng dimension")
            /*****
            *    Here we deal with the case for a directly adjacent box
            *****/
            if (number_of_differing_dimensions == 1)
            {
                double the_same = 0.0, the_next1 = 0.0;

                if (x_is_different)
                {
                    the_same = fabs(start_point_x - x_boundary);
                    the_next1 = fabs(x_boundary - end_point_x);
                } // if (x_is_different)
                else if (y_is_different)
                {
                    the_same = fabs(y_boundary - start_point_y);
                    the_next1 = fabs(end_point_y - y_boundary);
                } // if (y_is_different)
                else if (z_is_different)
                {
                    the_same = fabs(z_boundary - start_point_z);
                    the_next1 = fabs(end_point_z - z_boundary);
                } // if (z_is_different)

                v_same = value * (the_same / (the_same + the_next1));

                if (do_density)
                {
                    if (startBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_same / (startBoxVolumeProportion * GetBoxVolume(start_box)), start_box);
                    if (endBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, (value - v_same) / (endBoxVolumeProportion * GetBoxVolume(end_box)), end_box);
                }
                else
                {
                    if (startBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_same, start_box);
                    if (endBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, value - v_same, end_box);
                }

                //LOG_DEBUG << "Added values [" << v_same << "," << (value-v_same) << "] to CharacteristicIndex " << characteristic_index << " in boxes [" << start_box << "," << end_box << "] (do_density=" << do_density << ")";
            } // if (number_of_differing_dimensions == 1)
            // #pragma mark 2 differeng dimensions
            /*****
            *    And here, with the case for a diagonally adjacent box
            *****/
            else if (number_of_differing_dimensions == 2)
            {
                //
                // There are 3 planes in which the length could be pointing.
                // For each plane, there are 4 diagonal boxes into which the length
                // could have gone. For each of the boxes, there are 2 possible
                // intermediate boxes through which it might have gone.
                // So 3 x 4 x 2 = 24 possibilities to check
                //
                BoxIndex middle_box = -1;
                double x0 = 0, x1 = 0, x2 = 0, z0 = 0, z1 = 0, z2 = 0;
                double x_closer, x_further;
                Boolean positive_vector;

                if (!y_is_different) // then x and z ARE different
                {
                    x0 = x_boundary;
                    x1 = start_point_x;
                    x2 = end_point_x;
                    z0 = z_boundary;
                    z1 = start_point_z;
                    z2 = end_point_z;
                } // if (!y_is_different)
                else if (!x_is_different) // then y and z ARE different
                {
                    x0 = y_boundary;
                    x1 = start_point_y;
                    x2 = end_point_y;
                    z0 = z_boundary;
                    z1 = start_point_z;
                    z2 = end_point_z;
                } // if (!x_is_different)
                else if (!z_is_different) // then x and y ARE different
                {
                    x0 = x_boundary;
                    x1 = start_point_x;
                    x2 = end_point_x;
                    z0 = y_boundary;
                    z1 = start_point_y;
                    z2 = end_point_y;
                } // if (!z_is_different)

                const double x_at_z0 = ((z0 - z2) * (x1 - x2)) / (z1 - z2) + x2;

                if (fabs(x1 - x0) < fabs(x1 - x_at_z0))
                { // The x boundary is closer to the start point
                    x_closer = x0;
                    x_further = x_at_z0;

                    positive_vector = (x2 > x1);

                    if (!x_is_different)
                    {
                        middle_box = (positive_vector) ?
                            GET_NEXT_BOX_BACK(start_box, start_box_coord) :
                            GET_NEXT_BOX_FRONT(start_box, start_box_coord);
                    }
                    else
                        middle_box = (positive_vector) ?
                        GET_NEXT_BOX_RIGHT(start_box, start_box_coord) :
                        GET_NEXT_BOX_LEFT(start_box, start_box_coord);
                }
                else
                { // The z boundary is closer to the start point
                    x_closer = x_at_z0;
                    x_further = x0;

                    positive_vector = (z2 > z1);

                    if (!z_is_different)
                        middle_box = (positive_vector) ? GET_NEXT_BOX_BACK(start_box, start_box_coord) : GET_NEXT_BOX_FRONT(start_box, start_box_coord);
                    else
                        middle_box = (positive_vector) ? GET_NEXT_BOX_BOTTOM(start_box, start_box_coord) : GET_NEXT_BOX_TOP(start_box, start_box_coord);
                }

                // by proportions, allocate an amount of the value to each box
                v_same = value * ((x_closer - x1) / (x2 - x1));
                v_end = value * ((x2 - x_further) / (x2 - x1));
                v_next1 = value - v_same - v_end;


                const double middleBoxVolumeProportion = GetCoincidentProportion(middle_box, volumeObjectIndex);
                //RmAssert(middleBoxVolumeProportion>0.0,"Error: This volume is logically zero, cannot add Characteristic length.");

                /* now store those amounts to the relevant boxes
                For differences between length and density, the reason i don't
                just adjust the values before storing them is
                1. no lines of code are saved
                2. the cached values are preserved the way i HAVE done it. */
                if (do_density)
                {
                    if (v_same > 0.0) // Minor optimisation
                    {
                        if (startBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_same / (startBoxVolumeProportion * GetBoxVolume(start_box)), start_box);
                    }

                    if (v_next1 > 0.0)
                    {
                        // MSA Important note: This section is a little kludgy, and relies on VO surfaces being impassable to be correct.
                        // MSA 11.06.24 Roots can now penetrate VO surfaces. HOWEVER!
                        // Root segments are broken into per-VO subsegments by the root tip growth code,
                        // so each root segment passed to this function should be entirely within a single VO.
                        if (middleBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_next1 / (middleBoxVolumeProportion * GetBoxVolume(middle_box)), middle_box);
                    }

                    if (v_end > 0.0)
                    {
                        if (endBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_end / (endBoxVolumeProportion * GetBoxVolume(end_box)), end_box);
                    }
                }
                else
                {
                    if (startBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_same, start_box);
                    if (middleBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_next1, middle_box);
                    if (endBoxVolumeProportion > 0.0) AddCharacteristicValue(characteristic_index, v_end, end_box);
                }
            } // else if (number_of_differing_dimensions == 2)
            else // then (number_of_differing_dimensions == 3)
            {
                LOG_WARN << LOG_LINE << "3 Dimensions of change in AddCharacteristicLength not implemented "
                    << " {Start:[" << start_point_x << "," << start_point_y << "," << start_point_z << "]"
                    << ", {End:[" << end_point_x << "," << end_point_y << "," << end_point_z << "]}";
#if defined _CS_DEBUG_ADDCHARACTERISTICLENGTH_
                char * cstr = "3 dimensions of change in AddCharacteristicLength()\r";
                if (debug_3_dimensions)
                {
#ifdef _CS_DEBUG_WINDOW_
                    write_(cstr);
#else // no debug window open
                    DebugStr("3 dimensions of change in AddCharacteristicLength()\r");
#endif // #ifdef _CS_DEBUG_WINDOW_
                } // if (debug_3_dimensions)
#endif // #if defined _CS_DEBUG_ADDCHARACTERISTICLENGTH_
            } // else (number_of_differing_dimensions == 3)
        } // else (end_box != start_box)

        return (end_box);
    } // AddCharacteristicLength

    void Scoreboard::CalculateBoxVolumes()
    {
        long int x_max, y_max, z_max;
        long int x, y, z;
        double box_volume;

        GetNumLayers(x_max, y_max, z_max);

        for (z = 1; z <= z_max; ++z)
        {
            for (y = 1; y <= y_max; ++y)
            {
                for (x = 1; x <= x_max; ++x)
                {
#ifdef _CS_DEBUG_
                    double x_thickness = myBoundaries_X.GetLayerThickness(x);
                    double y_thickness = myBoundaries_Y.GetLayerThickness(y);
                    double z_thickness = myBoundaries_Z.GetLayerThickness(z);
                    box_volume = x_thickness * y_thickness * z_thickness;
#else
                    box_volume = myBoundaries_X.GetLayerThickness(x) * myBoundaries_Y.GetLayerThickness(y) * myBoundaries_Z.GetLayerThickness(z);
#endif // #ifdef _CS_DEBUG_
                    SetCharacteristicValue(myBoxVolumeCharacteristicIndex, box_volume, x, y, z);
                }
            }
        }
    }

    //
    // Function:
    //  RegisterCharacteristic
    //
    // Description:
    //  
    //  
    //  
    //  
    // Working Notes:
    //  Keeps a record of, but does not assume ownership of, the CharacteristicDescriptor.
    //  
    //  Allocates and owns a CharacteristicValueArray for the new Characteristic.
    //
    void Scoreboard::RegisterCharacteristic(CharacteristicDescriptor* cd)
    {
        // add to the list
        myCharacteristicDescriptors.push_back(cd);

        // 
        cd->ScoreboardIndex = myCharacteristicDescriptors.size() - 1;

        // allocate some memory to the stuff
        long int number_of_boxes = myNumberOfLayers_X * myNumberOfLayers_Y * myNumberOfLayers_Z;

        // 
        CharacteristicValueArray cva = new CharacteristicValue[number_of_boxes];
        myCharacteristicValues.push_back(cva);
    }


#if defined ROOTMAP_TODO_GUI
    //
    // Function:
    //  SetEditor
    //
    // Description:
    //  Sets up the designated TEditCell as a provider to this scoreboard. In that
    //  way, when the TEditCell decides that its value has changed, this object
    //  gets sent a ProviderChanged message (see below)
    //
    void Scoreboard::SetEditor(TEditCell *aProvider)
    {
        //    if (TCL_DYNAMIC_CAST(CCollaborator, aProvider))
        DependUpon(aProvider);
    }

    //
    // Function:
    //  ReleaseEditor
    //
    // Description:
    //  End the dependency of this scoreboard on the given edit cell
    //
    void Scoreboard::ReleaseEditor(TEditCell *aProvider)
    {
        //    if (TCL_DYNAMIC_CAST(CCollaborator, aProvider))
        CancelDependency(aProvider);
    }

    /* ProviderChanged
    Could be received for only one reason that this class is interested in */
    void Scoreboard::ProviderChanged(CCollaborator *aProvider,
        long reason,
        void *info)
    {
        if (reason == editcellEdittedCell)
        {
            // coerce to a usable type
            EditCellInfo* eci = (EditCellInfo*)info;

            // Set the value in the appropriate box
            SetCharacteristicValue(eci->ecCharacteristicIndex,
                eci->ecValue,
                eci->ecBox.x,
                eci->ecBox.y,
                eci->ecBox.z);

            // need to fill this in for the TEditCell
            eci->ecVT = myScoreboardStratum.value();

            // now tell all the tables and views
            BroadcastChange(scoreboardEdittedCell, info);
        }
        else
        {
            CCollaborator::ProviderChanged(aProvider, reason, info);
        }
    }

#endif // #if defined ROOTMAP_TODO_GUI

    /* --××--××--××--××--××--××--××--××--××--××--××--××--××--××--
    FillCustomInfoWithNULL
    Initializes the custom information pointers &/or Handles with NULL
    --××--××--××--××--××--××--××--××--××--××--××--××--××--××-- */
    void Scoreboard::FillSpecialDataWithNull(long characteristic_index)
    {
        const BoxIndex box_max = GetNumberOfBoxes();
        for (BoxIndex box_index = 0;
            box_index < box_max;
            ++box_index
            )
        {
            SetSpecialPointer(characteristic_index, box_index, 0);
        }
    }

    double Scoreboard::GetMean
    (int hi,
        int vi,
        ViewDirection viewdir,
        int characteristicindex)
    {
        Dimension dim = Y; // works for vFront
        if (vSide == viewdir)
        {
            dim = X;
        }
        else if (vTop == viewdir)
        {
            dim = Z;
        }
        const BoundaryArray& layers = GetBoundaryArray(dim);

        long int loopmax = layers.GetNumLayers();
        long int loopval;

        double mean;
        double cumulativeValue = 0.0;
        double depth = GetThickness(dim);

        double layer_thickness = 0.0;
        double characteristic_value = 0.0;

        switch (viewdir)
        {
        case vFront:
        {
            long int xval = hi;
            long int zval = vi;
            for (loopval = 1; loopval <= loopmax; ++loopval)
            {
                layer_thickness = layers.GetLayerThickness(loopval);
                characteristic_value = GetCharacteristicValue(characteristicindex, xval, loopval, zval);
                cumulativeValue += layer_thickness * characteristic_value;
            }
            break;
        }
        case vSide:
        {
            long int yval = hi;
            long int zval = vi;
            for (loopval = 1; loopval <= loopmax; ++loopval)
            {
                layer_thickness = layers.GetLayerThickness(loopval);
                characteristic_value = GetCharacteristicValue(characteristicindex, loopval, yval, zval);
                cumulativeValue += layer_thickness * characteristic_value;
            }
            break;
        }
        case vTop:
        {
            long int xval = hi;
            long int yval = vi;
            for (loopval = 1; loopval <= loopmax; ++loopval)
            {
                layer_thickness = layers.GetLayerThickness(loopval);
                characteristic_value = GetCharacteristicValue(characteristicindex, xval, yval, loopval);
                cumulativeValue += layer_thickness * characteristic_value;
            }
            break;
        }
        }

        mean = cumulativeValue / depth;
        return mean;
    }

#if !defined ROOTMAP_SCOREBOARD_GETCHARACTERISTICVALUE_INLINE
    double Scoreboard::GetCharacteristicValue(const CharacteristicIndex & characteristic_index, const BoxIndex & box_index) const
    {
        double return_value = Scoreboard::GetBoxValue(characteristic_index, box_index);
        wxLogMessage(wxT("Scoreboard::GetBoxValue(characteristic_index=%d, box_index=%d) returning %f"), characteristic_index, box_index, return_value);
        return (return_value);
    }
#endif // #if !defined ROOTMAP_SCOREBOARD_GETCHARACTERISTICVALUE_INLINE


    std::string Scoreboard::sBoxVolumeCharacteristicName = "Box Volume";
    std::string Scoreboard::sConfigName = "Scoreboard";


    class VOMatchingPredicate
    {
    private:
        VolumeObject* m_voptr;
    public:
        VOMatchingPredicate(const VolumeObjectScoreboardBoxInteraction& vosbi)
            : m_voptr(vosbi.volumeObject)
        {
        }

        inline const bool operator()(const VolumeObjectScoreboardBoxInteraction& rhs) const
        {
            return m_voptr == rhs.volumeObject;
        }
    };

    // MSA 11.04.27 Updated this method to cache its results, for faster overall access time
    // at a cost of approximately (sizeof(double) * myNumberOfBoxes * (myVOCount+1)) bytes.
    // Also wrapped it in an inline function call which will quickly return the cached result when available.
    const double Scoreboard::RetrieveCoincidentProportion(const BoxIndex& boxIndex, const size_t& volumeObjectIndex)
    {
        if (m_VOSBIMap.find(boxIndex) != m_VOSBIMap.end())
        {
            const VolumeObjectScoreboardBoxInteraction& vosbi = m_VOSBIMap[boxIndex];

            if (vosbi.volumeObject->GetIndex() == volumeObjectIndex)
            {
                m_coincidentProportions[boxIndex * (myVOCount + 1) + volumeObjectIndex] = vosbi.coincidentProportion;
                return vosbi.coincidentProportion;
            }
            else if ((vosbi.backgroundVolumeObject != __nullptr && vosbi.backgroundVolumeObject->GetIndex() == volumeObjectIndex)
                || (volumeObjectIndex == 0))
            {
                m_coincidentProportions[boxIndex * (myVOCount + 1) + volumeObjectIndex] = (1.0 - vosbi.coincidentProportion);
                return (1.0 - vosbi.coincidentProportion);
            }
        }
        // Else: No VolumeObject interaction.
        // Now, if there is no VO interaction, this box must be 100% coincident with volumeObjectIndex==0.
        // If there is any kind of interaction, it's either 
        //    a) 100% coincident with volumeObjectIndex!=0,
        //    b) <100% coincident with volumeObjectIndex!=0 AND the remaining amount coincident with volumeObjectIndex==0, or
        //    c) <100% coincident with volumeObjectIndex!=0 AND the remaining amount coincident with a DIFFERENT volumeObjectIndex!=0.
        m_coincidentProportions[boxIndex * (myVOCount + 1) + volumeObjectIndex] = (volumeObjectIndex == 0) ? 1.0 : 0.0;
        return (volumeObjectIndex == 0) ? 1.0 : 0.0;
    }


    // MSA 10.09.20 This method determines the obstruction between two Scoreboard boxes due to VolumeObjects.

    const TransferRestrictionData& Scoreboard::CalculateTransferRestrictionData(const BoxIndex& fromBoxIndex, const BoxIndex& toBoxIndex, const TransferRestrictedDimension& transferRestrictedDimension)
    {
        if (m_VOSBIMap.empty())
        {
            return myDefaultTRD; // If there are no VolumeObjects, skip out early.
        }

        TransferRestrictionData trd = myDefaultTRD; // Copy construct from the default, unrestricted object

        const std::pair<BoxIndex, BoxIndex> pair(fromBoxIndex, toBoxIndex);

        std::map<std::pair<BoxIndex, BoxIndex>, TransferRestrictionData>::const_iterator iter = m_transferRestrictionMap.find(pair);
        if (iter == m_transferRestrictionMap.end())
        {
            // No result cached. Calculate from scratch.
            if (fromBoxIndex == toBoxIndex)
            {
                if (m_VOSBIMap.find(fromBoxIndex) != m_VOSBIMap.end())
                {
                    const VolumeObjectScoreboardBoxInteraction& vosbi = m_VOSBIMap[fromBoxIndex];

                    trd.volumeObjectFrom = vosbi.volumeObject;
                    trd.volumeObjectTo = vosbi.volumeObject;
                    trd.backgroundVolumeObjectFrom = vosbi.backgroundVolumeObject; // Will usually be __nullptr
                    trd.backgroundVolumeObjectTo = vosbi.backgroundVolumeObject; // Will usually be __nullptr

                    // Directional component has no effect on intrabox transfer. (In fact, it probably hasn't even been calculated correctly)
                    //if(vosbi.transferRestrictedDimensions[transferRestrictedDimension])

                    trd.restrictedProportionFrom = 1.0 - vosbi.coincidentProportion;
                    trd.unrestrictedProportionFrom = vosbi.coincidentProportion;
                    trd.restrictedProportionTo = 1.0 - vosbi.coincidentProportion;
                    trd.unrestrictedProportionTo = vosbi.coincidentProportion;
                    trd.unrestrictedTransferWithinVO = true;

                    // For transfer within a box, we'll use the highest permeability.
                    trd.voFromPermeability = 0.0;
                    trd.voToPermeability = 0.0;
                    for (TransferRestrictedDimensionPermeabilities::const_iterator piter = vosbi.transferRestrictedDimensionPermeabilities.begin(); piter != vosbi.transferRestrictedDimensionPermeabilities.end(); ++piter)
                    {
                        trd.voFromPermeability = Utility::CSMax(trd.voFromPermeability, piter->second);
                        trd.voToPermeability = Utility::CSMax(trd.voToPermeability, piter->second);
                    }
                }
                //    Else: there is no VOSBI for this box, and transfer within it is unrestricted.
            }
            else
            {
                const VolumeObjectScoreboardBoxInteraction* fromVosbi = __nullptr;
                const VolumeObjectScoreboardBoxInteraction* toVosbi = __nullptr;
                if (m_VOSBIMap.find(fromBoxIndex) != m_VOSBIMap.end())
                {
                    fromVosbi = &(m_VOSBIMap[fromBoxIndex]);
                    trd.backgroundVolumeObjectFrom = fromVosbi->backgroundVolumeObject;
                }
                if (m_VOSBIMap.find(toBoxIndex) != m_VOSBIMap.end())
                {
                    toVosbi = &(m_VOSBIMap[toBoxIndex]);
                    trd.backgroundVolumeObjectTo = toVosbi->backgroundVolumeObject;
                }

                if (fromVosbi != __nullptr || toVosbi != __nullptr)
                {
                    if (fromVosbi != __nullptr && toVosbi != __nullptr && fromVosbi->volumeObject == toVosbi->volumeObject)
                    {
                        // Both Boxes are coincident with the same VolumeObject
                        trd.unrestrictedTransferWithinVO = true;
                        trd.volumeObjectFrom = fromVosbi->volumeObject;
                        trd.volumeObjectTo = toVosbi->volumeObject;
                    }
                    else
                    {
                        // One box is coincident with one VolumeObject, the other is coincident with another (or with none).
                        trd.unrestrictedTransferWithinVO = false;
                    }

                    if (fromVosbi != __nullptr && fromVosbi->transferRestrictedDimensions[transferRestrictedDimension])
                    {
                        // From box is restricted. 
                        trd.volumeObjectFrom = fromVosbi->volumeObject;
                        trd.unrestrictedProportionFrom = trd.unrestrictedTransferWithinVO ? (fromVosbi->coincidentProportion) : (1.0 - fromVosbi->coincidentProportion);
                        trd.restrictedProportionFrom = 1.0 - trd.unrestrictedProportionFrom;
                        TransferRestrictedDimensionPermeabilities::const_iterator f = fromVosbi->transferRestrictedDimensionPermeabilities.find(transferRestrictedDimension);
                        if (f != fromVosbi->transferRestrictedDimensionPermeabilities.end())
                        {
                            trd.voFromPermeability = f->second;
                        }
                        else
                        {
                            // If a permeability value is not found, AND the dimension in question is determined to be obstructed, then the permeability defaults to 0.0.
                            trd.voFromPermeability = 0.0;
                        }
                        if (trd.backgroundVolumeObjectFrom != __nullptr && toVosbi != __nullptr && toVosbi->backgroundVolumeObject != trd.backgroundVolumeObjectFrom && toVosbi->volumeObject != trd.backgroundVolumeObjectFrom)
                        {
                            // If From box has a background VO that is not a) also the background VO of To box, or b) the normal VO of To box, 
                            // the VolumeObjects are set up illegally.
                            RmAssert(false, "Illegal VolumeObject configuration. VolumeObjects may be located inside one another, but may not intersect, and each Scoreboard box may only contain surface(s) from ONE VolumeObject.");
                        }
                    } // if(fromVosbi!=NULL && fromVosbi->transferRestrictedDimensions[transferRestrictedDimension])

                    // MSA 11.06.29 Must reverse the transferRestrictedDimension here, because the positive direction ("out of") in fromBox results in the negative direction ("into") toBox, and vice versa.
                    TransferRestrictedDimension reverseDirectionDimension = ReverseDirection(transferRestrictedDimension);

                    if (toVosbi != __nullptr && toVosbi->transferRestrictedDimensions[reverseDirectionDimension])
                    {
                        // To box is (also?) restricted.
                        trd.volumeObjectTo = toVosbi->volumeObject;
                        trd.unrestrictedProportionTo = trd.unrestrictedTransferWithinVO ? (toVosbi->coincidentProportion) : (1.0 - toVosbi->coincidentProportion);
                        trd.restrictedProportionTo = 1.0 - trd.unrestrictedProportionTo;
                        TransferRestrictedDimensionPermeabilities::const_iterator f = toVosbi->transferRestrictedDimensionPermeabilities.find(reverseDirectionDimension);
                        if (f != toVosbi->transferRestrictedDimensionPermeabilities.end())
                        {
                            trd.voToPermeability = f->second;
                        }
                        else
                        {
                            // If a permeability value is not found, AND the dimension in question is determined to be obstructed, then the permeability defaults to 0.0.
                            trd.voToPermeability = 0.0;
                        }
                        if (trd.backgroundVolumeObjectTo != __nullptr && fromVosbi != __nullptr && fromVosbi->backgroundVolumeObject != trd.backgroundVolumeObjectTo && fromVosbi->volumeObject != trd.backgroundVolumeObjectTo)
                        {
                            // If To box has a background VO that is not a) also the background VO of From box, or b) the normal VO of From box, 
                            // the VolumeObjects are set up illegally.
                            RmAssert(false, "Illegal VolumeObject configuration. VolumeObjects may be located inside one another, but may not intersect, and each Scoreboard box may only contain surface(s) from ONE VolumeObject.");
                        }
                    } // if(toVosbi!=__nullptr && toVosbi->transferRestrictedDimensions[transferRestrictedDimension])
                } // if(fromVosbi!=__nullptr || toVosbi!=__nullptr)

                // Else: Both fromVosbi and toVosbi do not exist. No transfer restriction.
            }

            RmAssert(trd.unrestrictedProportionFrom + trd.restrictedProportionFrom == 1.0, "Error in transfer trd");
            RmAssert(trd.unrestrictedProportionTo + trd.restrictedProportionTo == 1.0, "Error in transfer trd");

            m_transferRestrictionMap[pair] = trd;
            return m_transferRestrictionMap[pair]; // Remember we're returning a reference here.
        }
        return iter->second;
    }
} /* namespace rootmap */


