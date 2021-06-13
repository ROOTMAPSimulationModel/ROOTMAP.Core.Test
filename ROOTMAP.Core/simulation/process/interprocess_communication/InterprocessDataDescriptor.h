#ifndef InterprocessDataDescriptor_H
#define InterprocessDataDescriptor_H
/////////////////////////////////////////////////////////////////////////////
// Name:        InterprocessDataDescriptor.h
// Purpose:     Declaration of the InterprocessDataDescriptor class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
// home of SpecialProcessData, which this class "Is A"
#include "simulation/process/interprocess_communication/Message.h"

namespace rootmap
{
    /* DataStorageType
    Used to describe the different ways the received data might have been stored */
    /*enum DataStorageType
    {
        dstNONE = 'NOne',
        dstScoreboard = 'SCbd',
        dstMemoryBlock = 'MBlk',
        dstCustomised = 'CStm'
    };
    typedef DataStorageType DataStorageType;*/

    // Forward declarations
    class Process;

    /**
     *
     *
     *   InterprocessDataDescriptor
     *   Contains particulars about the information that has been received.
     *   There are only two (at this stage) uses for this :
     *   1. Requests
     *   2. Cough Ups
     *
     *
     */
    class InterprocessDataDescriptor : public SpecialProcessData
    {
    public:
        /// Constructor Version 1 : Intended for requests
        InterprocessDataDescriptor(Process* server_process,
            long s_requested_number,
            long s_requested_index,
            Process* client_process,
            long c_requested_number,
            long c_requested_index,
            long c_rate_number,
            long c_rate_index,
            long c_buffer_number,
            long c_buffer_index,
            long current_timestamp);

        /**
         * Constructor Version 2 : Almost the same as the previous version, but
         * without the client requested rate information, since this
         * version is intended for unsolicited cough-ups
         */
        InterprocessDataDescriptor(Process* server_process,
            long s_requested_number,
            long s_requested_index,
            Process* client_process,
            long c_requested_number,
            long c_requested_index,
            long c_buffer_number,
            long c_buffer_index,
            long current_timestamp);

        /**
         * Constructor Version 3 : Intended for solicited cough-ups
         */
        InterprocessDataDescriptor(InterprocessDataDescriptor* other,
            long current_timestamp);

        /*
            SERVER FUNCTIONS
         */
        Process* GetServer() { return (server); }
        long GetServerRequestedIndex() { return (server_requested_index); }
        long GetServerRequestedNumber() { return (server_requested_number); }

        /*
            CLIENT FUNCTIONS
         */
        Process* GetClient() { return (client); }

        long GetClientRequestedNumber() { return (client_requested_number); }
        long GetClientRequestedIndex() { return (client_requested_index); }

        long GetClientRequestRateNumber() { return (client_request_rate_number); }
        long GetClientRequestRateIndex() { return (client_request_rate_index); }

        long GetClientReceiveBufferNumber() { return (client_receival_buffer_number); }
        long GetClientReceiveBufferIndex() { return (client_receival_buffer_index); }

        /*
            MISCELLANEOUS FUNCTIONS
         */
         //    DataStorageType GetStorage() { return (storage); }

        bool IsInUse() { return (in_use); }
        void SetInUse(bool f) { in_use = f; }
        bool IsRegistered() { return (IsInUse()); }
        void SetRegistered(bool f) { SetInUse(f); }

        long GetTime() { return (m_time); }
        void Refresh(long new_time) { m_time = new_time; }

        InterprocessDataDescriptor* GetNext() { return (next); }
        void SetNext(InterprocessDataDescriptor* new_next) { next = new_next; }


    private:
        /*
         *  SERVER INFORMATION
         */
         /// The requestee
        Process* server;

        /// the characteristic number of the requested characteristic
        ///
        /// This is the characteristic from which information is taken.
        /// It is used by the server in DoCoughUp()
        long int server_requested_number;

        /// the characteristic index of the requested characteristic
        long int server_requested_index;

        /*
         *  CLIENT INFORMATION
         */
        Process* client; // == requester

        /*    the characteristic number of the client's requested characteristic
            the characteristic index of the client's requested characteristic

            This is the actual characteristic into which the information
            will finally be transferred, and is used by the client
            in DoDelayedReceivalReaction() */
        long int client_requested_number;
        long int client_requested_index;

        /*    the characteristic number of the client's requested rate
            the characteristic index of the client's requested rate

            This is the quantity per day that the client is asking for,
            and is used by the server in DoCoughUp() */
        long int client_request_rate_number;
        long int client_request_rate_index;

        /*    the characteristic number of the client's coughed up receival buffer
            the characteristic index of the client's coughed up receival buffer

            This is the temporary storage for information transferred
            from the server to the client. It is used by the server in
            DoCoughUp(), and the client in DoDelayedReceivalReaction() */
        long int client_receival_buffer_number;
        long int client_receival_buffer_index;

        /**
         *  MISCELLANEOUS INFORMATION
         */
         /* decides how the data is stored, ie. whether the Handles or
             characteristic number and index are used */
             //    DataStorageType    storage;

             /*    if (request registration) then
                     in_use is set by the process that is receiving the registration to
                     indicate that the request HAS been registered and will be used.
                 else if (characteristic receival) then
                     in_use is set by the receiving process to note that the information
                     has yet to be used (ie.transferred to permanent storage) and so
                     information should only be ADDED TO this characteristic. */
        bool in_use;
        long int m_time;
        InterprocessDataDescriptor* next;

        friend class InterprocessDescriptorList;
    };
} /* namespace rootmap */

#endif // #ifndef InterprocessDataDescriptor_H
