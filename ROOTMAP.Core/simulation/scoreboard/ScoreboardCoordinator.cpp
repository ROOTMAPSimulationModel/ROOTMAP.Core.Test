/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardCoordinator.cpp
// Purpose:     Implementation of the ScoreboardCoordinator class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/scoreboard/ScoreboardCoordinator.h"
#include "simulation/scoreboard/IScoreboardIterationUser.h"

#include "simulation/data_access/interface/ScoreboardDAI.h"
#include "simulation/data_access/common/DataAccessManager.h"

#include "core/common/Exceptions.h"
#include "core/common/RmAssert.h"
#include "core/macos_compatibility/CCollaborator.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "simulation/process/modules/VolumeObject.h"
#include "simulation/process/modules/BoundingRectangularPrism.h"

#include <strstream>

#include "core/utility/Utility.h"
#include "wx/log.h"


#define SCOREBOARD_ITERATION_BEGIN if(!m_scoreboards.empty()) { for ( std::vector<Scoreboard *>::iterator s_iter = m_scoreboards.begin() ; s_iter != m_scoreboards.end() ; ++s_iter ) { Scoreboard * scoreboard_ = (*s_iter); if (0==scoreboard_) { continue; }
#define SCOREBOARD_ITERATION_END }}

namespace rootmap
{
    ScoreboardCoordinator::ScoreboardCoordinator(SimulationEngine& engine, DataAccessManager& dam)
        : m_engine(engine)
        , eXMeasurement(0.0)
        , eYMeasurement(0.0)
        , eZMeasurement(0.0)
    {
        dam.constructScoreboards(this);
    }

    ScoreboardCoordinator::~ScoreboardCoordinator()
    {
        // MSA 09.12.01 Destroying all Scoreboards (we should not just allow them to leak because we assume the program to be closing.
        //                                            For one, the user of GUI ROOTMAP may have closed this simulation and be about to open another.)
        for (ScoreboardCollection::iterator iter = m_scoreboards.begin(); iter != m_scoreboards.end(); ++iter)
        {
            delete (*iter);
        }
    }

    void ScoreboardCoordinator::createScoreboard(const ScoreboardDAI& scoreboardData)
    {
        BoundaryArray bax(scoreboardData.getStratum(), X, scoreboardData.getXBoundaryValueVector(), scoreboardData.getName());
        BoundaryArray bay(scoreboardData.getStratum(), Y, scoreboardData.getYBoundaryValueVector(), scoreboardData.getName());
        BoundaryArray baz(scoreboardData.getStratum(), Z, scoreboardData.getZBoundaryValueVector(), scoreboardData.getName());

        BuildScoreboard(scoreboardData.getStratum(), bax, bay, baz);
    }

