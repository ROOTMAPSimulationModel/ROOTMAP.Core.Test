#ifndef TableDAI_H
#define TableDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TableDAI.h
// Purpose:     Declaration of the TableDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/Structures.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "simulation/common/Types.h"

namespace rootmap
{
    class TableDAI
    {
    public:
        virtual std::string getProcessName() const = 0;
        virtual std::string getCharacteristicName() const = 0;
        virtual ViewDirection getViewDirection() const = 0;
        virtual long getLayer() const = 0;
        virtual ScoreboardStratum getScoreboardStratum() const = 0;
        /**
         * Virtual destructor, like all good base class destructors should be
         */
        virtual ~TableDAI()
        {
        }

    protected:
        TableDAI()
        {
        }
    }; // class TableDAI
} /* namespace rootmap */

#endif // #ifndef TableDAI_H
