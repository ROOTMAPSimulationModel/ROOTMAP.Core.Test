#ifndef PostOffice_H
#define PostOffice_H
/////////////////////////////////////////////////////////////////////////////
// Name:        PostOffice.h
// Purpose:     Declaration of the PostOffice class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
// Description:
// In RootMap, the PostOffice is an asynchronous, message-passing,
// interprocess-communication class.
/////////////////////////////////////////////////////////////////////////////
#include <queue>

#include "core/log/Logger.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"

#define kNow 0

namespace rootmap
{
    class Message;
    class Process;
    class Scoreboard;
    class MessageList;
    class PostOfficeDAI;
    class SimulationEngine;
    class DataAccessManager;
    class ProcessCoordinator;
    class ScoreboardCoordinator;
    class InterprocessDescriptor;
    class SharedAttributeManager;
    class VolumeObjectCoordinator;

    typedef std::queue<Message *> MessageQueue;

    typedef signed long MessageSendResult_t;

    class PostOffice
    {
    public:
        PostOffice(SimulationEngine& engine, DataAccessManager& dam);
        ~PostOffice();

        void constructPostOffice(PostOfficeDAI& postoffice_data);

        ProcessTime_t GetStart() const;
        ProcessTime_t GetNow() const;
        ProcessTime_t GetPrevious() const;
        ProcessTime_t GetNearestEnd() const;
        ProcessTime_t GetDefaultRunTime() const;

        //
        // Timestamped messages.
        //
        //
        // For sending to "this"
        void sendMessage(ProcessTime_t time, Process* src);
        //
        // For sending to a different process at a specified time
        void sendMessage(ProcessTime_t time, Process* src, Process* dest, SpecialProcessData* data, MessageType_t type);


        //
        // No time specified, ie. sent immediately. See m_immediateMessages below
        //
        // Note #1: there is no way to remove an immediate message from the queue.
        void sendMessage(Process* src, Process* dst, MessageType_t type, SpecialProcessData* data = 0);
        //
        // Note #2: this second form is currently meaningless, as the InterprocessDescriptor
        //          class is empty.  Functionless.  See mod 20030403
        void sendMessage(Process* src, Process* dst, InterprocessDescriptor* descriptor);


        //
        // Sending special messages - Start, Pause (MSA new as of 09.10.13) and End
        void SendStart();
        void SendPause();
        void SendEnd(ProcessTime_t endTime);

        //
        // Unsend
        void UnsendMessage(ProcessTime_t untime, Process* src);
        void UnsendMessage(ProcessTime_t untime, Process* src, Process* dest, SpecialProcessData* data, MessageType_t type);


        //
        // These are the business of the PostOffice.
        bool DoNextTimeStamp(ScoreboardCoordinator* scoreboardcoordinator, ProcessCoordinator* processcoordinator, SharedAttributeManager* sam, VolumeObjectCoordinator* voc);
        bool DoNextMessage(ScoreboardCoordinator* scoreboardcoordinator, ProcessCoordinator* processcoordinator, SharedAttributeManager* sam, VolumeObjectCoordinator* voc);


        //
        // Called when the timestamp to be processed is one of End, Start,
        // Initialise or Terminate
        void SendToAllProcesses(ProcessActionDescriptor* action);

        //
        // Querying the contents of the next message
        ProcessTime_t GetNextTimeStamp() const;
        Process* GetNextProcess() const;
        void GetNextDestinationName(std::string& aname) const;

        /**
         *
         */
        static ProcessTime_t CalculateTimeSpan(int years, int months, int days, int hours, int minutes, int seconds);

        /**
         *
         */
        static ProcessTime_t CalculateDateTime(int YY, int mm, int dd, int HH, int MM, int SS);

        void LogMessages() const;

    private:
        void IPostOffice();

        RootMapLoggerDeclaration();

        unsigned int m_seed;

        /**
         *  Since timestamps are not necesarily incremented by a constant, the
         *  previous timestamp cannot be merely calculated.
         *
         *  It is not REALLY required as yet, but put in just in case, for
         * everyone's sake
         */
        ProcessTime_t m_previous;

        /**
         *  The current timestamp being processed
         */
        ProcessTime_t m_now;

        /**
         *  at this stage, cStartTime will be the instance at which the
         *  instance of this class is instantiated. At some time in the future,
         *  however, the start time may well be whatever the user wishes
         */
        ProcessTime_t m_start;

        /**
         */
        ProcessTime_t m_nearestEnd;

        /**
         */
        ProcessTime_t m_defaultRunTime;


        bool m_simulationPaused;

        /**
         */
        bool m_sendingImmediate;


        /**
         *  The fully sorted list of time-stamped messages
         */
        MessageList* m_messages;

        /**
         *  The container of NON-time-stamped, As Immediate As Possible, messages !
         *
         *  At the moment, i intend not to use these.  I think that immediate
         *  messages need handling in one of two ways.  Only one should be chosen.
         *
         *   1.  Send to the destination process immediately, while the sending process
         *       is still in its current timestamp.
         *
         *   2.  Send to the destination process As Soon As the sending process finishes
         *       its processing in the timestamp that it sent the message.  In other
         *       words, plonk the immediate message at the front of the timed message
         *       list.  This is actually feasible, which the first option is not.
         *
         *   3.  Add the message to a special list, which is always dealt with both
         *       before and after any timed message (except Initialise) is sent,
         *       regardless of the timestamp, and if an End is the next message.
         *       Sounds reasonable.
         *
         *   Having documented which, it sounds like 3. is the best approach.
         */
        MessageQueue m_immediateMessages;

    public:

        /**
         */
        void DoImmediateMessages(ScoreboardCoordinator* scoreboardcoordinator, ProcessCoordinator* processcoordinator, SharedAttributeManager* sam, VolumeObjectCoordinator* voc);

        static wxString LogTime(ProcessTime_t t);
    };

    inline ProcessTime_t PostOffice::GetStart() const
    {
        return m_start;
    }

    inline ProcessTime_t PostOffice::GetNow() const
    {
        return m_now;
    }

    inline ProcessTime_t PostOffice::GetPrevious() const
    {
        return m_previous;
    }

    inline ProcessTime_t PostOffice::GetNearestEnd() const
    {
        return m_nearestEnd;
    }

    inline ProcessTime_t PostOffice::GetDefaultRunTime() const
    {
        return m_defaultRunTime;
    }
} /* namespace rootmap */

#include "simulation/process/common/IProcessIterationUser.h"

namespace rootmap
{
    class AllProcessSender
        : public IProcessIterationUser
    {
    public:
        AllProcessSender(ProcessActionDescriptor* actionDescriptor);
        virtual ~AllProcessSender();

        /**
         * Called by the ProcessCoordinator for each process iteration.
         *
         * @param process - the current iteration of process to be used
         */
        virtual void UseProcess(Process* process);

        /**
         * Called by the ProcessCoordinator for each process per scoreboard iteration.
         *
         * @param process - the current iteration of process to be used
         * @param scoreboard - the current iteration of scoreboard to be used
         */
        virtual void UseProcess(Process* process, const ScoreboardStratum& stratum);

    private:
        // hidden copy constructor
        AllProcessSender(const AllProcessSender&);

        RootMapLoggerDeclaration();

        ///
        ///
        ProcessActionDescriptor* m_descriptor;
    }; // class AllProcessSender
} /* namespace rootmap */

#endif // #ifndef PostOffice_H
