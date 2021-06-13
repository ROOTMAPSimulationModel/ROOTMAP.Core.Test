/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardAttributeSupplier.cpp
// Purpose:     Implementation of the ScoreboardAttributeSupplier class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/shared_attributes/ScoreboardAttributeSupplier.h"
#include "simulation/process/shared_attributes/ScoreboardSharedAttribute.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"


namespace rootmap
{
    RootMapLoggerDefinition(ScoreboardAttributeSupplier);

    SharedAttribute* ScoreboardAttributeSupplier::CreateSharedAttribute
    (CharacteristicDescriptor* cd,
        long int variant1Max,
        long int variant2Max
    )
    {
        //
        // Register with the scoreboard first, so that when the attribute itself
        // is created it can grab the ScoreboardIndex of the CharacteristicDescriptor
        myScoreboard->RegisterCharacteristic(cd);

        SharedAttribute* newattribute = new ScoreboardSharedAttribute
        (cd,
            false,
            myScoreboard,
            variant1Max,
            variant2Max
        );
        LOG_DEBUG << "Created ScoreboardSharedAttribute {Name:" << cd->Name
            << "} {Default:" << cd->Default
            << " {ScoreboardIndex:" << cd->ScoreboardIndex << "}";

        myScoreboard->FillCharacteristicWithValue(cd->ScoreboardIndex, cd->Default);

        return newattribute;
    }


    ScoreboardAttributeSupplier::ScoreboardAttributeSupplier(Scoreboard* sb)
        : myScoreboard(sb)
        , mySupplierName("Scoreboard")
    {
        RootMapLoggerInitialisation("rootmap.ScoreboardAttributeSupplier");
        SharedAttributeRegistrar::RegisterSupplier(this);
    }


    ScoreboardAttributeSupplier::~ScoreboardAttributeSupplier()
    {
    }

    const std::string& ScoreboardAttributeSupplier::GetSharedAttributeSupplierName()
    {
        return mySupplierName;
    }
} /* namespace rootmap */


