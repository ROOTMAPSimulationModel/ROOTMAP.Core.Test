#ifndef ScoreboardCoordinator_H
#define ScoreboardCoordinator_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardCoordinator.h
// Purpose:     Declaration of the ScoreboardCoordinator class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Types.h"
#include "core/common/DoubleCoordinates.h"
#include "simulation/common/Types.h"
#include "core/scoreboard/BoundaryArray.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "core/scoreboard/ScoreboardUtility.h"
#include <list>

class CCollaborator;

namespace rootmap
{
    struct CharacteristicDescriptor;
    class ScoreboardDAI;
    class IScoreboardIterationUser;
    class SimulationEngine;
    class DataAccessManager;
    class Process;
    class VolumeObject;

    /**
     * The ScoreboardCoordinator is the aptly named coordinator of all the
     * scoreboards. It owns the BoundaryArrays for each Dimension, and the
     * Scoreboard for each stratum. It coordinates access to all measurements,
     * scoreboards and dependencies between them. It provides mechanisms to
     * search for a Scoreboard/Box pair for any given soil coordinate, and
     * allows for iterating over all scoreboards via the IScoreboardIterationUser
     * mechanism.
     *
     * Some general tips for function nomenclature:
     *
     * Measurement: the measurement in one dimension of a scoreboard
     * Depth : the measurement in a ViewDirection of a scoreboard. That means,
     *         "into the screen" of a scoreboard projected in 2D
     * Extent: the measurement of the visible width and height of a
     *         scoreboard projected in 2D
     * Bounds: both the visible rectangle of a scoreboard(s) in the given
     *         ViewDirection, and the bounds in all 3 Dimensions
     *
     * There is a GetScoreboardXyz and GetTotalScoreboardXyz for each
     * Xyz={Measurement,Depth,Extent,Bounds}
     */
    class ScoreboardCoordinator
    {
    public:
        ScoreboardCoordinator(SimulationEngine& engine, DataAccessManager& dam);
        virtual ~ScoreboardCoordinator();

        void createScoreboard(const ScoreboardDAI& scoreboardData);

        void RegisterVolumeObject(VolumeObject* voptr);

        /**
         *
         */
        void RegisterAllCharacteristics();

        /**
         *
         */
        void RegisterCharacteristic(CharacteristicDescriptor* cd);

        /**
         *
         */
        void BuildScoreboard(const ScoreboardStratum& ss,
            const BoundaryArray& ba_x,
            const BoundaryArray& ba_y,
            const BoundaryArray& ba_z);

        /**
         *
         */
        void AddScoreboard(Scoreboard* scoreboard);

        /**
         *
         */
        Scoreboard* GetScoreboard(const ScoreboardStratum& stratum) const;

        /**
         *
         */
        Scoreboard* GetTopScoreboard();

        /**
         *
         */
        Scoreboard* GetBottomScoreboard();

        /**
         *
         */
        void DependUponScoreboard(const ScoreboardStratum& stratum, CCollaborator* dependent);

        /**
         *
         */
        void DependUponAllScoreboards(CCollaborator* dependent);

        /**
         *
         */
        void CancelDependencyOnScoreboard(const ScoreboardStratum& stratum, CCollaborator* dependent);

        /**
         *
         */
        void CancelDependencyOnAllScoreboards(CCollaborator* dependent);


        // //////////////////////////////
        // BoundaryArrays
        // //////////////////////////////

        /**
         *
         */
        BoundaryArray* MakeBoundaryArray(const ScoreboardStratum& stratum,
            Dimension d,
            const BoundaryValueVector& boundaries,
            const std::string& name);

        /**
         *
         */
        const BoundaryArray& GetBoundaryArray(ViewDirection viewdir, const ScoreboardStratum& stratum) const;

        /**
         *
         */
        const BoundaryArray& GetBoundaryArray(const Dimension& d, const ScoreboardStratum& stratum) const;

        /**
         *
         */
        const BoundaryArray& GetVisibleHBoundaries(ViewDirection viewdir, const ScoreboardStratum& stratum) const;

        /**
         *
         */
        const BoundaryArray& GetVisibleVBoundaries(ViewDirection viewdir, const ScoreboardStratum& stratum) const;


        // //////////////////////////////
        // Measurement accessors
        // //////////////////////////////

