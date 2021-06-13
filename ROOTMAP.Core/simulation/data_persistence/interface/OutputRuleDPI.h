#ifndef OutputRuleDPI_H
#define OutputRuleDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputRuleDPI.h
// Purpose:     Declaration of the OutputRuleDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

//#include "core/scoreboard/ScoreboardStratum.h"

#include "simulation/common/Types.h"
#include "simulation/data_access/interface/OutputDAI.h"

#include <vector>

namespace rootmap
{
    class Process;
    class ScoreboardStratum;

    class OutputRuleDPI
    {
    public:

        /**
         * @return
         */
        virtual storeProcessName(std::string& pname) = 0;

        /**
         * @return
         */
        virtual storeType(std::string& type) = 0;

        /**
         * For type=="ScoreboardData", this is the characteristic name
         * @return
         */
        virtual storeName(std::string& name) = 0;

        /**
         * @return
         */
        virtual storeStratum(ScoreboardStratum& stratum) = 0;

        /**
         * @return
         */
        virtual storeFileName(std::string& filename) = 0;

        /**
         * For ScoreboardData, this is the DimensionOrder
         * @return
         */
        virtual storeSpecification1(std::string& spec1) = 0;

        /**
         * @return
         */
        virtual storeSpecification2(std::string& spec2) = 0;

        /**
         * @return
         */
        virtual storeIntervalWhen(OutputDAI::IntervalWhen& intervalWhen) = 0;

        /**
         * @return
         */
        virtual storeRegularWhen(OutputDAI::RegularWhen& regularWhen) = 0;

        /**
         * @return
         */
        virtual storeReopenStrategy(OutputDAI::std::string& reopenStrategy) = 0;

        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~OutputRuleDPI()
        {
        }

    protected:
        OutputRuleDPI()
        {
        }
    }; // class OutputRuleDPI
} /* namespace rootmap */

#endif // #ifndef OutputRuleDPI_H
