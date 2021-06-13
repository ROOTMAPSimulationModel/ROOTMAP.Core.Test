#ifndef ISimulationActivityObserver_H
#define ISimulationActivityObserver_H

/////////////////////////////////////////////////////////////////////////////
// Name:        ISimulationActivityObserver.h
// Purpose:     Declaration of the ISimulationActivityObserver class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class PostOffice;

    class ISimulationActivityObserver
    {
    protected:
        ISimulationActivityObserver()
        {
        }

    public:
        virtual ~ISimulationActivityObserver()
        {
        }

        /**
         * Sent to each observer by the SimulationEngine just before the processes
         * are sent the Start message. The simulation is marked as being in
         * "starting" phase at this point
         *
         * @param postOffice reference to the postOffice.
         */
        virtual void PreRunNotification(const PostOffice& postOffice) = 0;

        /**
         * Sent to each observer by the SimulationEngine before each timestamp
         * is run.
         *
         * @param postOffice reference to the postOffice.
         */
        virtual void PreTimestampNotification(const PostOffice& postOffice) = 0;

        /**
         * May be sent to each observer by the SimulationEngine after each
         * PostOffice message is processed.
         *
         * @param postOffice reference to the postOffice.
         */
        virtual void MidTimestampNotification(const PostOffice& postOffice) = 0;

        /**
         * Sent to each observer by the SimulationEngine after each timestamp
         * is complete.  This is the ideal place for window updates to be
         * triggered.
         *
         * Note that "Post" in this sense is not the verb but the prefix,
         * meaning "after" or "later"
         *
         * @param postOffice reference to the postOffice.
         */
        virtual void PostTimestampNotification(const PostOffice& postOffice) = 0;

        /**
         * Sent to each observer by the SimulationEngine just after the processes
         * are sent the Ending message (but before control is returned to the
         * user). The simulation is in "stopping" phase at this point.
         *
         * @param postOffice reference to the postOffice.
         */
        virtual void PostRunNotification(const PostOffice& postOffice) = 0;

        /**
         *    MSA 09.12.07 New method for informing SAOs to disconnect.
         *  Implementors may set their internal pointers to simulation classes
         *    to NULL, trip a flag, etc. etc.
         *  This method is called when the SimulationEngine is destroyed,
         *  disassociating the simulation classes (which are destroyed at that time)
         *  from the app classes.
         *  This instructs the app classes to close down later without attempting to
         *  communicate with the simulation classes.
         */
        virtual void DisconnectNotification() = 0;

    private:

        ///
        /// member declaration
    }; // class ISimulationActivityObserver
} /* namespace rootmap */

#endif // #ifndef ISimulationActivityObserver_H
