#ifndef DummySimulationEngineDA_H
#define DummySimulationEngineDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummySimulationEngineDA.h
// Purpose:     Declaration of the DummySimulationEngineDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/SimulationEngineDAI.h"

namespace rootmap
{
    class DummySimulationEngineDA : public SimulationEngineDAI
    {
    public:
        virtual ~DummySimulationEngineDA();

        DummySimulationEngineDA();

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
    }; // class DummySimulationEngineDA
} /* namespace rootmap */

#endif // #ifndef DummySimulationEngineDA_H
