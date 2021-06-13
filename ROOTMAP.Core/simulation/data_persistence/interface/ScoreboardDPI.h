#ifndef ScoreboardDPI_H
#define ScoreboardDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDPI.h
// Purpose:     Declaration of the ScoreboardDPI class
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
    class ScoreboardStratum;

    class ScoreboardDPI
    {
    public:
        virtual void storeName(const std::string& name) = 0;
        virtual void storeStratum(const ScoreboardStratum& stratum) = 0;
        virtual void storeBoundaries(const BoundaryValueVector& xbounds,
            const BoundaryValueVector& ybounds,
            const BoundaryValueVector& zbounds) = 0;

        virtual ~ScoreboardDPI()
        {
        }

    protected:
        ScoreboardDPI()
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
    }; // class ScoreboardDPI
} /* namespace rootmap */

#endif // #ifndef ScoreboardDPI_H
