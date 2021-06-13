#ifndef MessageList_H
#define MessageList_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.h
// Purpose:     Declaration of the MessageList class
// Created:     1994
// Author:      RvH
// $Date: 2009-03-03 03:36:51 +0900 (Tue, 03 Mar 2009) $
// $Revision: 43 $
// Copyright:   ©2002-2009 University of Tasmania
//
// Wrapper for a self-contained implementation of a single-linked list of
// Message instances.
//
// Messages are inserted based on a priority determined by their MessageType
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/Message.h"
#include "simulation/process/common/Process_Dictionary.h"

#include "core/log/Logger.h"

#include <list>

namespace rootmap
{
    class MessageList
    {
    public:
        /**
         * Constructor
         */
        MessageList();

        /**
         * Destructor
         */
        ~MessageList();

        /**
         * Add a normal interprocess message
         */
        Message* Add(ProcessTime_t t, Process* src, Process* dest, SpecialProcessData* d, MessageType_t mt);

        /**
         * Add a kStartMessage
         */
        void AddStart();

        /**
         * Add a kPauseMessage
         */
        void AddPause();

        /**
         * Add a kEndMessage
         */
        void AddEnd(ProcessTime_t t);

        /**
         * Remove a normal interprocess message
         */
        bool Remove(ProcessTime_t t, Process* src, Process* dest, SpecialProcessData* d, MessageType_t mt);

        /**
         * Remove an interprocess message, regardless of timestamp
         */
        bool Remove(Process* src, Process* dest, SpecialProcessData* d, MessageType_t mt);

        /**
         * Access at the next message time without removing the element from the list
         */
        ProcessTime_t PeekNextTime();

        /**
         * Access at the next message destination without removing the element from the list
         */
        Process* PeekNextProcess();

        /**
         * Access at the next message destination name without removing the element from the list
         *
         * @param name the string to populate with the type name
         * @param fAppend if true then append to, rather than replace, the input string
         */
        void PeekNextDestinationName(std::string& name, bool fAppend = false);

        /**
         * Access at the next message type name without removing the element from the list
         *
         * @param name the string to populate with the type name
         * @param fAppend if true then append to, rather than replace, the input string
         */
        void PeekNextTypeName(std::string& name, bool fAppend = false);

        /**
         * Remove and return the head message element off the list
         * Provided as a seperate, specific function because it is used by several functions.
         * Best used as
         *      if ((Message* message = Pop()) != __nullptr)
         *      {
         *          do something...
         *      }
         */
        Message* Pop();

        /**
         * Accessor for the size of the list
         */
        size_t GetNumMessages() const;

        /**
         * Causes the list of messages to be written to the log file
         */
        void LogMessages() const;


    private:
        RootMapLoggerDeclaration();

        /**
         * Inserts the message into the correct place in the list
         */
        void Add(Message* message);

        /**
         * Type definition for a list of pointers to Message.
         */
        typedef std::list<Message *> MessageListType;

        /**
         * The list of message pointers
         */
        MessageListType m_messageList;
    }; // class MessageList

    inline size_t MessageList::GetNumMessages() const
    {
        return m_messageList.size();
    }
} /* namespace rootmap */

#endif // #ifndef MessageList_H