    void ScoreboardCoordinator::RegisterVolumeObject(VolumeObject* voptr)
    {
        SCOREBOARD_ITERATION_BEGIN

            BoundingRectangularPrism* scoreboardSimulationVO = new BoundingRectangularPrism(0,
                DoubleCoordinate(0, 0, 0),
                DoubleCoordinate(scoreboard_->GetThickness(X), scoreboard_->GetThickness(Y), scoreboard_->GetThickness(Z)),
                "dummy");

        RmAssert(voptr->Within(scoreboardSimulationVO),
            "Warning: VolumeObject lies at least partially outside the Scoreboard.\nThis will result in errors if the root system grows outside the Scoreboard.\nTo avoid potential problems, define the Scoreboard as large enough to contain all predicted root growth, and/or define all VolumeObjects as inside the Scoreboard.");

        wxDELETE(scoreboardSimulationVO);

        scoreboard_->IncrementVOCount();

        std::map<BoxIndex, VolumeObjectScoreboardBoxInteraction>::const_iterator insertionIterator = scoreboard_->m_VOSBIMap.begin();
        for (BoxIndex box_index = scoreboard_->begin(); box_index <= scoreboard_->end(); ++box_index)
        {
            DoubleBox box;
            scoreboard_->GetBoxDimensions(box_index, box);

            TransferRestrictedDimensionBitset transfer_restricted_dimensions;
            TransferRestrictedDimensionPermeabilities transfer_restricted_dimension_permeabilities;

            const double proportion = voptr->GetCoincidentProportion(box, transfer_restricted_dimensions, transfer_restricted_dimension_permeabilities);

            if (proportion > 0.0)
            {
                VolumeObjectScoreboardBoxInteraction vosbi;
                vosbi.backgroundVolumeObject = __nullptr;
                vosbi.boxIndex = box_index;
                vosbi.volumeObject = voptr;
                vosbi.coincidentProportion = proportion;
                vosbi.transferRestrictedDimensions = transfer_restricted_dimensions;
                // MSA 11.06.17 Check if this does a deep copy
                // MSA 11.06.17 Yep seems to.
                vosbi.transferRestrictedDimensionPermeabilities = transfer_restricted_dimension_permeabilities;
                if (scoreboard_->m_VOSBIMap.find(box_index) == scoreboard_->m_VOSBIMap.end())
                {
                    scoreboard_->m_VOSBIMap[box_index] = vosbi;
                }
                else
                {
                    // This box already has a VOSBI. 
                    // Must ensure either the existing VOSBI or thisnew one is
                    // a "background" one (that is, no surfaces, 100% coincidence;
                    // this box must be completely enclosed by the "background" VO).
                    double existingCoincidentProportion = scoreboard_->m_VOSBIMap[box_index].coincidentProportion;
                    if (proportion == 1.0 && transfer_restricted_dimensions.none())
                    {
                        if (existingCoincidentProportion == 1.0 && scoreboard_->m_VOSBIMap[box_index].transferRestrictedDimensions.none())
                        {
                            // Both VOs are 100% coincident and have no surfaces in this box.
                            if (scoreboard_->m_VOSBIMap[box_index].volumeObject->Within(voptr))
                            {
                                // Thenew VO contains the existing one.
                                if (scoreboard_->m_VOSBIMap[box_index].backgroundVolumeObject == __nullptr
                                    ||
                                    voptr->Within(scoreboard_->m_VOSBIMap[box_index].backgroundVolumeObject))
                                {
                                    // If there was no pre-existing background VO, 
                                    // or the pre-existing background VO contains thenew background VO,
                                    // set to thenew background VO.
                                    scoreboard_->m_VOSBIMap[box_index].backgroundVolumeObject = voptr;
                                }
                                // Else: thenew background VO is "behind" (outside) the existing background VO.
                            }
                            else
                            {
                                // The existing VO contains thenew one.
                                // That is, this is the foreground VOSBI. Need to swap things around.
                                VolumeObject* bgv = scoreboard_->m_VOSBIMap[box_index].volumeObject;
                                vosbi.backgroundVolumeObject = bgv;
                                scoreboard_->m_VOSBIMap[box_index] = vosbi;
                            }
                        }
                        else
                        {
                            if (scoreboard_->m_VOSBIMap[box_index].backgroundVolumeObject == __nullptr || voptr->Within(scoreboard_->m_VOSBIMap[box_index].backgroundVolumeObject))
                            {
                                // This is the background VO, and the existing VO has surfaces in this box.
                                scoreboard_->m_VOSBIMap[box_index].backgroundVolumeObject = voptr;
                            }
                        }
                    }
                    else if (existingCoincidentProportion == 1.0 && scoreboard_->m_VOSBIMap[box_index].transferRestrictedDimensions.none())
                    {
                        // This is the foreground VOSBI. Need to swap things around.
                        VolumeObject* bgv = scoreboard_->m_VOSBIMap[box_index].volumeObject;
                        vosbi.backgroundVolumeObject = bgv;
                        scoreboard_->m_VOSBIMap[box_index] = vosbi;
                    }
                    else
                    {
                        // MSA 11.06.23 How do we throw a meaningful, APP-project-independent error to the user?
                        // For now, using a debug assertion.
                        RmAssert(false, "Invalid VolumeObject encountered. VolumeObjects may be located inside one another, but may not intersect, and each Scoreboard box may only contain surface(s) from ONE VolumeObject.");
                    }
                }
            } // if(proportion>0)
        } // for "each box"
        SCOREBOARD_ITERATION_END
    }

