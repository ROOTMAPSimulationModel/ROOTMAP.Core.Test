#ifndef PlantSummarySupplier_H
#define PlantSummarySupplier_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummarySupplier.h
// Purpose:     Declaration of the PlantSummarySupplier class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"
#include "core/log/Logger.h"

namespace rootmap
{
    struct CharacteristicDescriptor;
    class PlantCoordinator;
    class ScoreboardCoordinator;

    class PlantSummarySupplier : public SharedAttributeSupplier
    {
    public:
        // SharedAttributeSupplier implementation
        virtual SharedAttribute* CreateSharedAttribute(CharacteristicDescriptor* cd, long int variant1Max, long int variant2Max);
        virtual const std::string& GetSharedAttributeSupplierName();

        // construction/destruction
        PlantSummarySupplier(PlantCoordinator& plantcoordinator, ScoreboardCoordinator& scoreboardCoordinator);
        virtual ~PlantSummarySupplier();

    private:
        RootMapLoggerDeclaration();
        PlantCoordinator& myPlantCoordinator;
        ScoreboardCoordinator& myScoreboardCoordinator;
        std::string mySupplierName;
    };
} /* namespace rootmap */

#endif    // PlantSummarySupplier_H
