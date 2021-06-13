#ifndef IProcessIterationUser_H
#define IProcessIterationUser_H

/////////////////////////////////////////////////////////////////////////////
// Name:        IProcessIterationUser.h
// Purpose:     Declaration of the IProcessIterationUser class
// Created:     06/06/2006
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
//
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class Process;
    class ScoreboardStratum;

    class IProcessIterationUser
    {
    public:
        IProcessIterationUser();
        virtual ~IProcessIterationUser();

        /**
         * UseProcess
         *
         * Called by the ProcessCoordinator for each process iteration.
         *
         * @param process - the current iteration of process to be used
         */
        virtual void UseProcess(Process* process) = 0;

        /**
         * UseProcess
         *
         * Called by the ProcessCoordinator for each process per scoreboard iteration.
         *
         * @param process - the current iteration of process to be used
         * @param scoreboard - the current iteration of scoreboard to be used
         */
        virtual void UseProcess(Process* process, const ScoreboardStratum& stratum) = 0;
    }; // class IProcessIterationUser
} /* namespace rootmap */

#endif // #ifndef IProcessIterationUser_H
