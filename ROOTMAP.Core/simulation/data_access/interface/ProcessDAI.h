#ifndef ProcessDAI_H
#define ProcessDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessDAI.h
// Purpose:     Declaration of the ProcessDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"
#include <vector>

namespace rootmap
{
    class CharacteristicDAI;

    typedef std::vector<CharacteristicDAI *> CharacteristicDAICollection;

    class ProcessDAI
    {
    public:
        virtual ProcessIdentifier getIdentifier() const = 0;
        virtual std::string getName() const = 0;
        virtual ScoreboardStratum getStratum() const = 0;
        virtual ProcessActivity getActivity() const = 0;
        virtual bool doesOverride() const = 0;

        virtual const CharacteristicDAICollection& getCharacteristics() const = 0;

        virtual ~ProcessDAI()
        {
        }

    protected:
        ProcessDAI()
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
    }; // class ProcessDAI
} /* namespace rootmap */

#endif // #ifndef ProcessDAI_H