    /* BuildScoreboard
    For expediency, we set the number of slots set aside to the maximum number possible -
    being either as many as there are boxes required, or that number divided by some multiple
    of two (2) until it is no larger than the largest short.

    We also need to set the topLeft of the eDefaultViewInfo to the top left of the top leftest
    scoreboard, which means the uppermost scoreboard */
    void ScoreboardCoordinator::BuildScoreboard
    (const ScoreboardStratum& stratum,
        const BoundaryArray& ba_x,
        const BoundaryArray& ba_y,
        const BoundaryArray& ba_z)
    {
        Scoreboard* theScoreboard = new Scoreboard(stratum, ba_x, ba_y, ba_z);

        AddScoreboard(theScoreboard);
    }

    void ScoreboardCoordinator::AddScoreboard(Scoreboard* scoreboard)
    {
        unsigned long int stratumvalue = scoreboard->GetScoreboardStratum().value();

        // expand the vector if required
        if (m_scoreboards.size() <= stratumvalue)
        {
            m_scoreboards.resize(stratumvalue + 1, 0);
            m_zMeasurements.resize(stratumvalue + 1, 0);
        }

        if (m_scoreboards.at(stratumvalue) != 0)
        {
            std::ostrstream s;
            s << "Scoreboard already exists at stratum value " << stratumvalue;
            throw std::runtime_error(s.str());
        }

        m_scoreboards[stratumvalue] = scoreboard;
        m_zMeasurements[stratumvalue] = scoreboard->GetThickness(Z);
        m_usedStrata.insert(stratumvalue);

        if (eXMeasurement < std::numeric_limits<double>::epsilon())
        {
            eXMeasurement = scoreboard->GetThickness(X);
        }
        else if (eXMeasurement != scoreboard->GetThickness(X))
        {
            //RmAssert((eXMeasurement != scoreboard->GetThickness(X)),wxT("New scoreboard X measurement does not match current"));
            wxLogError(wxT("New scoreboard X measurement %s does not match current %s"), Utility::ToString(scoreboard->GetThickness(X)).c_str(), Utility::ToString(eXMeasurement).c_str());
        }

        if (eYMeasurement < std::numeric_limits<double>::epsilon())
        {
            eYMeasurement = scoreboard->GetThickness(Y);
        }
        else if (eYMeasurement != scoreboard->GetThickness(Y))
        {
            //RmAssert((eYMeasurement != scoreboard->GetThickness(Y)),wxT("New scoreboard Y measurement does not match current"));
            wxLogError(wxT("New scoreboard Y measurement %s does not match current %s"), Utility::ToString(scoreboard->GetThickness(Y)).c_str(), Utility::ToString(eYMeasurement).c_str());
        }

        eZMeasurement += scoreboard->GetThickness(Z);
    }

    const BoundaryArray& ScoreboardCoordinator::GetVisibleHBoundaries
    (ViewDirection viewdir,
        const ScoreboardStratum& stratum) const
    {
        switch (viewdir)
        {
        case vFront:
            return GetBoundaryArray(X, stratum);
            break;
        case vSide:
            return GetBoundaryArray(Y, stratum);
            break;
        case vTop:
            return GetBoundaryArray(X, stratum);
            break;
        default:
            throw std::logic_error("Bad ViewDirection given to ScoreboardCoordinator::GetVisibleHBoundaries");
            break;
        }
    }


    const BoundaryArray& ScoreboardCoordinator::GetVisibleVBoundaries
    (ViewDirection viewdir,
        const ScoreboardStratum& stratum) const
    {
        switch (viewdir)
        {
        case vFront:
            return GetBoundaryArray(Z, stratum);
            break;
        case vSide:
            return GetBoundaryArray(Z, stratum);
            break;
        case vTop:
            return GetBoundaryArray(Y, stratum);
            break;
        default:
            throw std::logic_error("Bad ViewDirection given to ScoreboardCoordinator::GetVisibleVBoundaries");
            break;
        }
    }


    void ScoreboardCoordinator::GetScoreboardExtent
    (const ScoreboardStratum& stratum,
        ViewDirection viewdir,
        double* width, double* height)
    {
        switch (viewdir)
        {
        case vFront:
            *width = eXMeasurement;
            if (stratum == ScoreboardStratum::ALL)
            {
                *height = eZMeasurement;
            }
            else
            {
                *height = m_zMeasurements[stratum.value()];
            }
            break;
        case vSide:
            *width = eYMeasurement;
            if (stratum == ScoreboardStratum::ALL)
            {
                *height = eZMeasurement;
            }
            else
            {
                *height = m_zMeasurements[stratum.value()];
            }
            break;
        case vTop:
            *width = eXMeasurement;
            *height = eYMeasurement;
            break;
        }
    }

