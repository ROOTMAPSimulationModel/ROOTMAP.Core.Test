/////////////////////////////////////////////////////////////////////////////
// Name:        PlantSummaryHelper.h
// Purpose:     Declaration of the PlantSummaryHelper class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef PlantSummaryHelper_H
#define PlantSummaryHelper_H

#include "core/log/Logger.h"

namespace rootmap
{
    class Process;
    class PlantCoordinator;
    class ProcessCoordinator;
    class ProcessActionDescriptor;
    class PlantSummaryRegistration;

    class PlantSummaryHelper
    {
    public:
        PlantSummaryHelper(ProcessCoordinator* process_coordinator, PlantCoordinator* plant_coordinator);
        virtual ~PlantSummaryHelper();

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        PlantSummaryRegistration* MakePlantSummaryRegistration(ProcessActionDescriptor* pad);

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        bool PlantSummaryHelper::SendPlantSummaryRegistration
        (PlantSummaryRegistration* psr,
            ProcessActionDescriptor* action,
            Process* p);


    private:
        RootMapLoggerDeclaration();

        ///
        /// member declaration
        PlantCoordinator* m_plantCoordinator;

        ///
        /// member declaration
        ProcessCoordinator* m_processCoordinator;
    }; // class PlantSummaryHelper
} /* namespace rootmap */

#endif // #ifndef PlantSummaryHelper_H
