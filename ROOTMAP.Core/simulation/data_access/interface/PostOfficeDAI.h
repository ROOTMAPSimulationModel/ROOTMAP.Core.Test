#ifndef PostOfficeDAI_H
#define PostOfficeDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PostOfficeDAI.h
// Purpose:     Declaration of the PostOfficeDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"

namespace rootmap
{
    class PostOfficeDAI
    {
    public:
        virtual unsigned int getSeed() const = 0;
        /**
         * @return the previous timestamp
         */
        virtual ProcessTime_t getPreviousTime() const = 0;

        /**
         * retrieves the value of the nearest end time
         */
        virtual ProcessTime_t getNow() const = 0;

        /**
         * retrieves the value of the nearest end time
         */
        virtual ProcessTime_t getStartTime() const = 0;

        /**
         * retrieves the value of the nearest end time
         */
        virtual ProcessTime_t getNearestEnd() const = 0;

        /**
         * retrieves the default value for the amount of time that is "Run"
         */
        virtual ProcessTime_t getDefaultRunTime() const = 0;


    protected:
        PostOfficeDAI()
        {
        }

        virtual ~PostOfficeDAI()
        {
        }
    }; // class PostOfficeDAI
} /* namespace rootmap */

#endif // #ifndef PostOfficeDAI_H
