/////////////////////////////////////////////////////////////////////////////
// Name:        DummyDataAccessManager.cpp
// Purpose:     Implementation of the DummyDataAccessManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyDataAccessManager.h"
#include "simulation/data_access/dummyconfig/DummyScoreboardDA.h"
#include "simulation/data_access/dummyconfig/DummyProcessDA.h"
#include "simulation/data_access/dummyconfig/DummyCharacteristicDA.h"
#include "simulation/data_access/dummyconfig/DummyPostOfficeDA.h"
#include "simulation/data_access/dummyconfig/DummyTableDA.h"
#include "simulation/data_access/dummyconfig/DummyViewDA.h"

#include "simulation/common/IdentifierUtility.h"
#include "simulation/data_access/common/ITableDataUser.h"
#include "simulation/data_access/common/IViewDataUser.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"


namespace rootmap
{
    DummyDataAccessManager::DummyDataAccessManager()
        : m_txConfigurationDataStore(0)
    {
    }

    DummyDataAccessManager::~DummyDataAccessManager()
    {
    }

    void DummyDataAccessManager::constructScoreboards(ScoreboardCoordinator* scoord)
    {
        DummyScoreboardDA dsd;
        scoord->createScoreboard(dsd);
    }

    void DummyDataAccessManager::constructProcesses(ProcessCoordinator* pcoord)
    {
        ScoreboardStratum stratum(ScoreboardStratum::Soil);

        // Dummy Process 1 - just a process, no characteristics
        DummyProcessDA dummy_process_1("DummyProcessNothing", true, IdentifierUtility::instance().useNextProcessIdentifier(stratum), stratum, paMiscellaneous);
        pcoord->createProcess(dummy_process_1);

        // Dummy Process 2 - a process with 2 characteristics
        DummyProcessDA dummy_process_2("DummyProcessWith2Characteristics", false, IdentifierUtility::instance().useNextProcessIdentifier(stratum), stratum, paNONE);

        DummyCharacteristicDA* dummy_characteristic_1 = new DummyCharacteristicDA
        (IdentifierUtility::instance().useNextCharacteristicIdentifier(stratum),
            "Dummy Characteristic 1", "<No Units>", stratum, 0.0, 100.0, 66.6, true, true, true, false);
        DummyCharacteristicDA* dummy_characteristic_2 = new DummyCharacteristicDA
        (IdentifierUtility::instance().useNextCharacteristicIdentifier(stratum),
            "Dummy Characteristic 2", "m^2/m^3", stratum, 0.0, 1.0, 0.34567, true, false, true, false);

        dummy_process_2.addCharacteristicDA(dummy_characteristic_1);
        dummy_process_2.addCharacteristicDA(dummy_characteristic_2);

        pcoord->createProcess(dummy_process_2);
    }

    void DummyDataAccessManager::constructPlants(PlantCoordinator* /* pcoord */)
    {
    }

    void DummyDataAccessManager::constructOutputs(DataOutputCoordinator* /* docoord */)
    {
    }

    void DummyDataAccessManager::constructPostOffice(PostOffice* poffice)
    {
        DummyPostOfficeDA dummy_postoffice(-1, 0, 0, 0, 30 /*days*/ * 24 /*hours*/ * 60 /*days*/ * 60 /*minutes*/ * 60 /*seconds*/);
        poffice->constructPostOffice(dummy_postoffice);
    }

    void DummyDataAccessManager::constructSharedAttributes(SharedAttributeManager* /* sam */)
    {
    }

    void DummyDataAccessManager::initialiseScoreboardData(SimulationEngine* /* sengine */)
    {
    }

    void DummyDataAccessManager::visualiseTables(ITableDataUser* tdatauser)
    {
        DummyTableDA dummy_table;
        tdatauser->createTable(dummy_table);
    }

    void DummyDataAccessManager::visualiseViews(IViewDataUser* vdatauser)
    {
        DummyViewDA dummy_view;
        vdatauser->createView(dummy_view);
    }
} /* namespace rootmap */

