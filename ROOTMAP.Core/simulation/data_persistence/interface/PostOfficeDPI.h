#ifndef PostOfficeDPI_H
#define PostOfficeDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PostOfficeDPI.h
// Purpose:     Declaration of the PostOfficeDPI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"

namespace rootmap
{
    class PostOfficeDPI
    {
    public:
        /**
         * @return the previous timestamp
         */
        virtual void storePreviousTime(ProcessTime_t previous) = 0;

        /**
         * stores the value of the nearest end time
         */
        virtual void storeNow(ProcessTime_t now) = 0;

        /**
         * stores the value of the nearest end time
         */
        virtual void storeStartTime(ProcessTime_t start) = 0;

        /**
         * stores the value of the nearest end time
         */
        virtual void storeNearestEnd(ProcessTime_t nearestEnd) = 0;

        /**
         * stores the default value for the amount of time that is "Run"
         */
        virtual void storeDefaultRunTime(ProcessTime_t defaultRunTime) = 0;


    protected:
        PostOfficeDPI()
        {
        }

        virtual ~PostOfficeDPI()
        {
        }
    }; // class PostOfficeDPI
} /* namespace rootmap */

#endif // #ifndef PostOfficeDPI_H