    void ScoreboardCoordinator::GetTotalScoreboardBounds(ViewDirection viewdir, DoubleRect* soilRect)
    {
        switch (viewdir)
        {
        case vFront:
        {
            const BoundaryArray& h_boundaries_f = GetBoundaryArray(X, ScoreboardStratum(ScoreboardStratum::zeroAtTop));

            soilRect->left = h_boundaries_f.GetTop();
            soilRect->right = h_boundaries_f.GetBottom();

            soilRect->top = GetTopScoreboard()->GetBoundaryArray(Z).GetTop();
            soilRect->bottom = GetBottomScoreboard()->GetBoundaryArray(Z).GetBottom();
        }
        break;

        case vSide:
        {
            const BoundaryArray& h_boundaries_s = GetBoundaryArray(Y, ScoreboardStratum(ScoreboardStratum::zeroAtTop));

            soilRect->left = h_boundaries_s.GetTop();
            soilRect->right = h_boundaries_s.GetBottom();

            soilRect->top = GetTopScoreboard()->GetBoundaryArray(Z).GetTop();
            soilRect->bottom = GetBottomScoreboard()->GetBoundaryArray(Z).GetBottom();
        }
        break;

        case vTop:
        {
            const BoundaryArray& h_boundaries_t = GetBoundaryArray(X, ScoreboardStratum(ScoreboardStratum::zeroAtTop));
            soilRect->left = h_boundaries_t.GetTop();
            soilRect->right = h_boundaries_t.GetBottom();

            const BoundaryArray& v_boundaries_t = GetBoundaryArray(Y, ScoreboardStratum(ScoreboardStratum::zeroAtTop));
#if defined ROOTMAP_VIEW_FROM_TOP_FIXME
            soilRect->top = v_boundaries_t.GetBottom();
            soilRect->bottom = v_boundaries_t.GetTop();
#else
            soilRect->top = v_boundaries_t.GetTop();
            soilRect->bottom = v_boundaries_t.GetBottom();
#endif // #if defined ROOTMAP_VIEW_FROM_TOP_FIXME
        }
        break;
        }
    }

    void ScoreboardCoordinator::GetScoreboardBounds
    (const ScoreboardStratum& stratum,
        ViewDirection viewdir,
        DoubleRect* soilRect)
    {
        if (!GetScoreboard(stratum))
        {
            soilRect->left = 0;
            soilRect->right = 0;
            soilRect->top = 0;
            soilRect->bottom = 0;
            return;
        }

        switch (viewdir)
        {
        case vFront:
        {
            const BoundaryArray& h_boundaries = GetBoundaryArray(vSide, stratum);
            const BoundaryArray& v_boundaries = GetBoundaryArray(vTop, stratum);
            soilRect->left = h_boundaries.GetTop();
            soilRect->right = h_boundaries.GetBottom();
            soilRect->top = v_boundaries.GetTop();
            soilRect->bottom = v_boundaries.GetBottom();
        }
        break;

        case vSide:
        {
            const BoundaryArray& h_boundaries = GetBoundaryArray(vFront, stratum);
            const BoundaryArray& v_boundaries = GetBoundaryArray(vTop, stratum);
            soilRect->left = h_boundaries.GetTop();
            soilRect->right = h_boundaries.GetBottom();
            soilRect->top = v_boundaries.GetTop();
            soilRect->bottom = v_boundaries.GetBottom();
        }
        break;

        case vTop:
        {
            const BoundaryArray& h_boundaries = GetBoundaryArray(vSide, stratum);
            const BoundaryArray& v_boundaries = GetBoundaryArray(vFront, stratum);
            soilRect->left = h_boundaries.GetTop();
            soilRect->right = h_boundaries.GetBottom();
            soilRect->top = v_boundaries.GetBottom();
            soilRect->bottom = v_boundaries.GetTop();
        }
        break;
        }
    }

