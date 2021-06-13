/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummarySupplier.cpp
// Purpose:     Implementation of the PlantSummarySupplier class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/plant/PlantSummarySupplier.h"
#include "simulation/process/plant/PlantSummary.h"
#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/shared_attributes/ScoreboardSharedAttribute.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"

#include "core/common/RmAssert.h"


namespace rootmap
{
    RootMapLoggerDefinition(PlantSummarySupplier);

    extern PlantSummaryRegistration* nonexistentCurrentPSR;
#define currentPSR nonexistentCurrentPSR

    SharedAttribute* PlantSummarySupplier::CreateSharedAttribute
    (CharacteristicDescriptor* cd,
        long int variant1Max,
        long int variant2Max
    )
    {
        // If this Debugger() statement is reached, it means that a PlantSummary
        // related SharedAttribute is trying to be made, but not via
        // PlantSummaryRegistration::MakeScoreboardSummaries()
        RmAssert(0 != currentPSR, "Attempt to create PlantSummary SharedAttribute outside of PlantSummaryRegistration");

        //currentPSR->psr_number_of_spatial_subsections = myPlantCoordinator.GetVolumeObjectCoordinator()->GetVolumeObjectList().size() + 1;

        PlantSummary* ps = new PlantSummary;

        Plant* p = 0;
        PlantSet pset;
        long int rootOrder = -1;
        size_t voIndex = 0;
        currentPSR->GetNewPlantSummaryInformation(rootOrder, p, pset, voIndex);

        ps->IPlantSummary(cd,
            p,
            pset,
            rootOrder,
            voIndex,
            cd->GetScoreboardStratum(),
            currentPSR->GetWrapDirection(),
            currentPSR->GetSummaryRoutine()
        );

        //
        //  Adding the Summary to the PlantCoordinator also registers with the scoreboard,
        //  so that when the attribute itself is created it can use the
        //  ScoreboardIndex of the CharacteristicDescriptor
        myPlantCoordinator.AddSummary(ps);

        Scoreboard* scoreboard = myScoreboardCoordinator.GetScoreboard(cd->GetScoreboardStratum());

        RmAssert(0 != scoreboard, "Could not find scoreboard for PlantSummarySupplier");

        SharedAttribute* newattribute = new ScoreboardSharedAttribute
        (cd,
            false,
            scoreboard,
            variant1Max,
            variant2Max
        );

#ifdef CS_DEBUG_STREAM
        CS_DEBUG_STREAM << "PlantSummarySupplier::CreateSharedAttribute Name:" << cd->Name
            << " Default:" << cd->Default
            << " ScoreboardIndex:" << cd->ScoreboardIndex << endl;
#endif

        scoreboard->FillCharacteristicWithValue(cd->ScoreboardIndex, cd->Default);

        return newattribute;
    }


    PlantSummarySupplier::PlantSummarySupplier
    (PlantCoordinator& plantcoordinator,
        ScoreboardCoordinator& scoreboardCoordinator
    )
        : myPlantCoordinator(plantcoordinator)
        , myScoreboardCoordinator(scoreboardCoordinator)
        , mySupplierName("PlantSummary")
    {
        RootMapLoggerInitialisation("rootmap.PlantSummarySupplier");
        SharedAttributeRegistrar::RegisterSupplier(this);
    }


    PlantSummarySupplier::~PlantSummarySupplier()
    {
    }

    const std::string& PlantSummarySupplier::GetSharedAttributeSupplierName()
    {
        return mySupplierName;
    }
} /* namespace rootmap */