        /**
         * The way to figure out how big a scoreboard looks from any direction.
         *
         * Use GetTotalScoreboardMeasurement for ScoreboardStratum::ALL
         */
        double GetScoreboardMeasurement(const ScoreboardStratum& stratum,
            Dimension dimension);

        /**
         * The way to figure out how big the combined scoreboards are from any
         * direction.
         */
        double GetTotalScoreboardMeasurement(const Dimension& dimension) const;

        /**
         * The way to figure out how big a scoreboard looks from any direction.
         */
        void GetScoreboardExtent(const ScoreboardStratum& stratum,
            ViewDirection viewdir,
            double* width, double* height);

        /**
         *
         */
        void GetTotalScoreboardExtent(ViewDirection viewdir,
            double* width, double* height) const;

        /**
         *
         */
        void GetScoreboardBounds(const ScoreboardStratum& stratum,
            ViewDirection viewdir,
            DoubleRect* soilRect);

        /**
         * The way to figure out how far a view window should scroll in either
         * direction, depending on the direction in which we are looking.
         *
         * Populates the soilRect with the visible (ie. 2D projected) boundary
         * of the combined scoreboards.
         */
        void GetTotalScoreboardBounds(ViewDirection viewdir,
            DoubleRect* soilRect);

        /**
         * populates the box with the boundary of the given scoreboard
         */
        void GetScoreboardBounds(const ScoreboardStratum& stratum,
            DoubleBox& bounds);

        /**
         * populates the box with the boundary of combined scoreboards
         */
        void GetTotalScoreboardBounds(DoubleBox& bounds);

        /**
         * returns the depth "into the monitor" of the combined scoreboards
         * when looking through the scoreboards in the given direction
         */
        double GetScoreboardDepth(const ViewDirection& viewdir) const;


        /**
         *
         */
        double GetMean(long int hi, long int vi, ViewDirection viewdir, long int xo, const ScoreboardStratum& stratum);

        /**
         *
         */
        void GetBoxSoil(DoubleBox* box, const ScoreboardStratum& stratum, BoxCoordinate* coordinate);

        /**
         *
         */
        void GetBoxSoil(DoubleBox* box, const ScoreboardStratum& stratum, short int x, short int y, short int z);

        /**
         *
         */
        void GetBoxSoilRect(DoubleRect* boxRect, const ScoreboardStratum& stratum, BoxCoordinate* coordinate, ViewDirection viewdir);

        /**
         *
         */
        Scoreboard* FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate, WrapDirection* wrap_direction);

