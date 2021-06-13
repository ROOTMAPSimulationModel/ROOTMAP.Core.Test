#include "simulation/process/raytrace/RenderCoordinator.h"
#include "simulation/process/raytrace/ScoreboardRenderer.h"
#include "simulation/common/SimulationEngine.h"
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/Process.h"
#include "core/common/RmAssert.h"

namespace rootmap
{
    RootMapLoggerDefinition(RenderCoordinator);

    RenderCoordinator::RenderCoordinator(SimulationEngine& engine, DataAccessManager& dam)
        : m_engine(engine)
        , m_dataAccessManager(dam)
    {
        RootMapLoggerInitialisation("rootmap.RenderCoordinator");
    }

    RenderCoordinator::~RenderCoordinator()
    {
    }

    void RenderCoordinator::Initialise()
    {
        // Build ScoreboardColourInfo from data access manager
        // Can't just do this upon construction, because the Processes aren't initialised then
        m_dataAccessManager.initialiseRenderer(this);
        // Provide ScoreboardRenderer with the info it needs
        Process* p = m_engine.GetProcessCoordinator().FindProcessByProcessName("ScoreboardRenderer");
        if (p)
        {
            ScoreboardRenderer* srp = dynamic_cast<ScoreboardRenderer*>(p);
            srp->SetScoreboardColourInfo(m_scoreboardColourInfo);
        }
    }

    void RenderCoordinator::AddCharacteristicColourInfo(CharacteristicColourInfo& cci)
    {
        m_scoreboardColourInfo.push_back(cci);
    }

    void RenderCoordinator::AddCharacteristicColourData(ViewDAI::CharacteristicColourData& ccd)
    {
        CharacteristicColourInfo cci;

        cci.colourRangeMin = ccd.colourRangeMin;
        cci.colourRangeMax = ccd.colourRangeMax;
        cci.characteristicRangeMin = ccd.characteristicRangeMin;
        cci.characteristicRangeMax = ccd.characteristicRangeMax;

        // Get the Process and Characteristic numbers from names
        long characteristic_number = -1;
        Process* p = m_engine.GetProcessCoordinator().FindProcessByCharacteristicName(ccd.characteristicName, &characteristic_number);
        if (p)
        {
            RmAssert(p->GetProcessName() == ccd.processName, "Process Names do not match");

            cci.characteristicIndex = p->GetCharacteristicIndex(characteristic_number);
            cci.characteristicID = p->GetCharacteristicID(characteristic_number);

            m_scoreboardColourInfo.push_back(cci);
        }
        else
        {
            LOG_WARN << "Could not find process for characteristic " + ccd.characteristicName;
        }
    }
} /* namespace rootmap */