    void ScoreboardCoordinator::GetScoreboardBounds
    (const ScoreboardStratum& stratum,
        DoubleBox& bounds)
    {
        Scoreboard* sb = GetScoreboard(stratum);
        if (__nullptr != sb)
        {
            bounds.left = sb->GetTop(X);
            bounds.right = sb->GetBottom(X);

            bounds.top = sb->GetTop(Z);
            bounds.bottom = sb->GetBottom(Z);

            bounds.front = sb->GetTop(Y);
            bounds.back = sb->GetBottom(Y);
        }
        else
        {
            bounds.left = 0;
            bounds.right = 0;

            bounds.top = 0;
            bounds.bottom = 0;

            bounds.front = 0;
            bounds.back = 0;
        }
    }

    void ScoreboardCoordinator::GetTotalScoreboardBounds(DoubleBox& bounds)
    {
        Scoreboard* top_scoreboard = GetTopScoreboard();
        Scoreboard* bottom_scoreboard = GetBottomScoreboard();

        bounds.left = top_scoreboard->GetBoundaryArray(X).GetFirst();
        bounds.right = top_scoreboard->GetBoundaryArray(X).GetLast();

        bounds.front = top_scoreboard->GetBoundaryArray(Y).GetFirst();
        bounds.back = top_scoreboard->GetBoundaryArray(Y).GetLast();

        bounds.top = top_scoreboard->GetBoundaryArray(Z).GetFirst();
        bounds.bottom = bottom_scoreboard->GetBoundaryArray(Z).GetLast();
    }

    Scoreboard* ScoreboardCoordinator::GetTopScoreboard()
    {
        for (ScoreboardCollection::iterator iter = m_scoreboards.begin();
            iter != m_scoreboards.end(); ++iter)
        {
            if (0 != (*iter))
            {
                return (*iter);
            }
        }

        RmAssert(false, "No Scoreboard found during forward iteration");
        return __nullptr;
    }


    Scoreboard* ScoreboardCoordinator::GetBottomScoreboard()
    {
        //NO!
        //return (*(m_scoreboards.rbegin()));

        for (ScoreboardCollection::reverse_iterator iter(m_scoreboards.rbegin());
            iter != m_scoreboards.rend(); ++iter)
        {
            if (0 != (*iter))
            {
                return (*iter);
            }
        }

        RmAssert(false, "No Scoreboard found during reverse iteration");
        return __nullptr;
    }

    void ScoreboardCoordinator::DependUponScoreboard(const ScoreboardStratum& stratum, CCollaborator* dependent)
    {
        Scoreboard* sboard = 0;

        if ((sboard = GetScoreboard(stratum)) != 0)
        {
            dependent->DependUpon(sboard);
        }
    }


    void ScoreboardCoordinator::DependUponAllScoreboards(CCollaborator* dependent)
    {
        for (ScoreboardStratum stratum(ScoreboardStratum::first());
            stratum <= ScoreboardStratum::last();
            ++stratum)
        {
            DependUponScoreboard(stratum, dependent);
        }
    }


    void ScoreboardCoordinator::CancelDependencyOnScoreboard(const ScoreboardStratum& stratum, CCollaborator* dependent)
    {
        Scoreboard* sboard = 0;
        if ((sboard = GetScoreboard(stratum)) != 0)
        {
            dependent->CancelDependency(sboard);
        }
    }


    void ScoreboardCoordinator::CancelDependencyOnAllScoreboards(CCollaborator* dependent)
    {
        for (ScoreboardStratum stratum(ScoreboardStratum::first());
            stratum <= ScoreboardStratum::last();
            ++stratum)
        {
            CancelDependencyOnScoreboard(stratum, dependent);
        }
    }