        /**
         *
         */
        void FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, Scoreboard* scoreboard, const DoubleCoordinate* soil_coordinate, WrapDirection* wrap_direction);

        /**
         *
         */
        Scoreboard* FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, const DoubleCoordinate* soil_coordinate);

        /**
         *
         */
        void FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, Scoreboard* scoreboard, const DoubleCoordinate* soil_coordinate);

        /**
         *
         */
        double GetBoxVolume(const ScoreboardStratum& stratum, long x, long y, long z);

        /**
         *
         */
        void CalculateBoxVolumes(const ScoreboardStratum& stratum);

        /**
         *
         */
        void IterateOverScoreboards(IScoreboardIterationUser* user);

        virtual DoubleCoordinate GetOrigin() const;

    private:

        /** Combined X Measurement */
        double eXMeasurement;
        /** Combined Y Measurement */
        double eYMeasurement;
        /** Combined Z Measurement */
        double eZMeasurement;

        /**
         * The x and y total measurements for each individual scoreboard
         * should all be identical. This array represents the Z measurements
         * of the individual scoreboards
         */
        std::vector<double> m_zMeasurements;

        /**
         *
         */
        typedef std::vector<Scoreboard *> ScoreboardCollection;

        /**
         *
         */
        ScoreboardCollection m_scoreboards;

        std::set<ScoreboardStratumDataType> m_usedStrata;

        /**
         *
         */
        SimulationEngine& m_engine;

        ScoreboardCoordinator(const ScoreboardCoordinator&);
        ScoreboardCoordinator& operator=(const ScoreboardCoordinator&);

    public:
        /**
         * The name used by config
         */
        static std::string sConfigName;
    }; // class ScoreboardCoordinator

    inline DoubleCoordinate ScoreboardCoordinator::GetOrigin() const
    {
        return DoubleCoordinate(eXMeasurement / 2.0, eYMeasurement / 2.0, eZMeasurement / 2.0);
    }

    // MSA 11.04.27 Newly inlined functions

    inline Scoreboard* ScoreboardCoordinator::GetScoreboard(const ScoreboardStratum& stratum) const
    {
        //if ((stratum==ScoreboardStratum::NONE) || (stratum==ScoreboardStratum::ALL))
        //{
        //    return 0;
        //}
        //return (m_scoreboards[stratum.value()]);
        if (m_usedStrata.count(stratum.value()) > 0)
        {
            return (m_scoreboards[stratum.value()]);
        }
        return 0;
    }

    inline void ScoreboardCoordinator::RegisterCharacteristic(CharacteristicDescriptor* cd)
    {
        GetScoreboard(cd->GetScoreboardStratum())->RegisterCharacteristic(cd);
    }

    inline const BoundaryArray& ScoreboardCoordinator::GetBoundaryArray
    (ViewDirection viewdir,
        const ScoreboardStratum& stratum) const
    {
        return (GetBoundaryArray(ScoreboardUtility::ViewDirection2Dimension(viewdir), stratum));
    }


    inline const BoundaryArray& ScoreboardCoordinator::GetBoundaryArray
    (const Dimension& d,
        const ScoreboardStratum& stratum) const
    {
        Scoreboard* sboard = GetScoreboard(stratum);
        if (sboard != 0)
        {
            return sboard->GetBoundaryArray(d);
        }

        RmAssert(__nullptr != sboard, "No Scoreboard for given stratum");
        //throw std::runtime_error("No Scoreboard for given ScoreboardStratum");
        // this next one just to satisfy the compiler
        return sboard->GetBoundaryArray(d);
    }

    inline double ScoreboardCoordinator::GetScoreboardMeasurement
    (const ScoreboardStratum& stratum,
        Dimension dimension)
    {
        switch (dimension)
        {
        case X:
            return eXMeasurement;
        case Y:
            return eYMeasurement;
        case Z:
            return m_zMeasurements[stratum.value()];
        case NoDimension:
        default:
            return 0.0;
        }
    }

    inline double ScoreboardCoordinator::GetTotalScoreboardMeasurement(const Dimension& dimension) const
    {
        switch (dimension)
        {
        case X: return eXMeasurement;
        case Y: return eYMeasurement;
        case Z: return eZMeasurement;
        case NoDimension:
        default:
            return 0.0;
        }
    }

    inline double ScoreboardCoordinator::GetScoreboardDepth(const ViewDirection& viewdir) const
    {
        switch (viewdir)
        {
        case vFront: return eYMeasurement;
        case vSide: return eXMeasurement;
        case vTop: return eZMeasurement;
        case NoDimension:
        default:
            return 0.0;
        }
    }

    inline void ScoreboardCoordinator::GetTotalScoreboardExtent(ViewDirection viewdir, double* width, double* height) const
    {
        switch (viewdir)
        {
        case vFront:
            *width = eXMeasurement;
            *height = eZMeasurement;
            break;
        case vSide:
            *width = eYMeasurement;
            *height = eZMeasurement;
            break;
        case vTop:
            *width = eXMeasurement;
            *height = eYMeasurement;
            break;
        case vNONE:
        default:
            break;
        } // switch (viewdir)
    }

    inline void ScoreboardCoordinator::GetBoxSoil(DoubleBox* box, const ScoreboardStratum& stratum, short int x, short int y, short int z)
    {
        BoxCoordinate coord(x, y, z);
        ScoreboardCoordinator::GetBoxSoil(box, stratum, &coord);
    }

    inline void ScoreboardCoordinator::FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, Scoreboard* scoreboard, const DoubleCoordinate* soil_coordinate, WrapDirection* wrap_direction)
    {
        scoreboard->FindBoxBySoilCoordinate(box_coordinate, soil_coordinate, wrap_direction);
    }

    inline void ScoreboardCoordinator::FindBoxBySoilCoordinate(BoxCoordinate* box_coordinate, Scoreboard* scoreboard, const DoubleCoordinate* soil_coordinate)
    {
        scoreboard->FindBoxBySoilCoordinate(box_coordinate, soil_coordinate);
    }

    //
    // 20020807 RvH - Moved functionality to Scoreboard class
    inline void ScoreboardCoordinator::CalculateBoxVolumes(const ScoreboardStratum& stratum)
    {
        GetScoreboard(stratum)->CalculateBoxVolumes();
    }
} /* namespace rootmap */

#endif // #ifndef ScoreboardCoordinator_H
