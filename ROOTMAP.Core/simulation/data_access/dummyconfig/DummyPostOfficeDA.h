#ifndef DummyPostOfficeDA_H
#define DummyPostOfficeDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPostOfficeDA.h
// Purpose:     Declaration of the DummyPostOfficeDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/PostOfficeDAI.h"

namespace rootmap
{
    class DummyPostOfficeDA : public PostOfficeDAI
    {
    public:
        virtual unsigned int getSeed() const;
        /**
         * @return the previous timestamp
         */
        virtual ProcessTime_t getPreviousTime() const;

        /**
         * retrieves the value of the nearest end time
         */
        virtual ProcessTime_t getNow() const;

        /**
         * retrieves the value of the nearest end time
         */
        virtual ProcessTime_t getStartTime() const;

        /**
         * retrieves the value of the nearest end time
         */
        virtual ProcessTime_t getNearestEnd() const;

        /**
         * retrieves the default value for the amount of time that is "Run"
         */
        virtual ProcessTime_t getDefaultRunTime() const;

        DummyPostOfficeDA(ProcessTime_t previous,
            ProcessTime_t now,
            ProcessTime_t start,
            ProcessTime_t nearest_end,
            ProcessTime_t default_run_time
        );
        virtual ~DummyPostOfficeDA();

    private:
        ProcessTime_t m_previous;
        ProcessTime_t m_now;
        ProcessTime_t m_start;
        ProcessTime_t m_nearestEnd;
        ProcessTime_t m_defaultRunTime;
    }; // class DummyPostOfficeDA
} /* namespace rootmap */

#endif // #ifndef DummyPostOfficeDA_H
