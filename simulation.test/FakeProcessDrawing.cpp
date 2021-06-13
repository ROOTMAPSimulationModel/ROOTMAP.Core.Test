/////////////////////////////////////////////////////////////////////////////
// Name:        FakeProcessDrawing.cpp
// Purpose:     Fake stub implementation of the ProcessDrawing class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/ProcessDrawing.h"

namespace rootmap
{
    ProcessDrawing::ProcessDrawing(Process* p)
        : m_process(p)
        , m_singleBuffer(__nullptr)
    {
    }

    ProcessDrawing::~ProcessDrawing()
    {
    }

    void ProcessDrawing::BeginDrawing(IProcessDrawingBuffer* /* buffer */)
    {
    }

    void ProcessDrawing::EndDrawing(IProcessDrawingBuffer* /* buffer */)
    {
    }

    void ProcessDrawing::AddDrawingBuffer(IProcessDrawingBuffer* buffer)
    {
    }

    void ProcessDrawing::RemoveDrawingBuffer(IProcessDrawingBuffer* buffer)
    {
    }

    void ProcessDrawing::DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard, IProcessDrawingBuffer* buffer)
    {
    }


    void ProcessDrawing::MovePenTo(const DoubleCoordinate& fi)
    {
    }

    void ProcessDrawing::DrawLineTo(const DoubleCoordinate& fi)
    {
    }

    void ProcessDrawing::DrawLine(const DoubleCoordinate& from_coord, const DoubleCoordinate& to_coord)
    {
    }

    void ProcessDrawing::DrawRectangle
    (const ScoreboardStratum& stratum,
        BoxCoordinate* bc,
        ViewDirection direction,
        double place)
    {
    }

    void ProcessDrawing::DrawRectangle(const DoubleBox& soil_box, ViewDirection direction, double place)
    {
    }

    void ProcessDrawing::DrawRectangle(const DoubleBox& soil_box, ViewDirection direction)
    {
    }

    void ProcessDrawing::DrawRectangle(const DoubleBox& soil_box)
    {
    }

    void ProcessDrawing::DrawOval(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place)
    {
    }

    void ProcessDrawing::DrawOval(const DoubleBox& soil_box, ViewDirection direction, double place)
    {
    }

    void ProcessDrawing::DrawOval(const DoubleBox& soil_box, ViewDirection direction)
    {
    }

    void ProcessDrawing::DrawOval(const DoubleBox& soil_box)
    {
    }

    void ProcessDrawing::DrawDot(DoubleCoordinate& fi)
    {
    }


    void ProcessDrawing::DrawLineTo(const DoubleCoordinate& fi, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawLine(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawRectangle
    (const ScoreboardStratum& stratum,
        BoxCoordinate* bc,
        ViewDirection direction,
        double place,
        wxColour& colour)
    {
    }

    void ProcessDrawing::DrawRectangle(const DoubleBox& soil_box, ViewDirection direction, double place, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawRectangle(const DoubleBox& soil_box, ViewDirection direction, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawRectangle(const DoubleBox& soil_box, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawOval(const ScoreboardStratum& stratum, BoxCoordinate* bc, ViewDirection direction, double place, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawOval(const DoubleBox& soil_box, ViewDirection direction, double place, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawOval(const DoubleBox& soil_box, ViewDirection direction, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawOval(const DoubleBox& soil_box, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawDot(DoubleCoordinate& fi, wxColour& colour)
    {
    }

    void ProcessDrawing::RemoveDot(const DoubleCoordinate& fi)
    {
    }

    void ProcessDrawing::RemoveAllDots()
    {
    }

    void ProcessDrawing::RemoveRectangle
    (const ScoreboardStratum& stratum,
        BoxCoordinate* bc,
        ViewDirection direction,
        double place)
    {
    }

    void ProcessDrawing::DrawRootSegment(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, const long& order, const double& radius, const PlantElementIdentifier& elid)
    {
    }

    void ProcessDrawing::DrawRootSegment(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, const double& radius, const PlantElementIdentifier& elid, wxColour& colour)
    {
    }

    void ProcessDrawing::DrawCone(const DoubleCoordinate& coord1, const DoubleCoordinate& coord2, const double& radius1, const double& radius2, wxColour& colour, const ViewDirection& direction, const size_t& stacksNSlices, const bool& wireframe)
    {
    }
} /* namespace rootmap */
