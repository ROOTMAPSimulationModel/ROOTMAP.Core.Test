/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessSharedAttributeOwner.cpp
// Purpose:     Implementation of the ProcessSharedAttributeOwner class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/shared_attributes/ProcessSharedAttributeOwner.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "simulation/process/common/Process.h"


namespace rootmap
{
    RootMapLoggerDefinition(ProcessSharedAttributeOwner);

    ProcessSharedAttributeOwner::ProcessSharedAttributeOwner
    (const char* name,
        Process* p
    )
        : SharedAttributeOwner()
        , mySharedAttributeOwnerName(name)
        , myProcess(p)
    {
        RootMapLoggerInitialisation("rootmap.ProcessSharedAttributeOwner");
    }

    void ProcessSharedAttributeOwner::RegisterSharedAttribute
    (SharedAttribute* sa,
        SharedAttributeSupplier* supplier,
        const VariationNameArray& savKeyList,
        const SharedAttributeVariationList& savList
    )
    {
        const std::string& supplier_name = supplier->GetSharedAttributeSupplierName();

        LOG_INFO << "Registering Process attribute {Supplier:"
            << supplier_name << "} "
            << *(sa->GetCharacteristicDescriptor());

        if (supplier_name == "Scoreboard")
            // PlantSummary attributes are added in PlantCoordinator::AddSummary
            // || (supplier_name=="PlantSummary") )
        {
            myProcess->AddCharacteristic(sa->GetCharacteristicDescriptor());
        }

        SharedAttributeOwner::RegisterSharedAttribute(sa, supplier, savKeyList, savList);
    }


    const std::string& ProcessSharedAttributeOwner::GetSharedAttributeOwnerName() const
    {
        return mySharedAttributeOwnerName;
    }
} /* namespace rootmap */


