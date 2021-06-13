#ifndef DummyProcessSpecialDA_H
#define DummyProcessSpecialDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyProcessSpecialDA.h
// Purpose:     Declaration of the DummyProcessSpecialDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ProcessSpecialDAI.h"

namespace rootmap
{
    class DummyProcessSpecialDA : public ProcessSpecialDAI
    {
    public:
        virtual ~DummyProcessSpecialDA();

        DummyProcessSpecialDA();
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
    }; // class DummyProcessSpecialDA
} /* namespace rootmap */

#endif // #ifndef DummyProcessSpecialDA_H
