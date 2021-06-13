#ifndef Message_H
#define Message_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Message.h
// Purpose:     Declaration of the Message class
// Created:     28/05/2006
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "core/macos_compatibility/macos_compatibility.h"
#include "core/common/Types.h"

#include "core/log/Logger.h"

namespace rootmap
{
    class Process;
    class SpecialProcessData;

    class Message
    {
    public:
        typedef signed long int MessagePriority;

        // Two types of constructor, for when we do and don't know what the next message is
        Message(ProcessTime_t t, Process* source, Process* destination, SpecialProcessData* specialData, MessageType_t messageType);

        /*    NO destructor, since nothing is allocated by this class
            Note that the data handle is not disposed of by this class.
            That is because this class does not "own" it.
            It is used long after this message is destroyed */

        long GetTime() const;

        Process* GetSource() const;

        Process* GetDestination() const;

        SpecialProcessData* GetData() const;

        MessageType_t GetType() const;

        Message* GetNext();

        bool MatchParameters(Process* src, Process* dest, SpecialProcessData* d, MessageType_t mess_type);

        // void    SetType(MessageType_t mt);
        void SetNext(Message* m);

        std::string StringOfDestination() const;

        std::string StringOfSource() const;

        std::string StringOfType(bool plusExtra = false) const;

        /**
         *
         */
        void LogDebug() const;

        /**
         *
         */
        bool EqualTo(const Message& rhs) const;

        /**
         * Returns true if this message sorts before rhs in our own funny
         * scheme of things.
         *
         * can be used to implement the operator<() for STL container insert
         */
        bool SortBefore(const Message& rhs) const;

        bool operator<(const Message& rhs) const;

        /**
         * called during MessageList (ie. PostOffice) construction
         */
        static void InitialiseMessageTypePrioritiesMap();

    private:
        RootMapLoggerDeclaration();

        // time - activation time of the message
        ProcessTime_t m_time;

        // destination_process - the process to which to send the message
        Process* m_destination_process;

        // source_process - from whence this message originated
        Process* m_source_process;

        // data - additional information which processes might toss about
        SpecialProcessData* m_data;

        // the type of message
        MessageType_t m_type;

        // a pointer to the next message
        Message* m_next;

        // 
        MessagePriority m_priority;

        /**
         * type definition for a map of priorities, keyed on their message type
         */
        typedef std::map<MessageType_t, MessagePriority> MessageTypePriorityMap;

        /**
         *
         */
        static const MessagePriority sMaxMessagePriority;

        /**
         * map of priorities, keyed on their message type
         *
         * Messages are prioritised on a scale from 0 to 100.
         * The lower the number, the earlier in a timestamp they come.
         */
        static MessageTypePriorityMap m_messageTypePriorityMap;

        /**
         *
         */
        static MessagePriority FindPriority(MessageType_t messageType);

        /**
         * The choice was to either make this non-member function a friend or
         * provide a GetPriority() accessor.
         */
        friend std::ostream& operator<<(std::ostream& s, const Message& m);

    public:
        /**
         * Functor used by MessageList for matching messages for removal
         */
        struct ParameterMatch
        {
        public:
            ParameterMatch(ProcessTime_t t,
                Process* src,
                Process* dst,
                SpecialProcessData* data,
                MessageType_t mt)
                : m_time(t), m_time_set(true), m_src(src), m_dst(dst), m_data(data), m_type(mt)
            {
            }

            ParameterMatch(Process* src,
                Process* dst,
                SpecialProcessData* data,
                MessageType_t mt)
                : m_time(0), m_time_set(false), m_src(src), m_dst(dst), m_data(data), m_type(mt)
            {
            }

            bool operator()(Message* m);

        private:
            Process* m_dst;
            Process* m_src;
            SpecialProcessData* m_data;
            MessageType_t m_type;

            ProcessTime_t m_time;
            bool m_time_set;
        };
    };

    inline long Message::GetTime() const
    {
        return (m_time);
    }

    inline Process* Message::GetSource() const
    {
        return (m_source_process);
    }

    inline Process* Message::GetDestination() const
    {
        return (m_destination_process);
    }

    inline SpecialProcessData* Message::GetData() const
    {
        return (m_data);
    }

    inline MessageType_t Message::GetType() const
    {
        return (m_type);
    }

    inline Message* Message::GetNext()
    {
        return (m_next);
    }

    inline void Message::SetNext(Message* m)
    {
        m_next = m;
    }

    std::ostream& operator<<(std::ostream& s, const Message& m);


    /**
     * An abstract class intended as the base class for all information
     * transferred between process modules
     */
    class SpecialProcessData
    {
    public:
        virtual ~SpecialProcessData()
        {
        }

    protected:
        SpecialProcessData()
        {
        }

    private:
        SpecialProcessData(const SpecialProcessData&);
        SpecialProcessData& operator=(const SpecialProcessData&);
    }; // class SpecialProcessData
} /* namespace rootmap */

#endif // #ifndef Message_H
