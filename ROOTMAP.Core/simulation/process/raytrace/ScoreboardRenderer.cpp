/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardRenderer.cpp
// Purpose:     Implementation of the ScoreboardRenderer class
// Created:     22-03-2009
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

// Typically, your Process derived class will be in the following location
#include "simulation/process/raytrace/ScoreboardRenderer.h"

namespace rootmap
{
    // wxWidgets stuff, so the process can be dynamically instantiated by name
    IMPLEMENT_DYNAMIC_CLASS(ScoreboardRenderer, Process);
    DECLARE_DYNAMIC_CLASS_FORCE_USE(ScoreboardRenderer);

    ScoreboardRenderer::ScoreboardRenderer()
        : Process()
        , m_processCoordinator(__nullptr)
        , m_scoreboardCoordinator(__nullptr)
        , m_origin(0.0, 0.0, 0.0)
        , m_outputCount(0)
        , m_scoreboardColourInfo(__nullptr)
    {
    }

    ScoreboardRenderer::~ScoreboardRenderer()
    {
        // This class no longer owns the ScoreboardColourInfo
    }

    void ScoreboardRenderer::UseScoreboard(Scoreboard* scoreboard)
    {
    }

    long int ScoreboardRenderer::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        return kNoError;
    }

    void ScoreboardRenderer::RaytraceScoreboard
    (RaytracerData* raytracerData,
        const ScoreboardStratum& stratum)
    {
    }

    DoubleCoordinate ScoreboardRenderer::GetOrigin() const
    {
        return m_origin;
    }


    long int ScoreboardRenderer::Register(ProcessActionDescriptor* action)
    {
        return (Process::Register(action));
    }

    long int ScoreboardRenderer::DoGeneralPurpose0(ProcessActionDescriptor* action)
    {
        return (kNoError);
    }


    void ScoreboardRenderer::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        // No drawing for now
    }

    bool ScoreboardRenderer::DoesOverride() const
    {
        return (true);
    }

    bool ScoreboardRenderer::DoesDrawing() const
    {
        return (true);
    }

    bool ScoreboardRenderer::DoesRaytracerOutput() const
    {
        return (true);
    }

    void ScoreboardRenderer::RunOnce
    (RaytracerData* raytracerData /*,
    const ScoreboardStratum & stratum */)
    {
    }

} /* namespace rootmap */

