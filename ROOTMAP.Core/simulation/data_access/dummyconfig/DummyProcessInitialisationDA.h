#ifndef DummyProcessInitialisationDA_H
#define DummyProcessInitialisationDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyProcessInitialisationDA.h
// Purpose:     Declaration of the DummyProcessInitialisationDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ProcessInitialisationDAI.h"

namespace rootmap
{
    class DummyProcessInitialisationDA : public ProcessInitialisationDAI
    {
    public:
        virtual ~DummyProcessInitialisationDA();

        DummyProcessInitialisationDA();
        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */

    private:

        ///
        /// member declaration
    }; // class DummyProcessInitialisationDA
} /* namespace rootmap */

#endif // #ifndef DummyProcessInitialisationDA_H
