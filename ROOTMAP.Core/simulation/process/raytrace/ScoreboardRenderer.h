#ifndef ScoreboardRenderer_H
#define ScoreboardRenderer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardRenderer.h
// Purpose:     Declaration of the ScoreboardRenderer class
// Created:     22-03-2009
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
//other files whose definitions we use in this file
#include "simulation/process/common/Process.h"
#include "core/log/Logger.h"
#include "simulation/process/common/IProcessIterationUser.h"
#include "simulation/scoreboard/IScoreboardIterationUser.h"

namespace rootmap
{
    // "Forward Declarations"
    class Plant;
    class PlantSummary;
    class Scoreboard;
    class ProcessActionDescriptor;
    class ScoreboardCoordinator;
    class ScoreboardStratum;
    class SimulationEngine;
    class ProcessSharedAttributeOwner;

    struct ScoreboardBoxIndex;
    class ScoreboardColourInfo;
    class Process;

    /**
     * This process class provides the raytracing of the scoreboard boxes
     */
    class ScoreboardRenderer : public Process, public IScoreboardIterationUser
    {
    public:
        ScoreboardRenderer();
        ~ScoreboardRenderer();

        virtual void UseScoreboard(Scoreboard* scoreboard);

        /**
         * For obtaining the ScoreboardCoordinator before the output rules are
         * created
         */
        virtual long int Register(ProcessActionDescriptor* action);

        /**
         * This is how the SimulationEngine sends the CharacteristicColourInfo
         * that this class uses
         */
        virtual long int DoGeneralPurpose0(ProcessActionDescriptor* action); // 'GPm0'

        /**
         *
         */
        virtual bool DoesRaytracerOutput() const;

        /**
         * Performs the Raytrace activity. Iterates over the scoreboards,
         */
        virtual long int DoRaytracerOutput(RaytracerData* raytracerData);

        /**
         * Returns the point about which animations orbit if this is the
         * main process in the raytrace
         */
        virtual DoubleCoordinate GetOrigin() const;

        /**
         * Called by the PostOffice when deciding to send useful messages.
         * The base class returns false, we override to return true.
         */
        virtual bool DoesOverride() const;

        /**
         * This class doesn't actually perform drawing, however it is necessary
         * for raytracing
         */
        virtual bool DoesDrawing() const;

        /**
         * This class doesn't actually draw in the scoreboard - it just renders it
         * for raytracing. However, raytracing requires that DoesDrawing() returns
         * true, which in turn requires that DrawScoreboard be implemented.
         */
        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);

        void SetScoreboardColourInfo(ScoreboardColourInfo& sci);

    private:
        /**
         * Run the first time, to
         */
        void RunOnce(RaytracerData* raytracerData /*, const ScoreboardStratum & stratum*/);

        /**
         * Performs the raytrace activity for one scoreboard
         */
        void RaytraceScoreboard(RaytracerData* raytracerData, const ScoreboardStratum& stratum);


        RootMapLoggerDeclaration();


        /**
         * Used to iterate over processes, to retrieve names and scoreboardindices
         */
        ProcessCoordinator* m_processCoordinator;

        /**
         * Will eventually be used to iterate over scoreboards to render them
         */
        ScoreboardCoordinator* m_scoreboardCoordinator;

        /**
         * For implementing GetOrigin()
         */
        DoubleCoordinate m_origin;

        int m_outputCount;

        std::vector<CharacteristicIndex> m_scoreboardIndices;
        std::map<CharacteristicIndex, std::pair<double, double>> m_characteristicMinMaxMap;

        ScoreboardColourInfo* m_scoreboardColourInfo;

        // Put these last, because everything that comes after them is made public. See https://wiki.wxwidgets.org/DYNAMIC_CLASS_Macros
        DECLARE_DYNAMIC_CLASS(ScoreboardRenderer);
        DECLARE_DYNAMIC_CLASS_FORCE_USE(ScoreboardRenderer);
    };

    inline void ScoreboardRenderer::SetScoreboardColourInfo(ScoreboardColourInfo& sci)
    {
        m_scoreboardColourInfo = &sci;
    }
} /* namespace rootmap */

#endif // #ifndef ScoreboardRenderer_H
