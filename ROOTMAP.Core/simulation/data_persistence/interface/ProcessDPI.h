#ifndef ProcessDPI_H
#define ProcessDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessDPI.h
// Purpose:     Declaration of the ProcessDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

//#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"
#include <vector>

namespace rootmap
{
    class CharacteristicDPI;
    class ScoreboardStratum;

    typedef std::vector<CharacteristicDPI *> CharacteristicDPICollection;

    class ProcessDPI
    {
    public:
        virtual void storeIdentifier(ProcessIdentifier id) = 0;
        virtual void storeName(const std::string& name) = 0;
        virtual void storeStratum(const ScoreboardStratum& stratum) = 0;
        virtual void storeActivity(ProcessActivity activity) = 0;
        virtual void storeOverride(bool doesOverride) = 0;
        virtual void storeCharacteristics(const CharacteristicDPICollection& characteristics) = 0;

        virtual ~ProcessDPI()
        {
        }

    protected:
        ProcessDPI()
        {
        }

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
    }; // class ProcessDPI
}

#endif // #ifndef ProcessDPI_H