    /*
    Calculates the mean value of a line of boxes.
    */
    double ScoreboardCoordinator::GetMean
    (long int hi,
        long int vi,
        ViewDirection viewdir,
        long int characteristicindex,
        const ScoreboardStratum& stratum)
    {
        if ((stratum < ScoreboardStratum::first()) || // if its less than the first
            (stratum > ScoreboardStratum::last()) // or greater than the last
            )
        {
            Debugger();
        }

        Scoreboard* sb = GetScoreboard(stratum);
        const BoundaryArray& layers = GetBoundaryArray(viewdir, stratum);

        long int xval, yval, zval;
        long int loopmax = layers.GetNumLayers();
        long int loopval;

        double mean;
        double cumulativeValue = 0.0;
        double depth = GetScoreboardDepth(viewdir);

        double layer_thickness = 0.0;
        double characteristic_value = 0.0;

        switch (viewdir)
        {
        case vFront:
            xval = hi;
            zval = vi;
            for (loopval = 1; loopval <= loopmax; ++loopval)
            {
                layer_thickness = layers.GetLayerThickness(loopval);
                characteristic_value = sb->GetCharacteristicValue(characteristicindex, xval, loopval, zval);
                cumulativeValue += layer_thickness * characteristic_value;
            }
            break;
        case vSide:
            yval = hi;
            zval = vi;
            for (loopval = 1; loopval <= loopmax; ++loopval)
            {
                layer_thickness = layers.GetLayerThickness(loopval);
                characteristic_value = sb->GetCharacteristicValue(characteristicindex, loopval, yval, zval);
                cumulativeValue += layer_thickness * characteristic_value;
            }
            break;
        case vTop:
            xval = hi;
            yval = vi;
            for (loopval = 1; loopval <= loopmax; ++loopval)
            {
                layer_thickness = layers.GetLayerThickness(loopval);
                characteristic_value = sb->GetCharacteristicValue(characteristicindex, xval, yval, loopval);
                cumulativeValue += layer_thickness * characteristic_value;
            }
            break;
        case vNONE:
            RmAssert(false, "Invalid view direction");
            return -1;
        }

        mean = cumulativeValue / depth;
        return mean;
    }


    void ScoreboardCoordinator::GetBoxSoil
    (DoubleBox* box,
        const ScoreboardStratum& stratum,
        BoxCoordinate* coordinate)
    {
        const BoundaryArray& boundaries_X = GetBoundaryArray(X, stratum);
        box->left = boundaries_X.GetLayerTop(coordinate->x);
        box->right = boundaries_X.GetLayerBottom(coordinate->x);

        const BoundaryArray& boundaries_Y = GetBoundaryArray(Y, stratum);
        box->front = boundaries_Y.GetLayerTop(coordinate->y);
        box->back = boundaries_Y.GetLayerBottom(coordinate->y);

        const BoundaryArray& boundaries_Z = GetBoundaryArray(Z, stratum);
        box->top = boundaries_Z.GetLayerTop(coordinate->z);
        box->bottom = boundaries_Z.GetLayerBottom(coordinate->z);
    }


    void ScoreboardCoordinator::GetBoxSoilRect
    (DoubleRect* boxRect,
        const ScoreboardStratum& stratum,
        BoxCoordinate* coordinate,
        ViewDirection viewdir
    )
    {
        DoubleBox fbox;
        GetBoxSoil(&fbox, stratum, coordinate);
        DoubleBoxToRect(fbox, boxRect, viewdir);
    }


    Scoreboard* ScoreboardCoordinator::FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate, WrapDirection* wrap_direction)
    {
        Scoreboard* found_scoreboard = 0;

        SCOREBOARD_ITERATION_BEGIN
            FindBoxBySoilCoordinate(box_coordinate, scoreboard_, soil_coordinate, wrap_direction);
        if (box_coordinate->isValid())
        {
            found_scoreboard = scoreboard_;
            break;
        }
        SCOREBOARD_ITERATION_END

            return (found_scoreboard);
    }

    Scoreboard* ScoreboardCoordinator::FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate)
    {
        // throw UnimplementedFunctionException("ScoreboardCoordinator::FindBoxBySoilCoordinate(BoxCoordinate *, DoubleCoordinate *)");
        Scoreboard* found_scoreboard = 0;

        SCOREBOARD_ITERATION_BEGIN
            FindBoxBySoilCoordinate(box_coordinate, scoreboard_, soil_coordinate);
        if (box_coordinate->isValid())
        {
            found_scoreboard = scoreboard_;
            break;
        }
        SCOREBOARD_ITERATION_END

            return (found_scoreboard);
    }

    void ScoreboardCoordinator::IterateOverScoreboards(IScoreboardIterationUser* user)
    {
        SCOREBOARD_ITERATION_BEGIN
            user->UseScoreboard(scoreboard_);
        SCOREBOARD_ITERATION_END
    }
} /* namespace rootmap */

