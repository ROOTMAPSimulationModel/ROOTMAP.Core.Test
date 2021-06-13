/////////////////////////////////////////////////////////////////////////////
// Name:        DummyConfigurationDataStore.cpp
// Purpose:     Implementation of the DummyConfigurationDataStore class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "simulation/data_access/dummyconfig/DummyConfigurationDataStore.h"


namespace rootmap
{
    DummyConfigurationDataStore::DummyConfigurationDataStore()
        : myIsInitialised(false)
    {
    }

    DummyConfigurationDataStore::~DummyConfigurationDataStore()
    {
    }


    void DummyConfigurationDataStore::initialise()
    {
        if (myIsInitialised) return;

        /*
           EXAMPLE CONFIGURATION FILE CONTENT
        <rootmap>
          <!-- CONSTRUCTION CONFIGURATION -->
          <configuration>
            <name>Scoreboard Boundaries</name>
            <type>construction</type>
            <owner>ScoreboardCoordinator</owner>
            <location>Scoreboards.xml</location>
          </configuration>
          <configuration>
            <name>Plant Initial Values</name>
            <type>initialisation</type>
            <owner>PlantCoordinator</owner>
            <location>./shared attributes/PlantAttributeInitialValues.xml</location>
          </configuration>
        </rootmap>
        */
        addConstructionData("ScoreboardCoordinator", "Scoreboard Boundaries", "local");
        addConstructionData("ProcessCoordinator", "Processes", "local");
        addConstructionData("PlantCoordinator", "PlantTypes", "local");
        addConstructionData("PlantCoordinator", "Plants", "local");

        addInitialisationData("PlantCoordinator", "Plants", "local");
        addInitialisationData("DummyProcess", "DummyProcess Initial Data", "local");

        myIsInitialised = true;
    }
} /* namespace rootmap */

