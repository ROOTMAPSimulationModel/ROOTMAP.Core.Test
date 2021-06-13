/////////////////////////////////////////////////////////////////////////////
// Name:        InterprocessDataDescriptor.cpp
// Purpose:     Implementation of the InterprocessDataDescriptor class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/InterprocessDataDescriptor.h"


namespace rootmap
{
    InterprocessDataDescriptor::InterprocessDataDescriptor
    (Process* server_process,
        long s_requested_number,
        long s_requested_index,
        Process* client_process,
        long c_requested_number,
        long c_requested_index,
        long c_rate_number,
        long c_rate_index,
        long c_buffer_number,
        long c_buffer_index,
        long current_timestamp)
        : server(server_process)
        , server_requested_number(s_requested_number)
        , server_requested_index(s_requested_index)
        , client(client_process)
        , client_requested_number(c_requested_number)
        , client_requested_index(c_requested_index)
        , client_request_rate_number(c_rate_number)
        , client_request_rate_index(c_rate_index)
        , client_receival_buffer_number(c_buffer_number)
        , client_receival_buffer_index(c_buffer_index)
        , m_time(current_timestamp)
        , in_use(false)
        , next(0)
    {
    }

    InterprocessDataDescriptor::InterprocessDataDescriptor
    (Process* server_process,
        long s_requested_number,
        long s_requested_index,
        Process* client_process,
        long c_requested_number,
        long c_requested_index,
        long c_buffer_number,
        long c_buffer_index,
        long current_timestamp)
        : server(server_process)
        , server_requested_number(s_requested_number)
        , server_requested_index(s_requested_index)
        , client(client_process)
        , client_requested_number(c_requested_number)
        , client_requested_index(c_requested_index)
        , client_request_rate_number(-1)
        , client_request_rate_index(-1)
        , client_receival_buffer_number(c_buffer_number)
        , client_receival_buffer_index(c_buffer_index)
        , m_time(current_timestamp)
        , in_use(false)
        , next(0)
    {
    }

    InterprocessDataDescriptor::InterprocessDataDescriptor(InterprocessDataDescriptor* other, long current_timestamp)
        : server(other->server)
        , server_requested_number(other->server_requested_number)
        , server_requested_index(other->server_requested_index)
        , client(other->client)
        , client_requested_number(other->client_requested_number)
        , client_requested_index(other->client_requested_index)
        , client_request_rate_number(other->client_request_rate_number)
        , client_request_rate_index(other->client_request_rate_index)
        , client_receival_buffer_number(other->client_receival_buffer_number)
        , client_receival_buffer_index(other->client_receival_buffer_index)
        , m_time(current_timestamp)
        , in_use(false)
        , next(0)
    {
    }
} /* namespace rootmap */

