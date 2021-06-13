#ifndef ScoreboardDAI_H
#define ScoreboardDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDAI.h
// Purpose:     Declaration of the ScoreboardDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/scoreboard/BoundaryArray.h"

namespace rootmap
{
    class ScoreboardDAI
    {
    public:
        virtual std::string getName() const = 0;
        virtual ScoreboardStratum getStratum() const = 0;
        virtual BoundaryValueVector getXBoundaryValueVector() const = 0;
        virtual BoundaryValueVector getYBoundaryValueVector() const = 0;
        virtual BoundaryValueVector getZBoundaryValueVector() const = 0;

        virtual ~ScoreboardDAI()
        {
        }

    protected:
        ScoreboardDAI()
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
    }; // class ScoreboardDAI
} /* namespace rootmap */

#endif // #ifndef ScoreboardDAI_H
