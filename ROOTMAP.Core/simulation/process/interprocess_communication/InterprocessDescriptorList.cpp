/////////////////////////////////////////////////////////////////////////////
// Name:        InterprocessDescriptorList.cpp
// Purpose:     Implementation of the InterprocessDescriptorList class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/InterprocessDescriptorList.h"
#include "simulation/process/interprocess_communication/InterprocessDataDescriptor.h"


namespace rootmap
{
    InterprocessDescriptorList::InterprocessDescriptorList()
        : descriptor_was_found(false)
        , head(0)
        , tail(0)
        , numItems(0)
    {
    }

    InterprocessDescriptorList::~InterprocessDescriptorList()
    {
        // iterate down the list of descriptors, forgetting about them
        InterprocessDataDescriptor* next_descriptor;
        while (head != 0)
        {
            next_descriptor = head->GetNext();
            delete head;
            numItems--;
            head = next_descriptor;
        }
    }

    /* Add (general)
    Insert a message into the list. The descriptorÕs are ordered by ascending
    characteristic number. If an descriptor already exists with the new characteristic
    number, the new descriptor is added BEFORE all the others. NOTE that since this
    is a general routine, no checking for identical descriptorÕs is bothered with. */
    void InterprocessDescriptorList::AddRequest(InterprocessDataDescriptor* request)
    {
        InterprocessDataDescriptor* previous_request = 0;

        if (head == 0)
        {
            head = tail = request;
            ++numItems;
        }
        /****
        **    Next we check if the message still needs to go at the head of the list.
        **    There are two reasons we do this, and one situation in which we might
        **    need to do this.
        **    REASON 1.    If this message goes before the head, FindReceival() will
        **                not be able to return a previous_request, so we cannot use the
        **                usual insertion logic
        **    REASON 2.    Pre-Head insertion is quicker this way
        **
        **    SITUATION 1.    The characteristic number of the new message is less than
        **                    or equal to the characteristic number of the head.
        ****/
        else if (request->GetServerRequestedNumber() < head->GetServerRequestedNumber())
        {
            request->SetNext(head);
            head = request;
            ++numItems;
        }
        else
        {
            previous_request = FindRequest(request);
            if (!descriptor_was_found)
            {
                request->SetNext(previous_request->GetNext());
                previous_request->SetNext(request);
                ++numItems;

                if (request->GetNext() == 0)
                    tail = request;
            }
        }
    }

    void InterprocessDescriptorList::AddReceival(InterprocessDataDescriptor* receival)
    {
        InterprocessDataDescriptor* previous_receival = 0;

        if (head == 0)
        {
            head = tail = receival;
            ++numItems;
        }
        else if (receival->GetClientRequestedNumber() < head->GetClientRequestedNumber())
        {
            receival->SetNext(head);
            head = receival;
            ++numItems;
        }
        else
        {
            previous_receival = FindReceival(receival);
            if (!descriptor_was_found)
            {
                receival->SetNext(previous_receival->GetNext());
                previous_receival->SetNext(receival);
                ++numItems;

                if (receival->GetNext() == 0)
                    tail = receival;
            }
        }
    }

    InterprocessDataDescriptor* InterprocessDescriptorList::Pop(void)
    {
        // make a copy of the pointer to the idd to be popped
        InterprocessDataDescriptor* old_head = head;

        // make the next idd in the list the new head
        if (head != 0)
            head = head->GetNext();
        else
            return (0); // 96.09.04

        // give the old one no reason to think it is part of a list
        old_head->SetNext(0);

        // just make sure the tail is ok.
        if (head == 0)
            tail = 0;

        // update inherited data member
        --numItems;

        return (old_head);
    }

    InterprocessDataDescriptor* InterprocessDescriptorList::RemoveFirstValidReceival(long int time)
    {
        InterprocessDataDescriptor* receival = head;

        // first, do a check of the most likely case - the head is the next valid receival
        if (head->GetTime() <= time)
        {
            head = receival->GetNext();
            receival->SetNext(0);
            return (receival);
        }

        // otherwise, we need to search through the list
        InterprocessDataDescriptor* previous_receival = 0;

        while (receival != 0)
        {
            if (receival->GetTime() <= time)
            {
                /* the current receival is the valid one. The removal logic is easier
                knowing that having got this far, it is not the head of the list */
                previous_receival->SetNext(receival->GetNext());
                receival->SetNext(0);
                return (receival);
            }

            previous_receival = receival;
            receival = previous_receival->GetNext();
        }

        /* This is here just to please the compiler.
        If we get this far, it need only return 0 */
        return (receival);
    }

