/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummaryHelper.cpp
// Purpose:     Implementation of the PlantSummaryHelper class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/plant/PlantSummaryHelper.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"


namespace rootmap
{
    RootMapLoggerDefinition(PlantSummaryHelper);

    PlantSummaryHelper::PlantSummaryHelper(ProcessCoordinator* process_coordinator, PlantCoordinator* plant_coordinator)
        : m_plantCoordinator(plant_coordinator)
        , m_processCoordinator(process_coordinator)
    {
        RootMapLoggerInitialisation("rootmap.PlantSummaryHelper");
    }

    PlantSummaryHelper::~PlantSummaryHelper()
    {
    }

    PlantSummaryRegistration* PlantSummaryHelper::MakePlantSummaryRegistration
    (ProcessActionDescriptor* /* action */)
    {
        return (new PlantSummaryRegistration());
    }


    bool PlantSummaryHelper::SendPlantSummaryRegistration
    (PlantSummaryRegistration* psr,
        ProcessActionDescriptor* action,
        Process* p)
    {
        Use_ProcessCoordinator;
        Use_PostOffice;
        bool success = false;

        // get this if we haven't one already
        if (0 == m_plantCoordinator)
        {
            Process* plant_coordinator = Find_Process_By_Process_Name("PlantCoordinator");
            m_plantCoordinator = dynamic_cast<PlantCoordinator *>(plant_coordinator);
        }

        // use this if available
        if (0 != m_plantCoordinator)
        {
            long message_type = (psr->IsPerBox())
                ? kRegisterScoreboardPlantSummaryMessage
                : kRegisterGlobalPlantSummaryMessage;

            //
            // Send_Interprocess_Message(time, plant_coordinator, (Handle)psr, message_type);
            // Now untimed (ie. immediate)
            postoffice->sendMessage(p, m_plantCoordinator, message_type, psr);
            success = true;
        }

        return (success);
    }
} /* namespace rootmap */

