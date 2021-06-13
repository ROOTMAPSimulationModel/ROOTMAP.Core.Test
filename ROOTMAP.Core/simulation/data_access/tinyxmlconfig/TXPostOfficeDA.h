#ifndef TXPostOfficeDA_H
#define TXPostOfficeDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXPostOfficeDA.h
// Purpose:     Declaration of the TXPostOfficeDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "simulation/common/Types.h"
#include "simulation/data_access/interface/PostOfficeDAI.h"

namespace rootmap
{
    class TXPostOfficeDA : public PostOfficeDAI
    {
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

    public:
        virtual ~TXPostOfficeDA();

        TXPostOfficeDA(unsigned int seed,
            ProcessTime_t previousTime,
            ProcessTime_t now,
            ProcessTime_t startTime,
            ProcessTime_t nearestEnd,
            ProcessTime_t defaultRunTime);

    private:
        unsigned int m_seed;
        ProcessTime_t m_previousTime;
        ProcessTime_t m_now;
        ProcessTime_t m_startTime;
        ProcessTime_t m_nearestEnd;
        ProcessTime_t m_defaultRunTime;
    }; // class TXPostOfficeDA
} /* namespace rootmap */

#endif // #ifndef TXPostOfficeDA_H