    /*
    Find
    Tries to find an request with same characteristic number and process.
    The data member descriptor_was_found is adjusted according to whether
    the request was actually present or not. It is set to :

    TRUE if an request with identical characteristic number and process was found
    FALSE if an request as above was NOT found.

    In either case, this function returns a pointer to the request which the
    new (given) request could be inserted AFTER.

    NOTE :    This works because the descriptors are inserted in according
            to their characteristic number                            */
    InterprocessDataDescriptor* InterprocessDescriptorList::FindRequest(InterprocessDataDescriptor* request)
    {
        descriptor_was_found = false;

        if (head != 0) // standard check before doing ANYTHING on the list
        {
            // found = found_descriptor
            InterprocessDataDescriptor* found = head;

            /****
            **    Iterate along the list until we find a descriptor that fits the following :
            **    1. it is the last descriptor
            **    2. its time value is greater than or equal to ÒfindTÓ
            **
            **    That is to say :
            **    WHILE
            **        (this is NOT the last descriptor)
            **    AND
            **        (the next descriptor's time value is LESS THAN requestÕs)
            **    THEN
            **        keep fetching the next descriptor
            ****/
            while ((found->GetNext() != 0) && (found->GetNext()->GetServerRequestedNumber() < request->GetServerRequestedNumber()))
                found = found->GetNext();

            /****
            **    There are three cases we might be looking at from here :-
            **    1.    We are at the end of the list. Thus, we return the descriptor
            **        as "found", and leave descriptor_was_found as FALSE
            **    2.     The next request's characteristic number is greater than (BUT NOT
            **        EQUAL TO) that of the request we are looking for.
            **        Thus, we return "found" as is, and leave descriptor_was_found as FALSE
            **    3.    "found"s characteristic number is EQUAL TO the next request's
            **        characteristic number. Thus, we need to continue searching
            **        for the end of the run of characteristic number with the same
            **        value. HOWEVER we want to stop searching if we find an request
            **        with the same process (AS WELL) already in the queue.
            ****/
            if (found->GetNext() == 0)
            {
                return (found);
            }
            else if (found->GetNext()->GetServerRequestedNumber() > request->GetServerRequestedNumber())
            {
                return (found);
            }
            // (found->GetNext->GetServerRequestedNumber() == request->GetServerRequestedNumber())
            else
            {
                while ((found->GetNext() != 0) && (found->GetNext()->GetServerRequestedNumber() == request->GetServerRequestedNumber()))
                {
                    if (found->GetNext()->GetServer() == request->GetServer())
                    {
                        descriptor_was_found = true;
                        return (found);
                    }

                    found = found->GetNext(); // keep iteratively looking
                }

                if ((found->GetNext() == 0) || (found->GetNext()->GetServerRequestedNumber() > request->GetServerRequestedNumber()))
                {
                    return (found);
                }
            }

            return (found);
        }
        // else (head == 0)
        return (0);
    }

    InterprocessDataDescriptor* InterprocessDescriptorList::FindReceival(InterprocessDataDescriptor* receival)
    {
        descriptor_was_found = false;

        if (head != 0)
        {
            InterprocessDataDescriptor* found = head;

            while ((found->GetNext() != 0) && (found->GetNext()->GetClientRequestedNumber() < receival->GetClientRequestedNumber()))
                found = found->GetNext();

            if (found->GetNext() == 0)
            {
                return (found);
            }
            else if (found->GetNext()->GetClientRequestedNumber() > receival->GetClientRequestedNumber())
            {
                return (found);
            }
            else
            {
                while ((found->GetNext() != 0) && (found->GetNext()->GetClientRequestedNumber() == receival->GetClientRequestedNumber()))
                {
                    if (found->GetNext()->GetServer() == receival->GetServer())
                    {
                        descriptor_was_found = true;
                        return (found);
                    }

                    found = found->GetNext();
                }

                if ((found->GetNext() == 0) || (found->GetNext()->GetClientRequestedNumber() > receival->GetClientRequestedNumber()))
                    return (found);
            }

            return (found);
        }

        return (0);
    }
} /* namespace rootmap */
