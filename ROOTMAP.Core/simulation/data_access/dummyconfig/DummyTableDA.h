#ifndef DummyTableDA_H
#define DummyTableDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyTableDA.h
// Purpose:     Declaration of the DummyTableDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/TableDAI.h"

namespace rootmap
{
    class DummyTableDA
        : public TableDAI
    {
    public:
        DummyTableDA();
        ~DummyTableDA();

        /**
         * @return the process name
         */
        std::string getProcessName() const;

        /**
         * retrieves the characteristic name
         */
        std::string getCharacteristicName() const;

        /**
         * retrieves the view direction
         */
        ViewDirection getViewDirection() const;

        /**
         * retrieves the view direction
         */
        long getLayer() const;

        /**
         * retrieves the scoreboard stratum
         */
        ScoreboardStratum getScoreboardStratum() const;
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
    }; // class DummyTableDA
} /* namespace rootmap */

#endif // #ifndef DummyTableDA_H
