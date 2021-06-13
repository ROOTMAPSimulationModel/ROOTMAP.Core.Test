#ifndef RenderCoordinator_H
#define RenderCoordinator_H
#include "simulation/common/Types.h"
#include "simulation/process/raytrace/ScoreboardColourInfo.h"
#include "simulation/data_access/interface/ViewDAI.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class SimulationEngine;
    class DataAccessManager;

    class RenderCoordinator
    {
    public:
        // Sole constructor
        RenderCoordinator(SimulationEngine& engine, DataAccessManager& dam);

        virtual ~RenderCoordinator();

        void Initialise();
        void AddCharacteristicColourInfo(CharacteristicColourInfo& cci);
        void AddCharacteristicColourData(ViewDAI::CharacteristicColourData& ccd);

    private:
        ScoreboardColourInfo m_scoreboardColourInfo;
        SimulationEngine& m_engine;
        DataAccessManager& m_dataAccessManager;

        RootMapLoggerDeclaration();
    };
} /* namespace rootmap */

#endif // #ifndef RenderCoordinator_H
