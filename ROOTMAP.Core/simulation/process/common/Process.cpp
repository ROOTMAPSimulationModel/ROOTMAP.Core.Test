/////////////////////////////////////////////////////////////////////////////
// Name:        Process.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     27/05/2006
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/Process.h"

#include "simulation/common/IdentifierUtility.h"
#include "simulation/common/SimulationEngine.h"
#include "simulation/common/SprintUtility.h"
#include "simulation/common/Types.h"
#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/file/output/DataOutputFile.h"
#include "simulation/file/output/OutputRule.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/ProcessDrawing.h"
#include "simulation/process/common/ProcessExceptions.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/interprocess_communication/InterprocessDataDescriptor.h"
#include "simulation/process/interprocess_communication/InterprocessDescriptorList.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/process/modules/VolumeObject.h"

#include "core/common/Structures.h"
#include "core/common/RmAssert.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/utility/Utility.h"

#include <exception>


namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(Process, ProcessModuleBase)

        RootMapLoggerDefinition(Process);

    Process::Process()
        : m_id(0)
        , m_scoreboardStratum(ScoreboardStratum::NONE)
        , m_activity(paNONE)
        , m_name("")
        , m_previousTimeStamp(std::numeric_limits<ProcessTime_t>::min())
        , m_lastCoughUp(0)
        , m_delayedReactionActivated(false)
        , m_registeredRequests(0)
        , m_registeredReceivals(0)
        , m_drawing(0)
    {
        RootMapLoggerInitialisation("rootmap.Process");
    }

    void Process::Initialise(const ProcessDAI& data)
    {
        m_id = data.getIdentifier();
        m_scoreboardStratum = data.getStratum();
        m_activity = data.getActivity();
        m_name = data.getName();

        LOG_INFO << "Initialised Process {Name:" << m_name
            << ", ID:" << m_id
            << ", Stratum:" << m_scoreboardStratum.toString()
            << ", Activity:" << Utility::OSTypeToString(m_activity)
            << "}";


        const CharacteristicDAICollection& characteristic_data = data.getCharacteristics();
        for (CharacteristicDAICollection::const_iterator iter = characteristic_data.begin();
            iter != characteristic_data.end();
            ++iter)
        {
            ProcessModuleBase::AddCharacteristic(*(*iter));
        }

        if ((DoesDrawing()) && (0 == m_drawing))
        {
            m_drawing = new ProcessDrawing(this);
        }
    }


    void Process::InitialiseSpecial(const ProcessSpecialDAI& /* data */)
    {
        throw ProcessException("InitialiseSpecial() default implementation reached", m_name.c_str());
    }


    /**
     * Destructor
     */
    Process::~Process()
    {
        delete m_drawing;
        m_drawing = 0;
    }

    void Process::GetProcessName(std::string& s) const
    {
        s = m_name;
    }

    const std::string& Process::GetProcessName() const
    {
        return (m_name);
    }


    bool Process::DoesOverride() const { return (false); }


    long int Process::Terminate(ProcessActionDescriptor* action)
    {
        Use_Scoreboard;
        Use_ReturnValue;

        if (UsesSpecialPerBoxInformation())
            DisposeSpecialInformation(scoreboard);

        return (return_value);
    }

    long int Process::StartUp(ProcessActionDescriptor* action)
    {
        m_previousTimeStamp = action->GetTime();
        return (0);
    }

    long int Process::Ending(ProcessActionDescriptor* action)
    {
        m_previousTimeStamp = action->GetTime();
        return (0);
    }

    void Process::SetCharacteristicIndices(ProcessCoordinator* processcoordinator, VolumeObjectCoordinator* volumeobjectcoordinator, CharacteristicIndices& characteristicIndices, const std::string& name)
    {
        // Look for the Characteristic in an unmodified form.
        CharacteristicIndex volumeObjectIgnorantCI = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume(name);
        if (volumeObjectIgnorantCI == InvalidCharacteristicIndex)
        {
            wxString s = "Could not find characteristic " + name;
            throw new RmException(s);
        }
        characteristicIndices.push_back(volumeObjectIgnorantCI);

        // Then look for variations by VolumeObject        
        std::map<size_t, CharacteristicIndex> tempMap; // Temporary map storage
        for (VolumeObjectList::const_iterator voliter = volumeobjectcoordinator->GetVolumeObjectList().begin();
            voliter != volumeobjectcoordinator->GetVolumeObjectList().end();
            ++voliter)
        {
            std::string n = " VolumeObject " + std::to_string((*voliter)->GetIndex());
            // MSA 11.08.05 Update: if a per-VolumeObject variant is not found for this VolumeObject, 
            // just use the VO-ignorant case.
            // This will only come up in a few limited situations, such as Bulk Density variants.
            CharacteristicIndex ci = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume(name + n);
            tempMap[(*voliter)->GetIndex()] = ci == InvalidCharacteristicIndex
                ? volumeObjectIgnorantCI
                : ci;
        }
        for (size_t i = 1; i <= tempMap.size(); ++i)
        {
            CharacteristicIndex ci = tempMap[i];
            // MSA Note that 0 is in fact a valid CharacteristicIndex, but ci should never equal 0.
            // If this is the first Characteristic being handled, then the "first of the first" will be zero (I think) - 
            // but the first of the first is the VO-ignorant case, handled above.
            RmAssert(ci != InvalidCharacteristicIndex && ci != 0, "Invalid CharacteristicIndex");
            characteristicIndices.push_back(ci);
        }
    }

    long int Process::WakeUp(ProcessActionDescriptor* action)
    {
        Use_Code;
        Use_ReturnValue;

        switch (code)
        {
        case kNormalMessage:
        case kPreNormalMessage:
            return_value = DoNormalWakeUp(action);
            break;

        case kExternalWakeUpMessage:
            return_value = DoExternalWakeUp(action);
            break;

        case kRegisterRequestMessage:
            return_value = DoRegisterRequest(action);
            break;

        case kRegisteredCoughUpMessage:
            return_value = DoNormalReceival(action);
            break;

        case kNormalCoughUpMessage:
            return_value = DoNormalCoughUp(action);
            break;

        case kUnsolicitedCoughUpMessage:
            return_value = DoUnsolicitedReceival(action);
            break;

        case kCoughUpImmediateMessage:
            return_value = DoImmediateCoughUp(action);
            break;

        case kSelfCoughedUpImmediateMessage:
        case kOtherCoughedUpImmediateMessage:
            return_value = DoImmediateReceival(action);
            break;

        case kReceiverDelayedReactionMessage:
            return_value = DoDelayedReceivalReaction(action);
            break;

        case kGlobalPlantSummaryRegisteredMessage:
            return_value = DoGlobalPlantSummaryRegistered(action);
            break;

        case kScoreboardPlantSummaryRegisteredMessage:
            return_value = DoScoreboardPlantSummaryRegistered(action);
            break;

        case kSpecialInputDataMessage:
            return_value = DoSpecialInput(action);
            break;

        case kSpecialOutputDataMessage:
            return_value = DoSpecialOutput(action);
            break;

        case kGeneralPurposeMessage0:
        case kGeneralPurposeMessage1:
        case kGeneralPurposeMessage2:
        case kGeneralPurposeMessage3:
        case kGeneralPurposeMessage4:
        case kGeneralPurposeMessage5:
        case kGeneralPurposeMessage6:
        case kGeneralPurposeMessage7:
        case kGeneralPurposeMessage8:
        case kGeneralPurposeMessage9:
            return_value = DoGeneralPurpose(action);
            break;

        case kNullMessage:
        default:
            break;
        }

        // re-send this message if it was a periodic waking
        Use_Time;
        ProcessTime_t periodic_delay = GetPeriodicDelay(code);
        if (periodic_delay > 0)
        {
            Use_PostOffice;
            Send_GeneralPurpose(time + periodic_delay, this, __nullptr, code);
            LOG_INFO << "Sent periodic message to self {Name:" << m_name
                << ", Message:" << SprintUtility::StringOf(code)
                << ", Time:" << time
                << "}";
        }
        else
        {
            LOG_DEBUG << "No periodic wakeup set {Process:" << m_name
                << ", Message:" << SprintUtility::StringOf(code)
                << ", Time:" << time
                << "}";
        }

        m_previousTimeStamp = time;
        return (return_value);
    }

    /* DoNormalWakeUp
    Called when a 'Nrml' message is received.*/
    long int Process::DoNormalWakeUp(ProcessActionDescriptor* /* action */)
    {
        Use_ReturnValue;

        // As part of normal actions, we should always try
        // to give requesting process modules their dues
        // RvH: DoRegisterRequest now sets a periodic wakeup task specially
        // for DoNormalCoughUp
        //if ((m_registeredRequests != 0) && (m_registeredRequests->GetNumItems() > 0))
        //{
        //    DoNormalCoughUp(action);
        //}

        // if we DO have registered periodic tasks, we should do two things :
        // 1. Make sure all non-self caused immediate receivals are received
        // 2. Re-set the alarm
        //    
        // NOTE: deprecated by more general behaviour in WakeUp()
        //if (m_registeredPeriodicTasksActivated)
        //{
        //    Use_PostOffice;
        //    Send_WakeUp_Message(time + GetPeriodicDelay(0));
        //}

        return (return_value);
    }

    /* DoExternalWakeUp
    Called when a 'XWak' message is received. Sent by another process module,
    to wake "this" one up to do some processing based on the other process'
    action[s]. This default method does nothing, you'll need to override it
    for your process module to accept wake up calls from other process modules.*/
    long int Process::DoExternalWakeUp(ProcessActionDescriptor* /*action*/)
    {
        return (kNoError);
    }

    /* DoRegisterRequest
    Called when a 'RgRq' message is received. Registers another process'
    request for a characteristic that this process module controls.*/
    long int Process::DoRegisterRequest(ProcessActionDescriptor* action)
    {
        InterprocessDataDescriptor* request = (InterprocessDataDescriptor*)(action->GetData());

        // Register this request by adding it to the list of requests.
        // If there are no requests yet, make it the head of the list
        AppendRequest(request);

        // Set to periodically cough up any requested characteristics
        //SetPeriodicWaking(action);
        //
        SetPeriodicDelay(kNormalCoughUpMessage, PostOffice::CalculateTimeSpan(0, 0, 0, 12, 0, 0)); // 12 hours

        return (kNoError);
    }

    /* DoNormalCoughUp
    Called as part of the default action when a 'Nrml' message is received
    The intent here is to dish-out, or "cough-up" amounts of the
    characteristics that this process module controls, to any other
    process modules that might have requested some.

    We need to account for the possibility of being called upon to do
    a cough-up twice during the same timestamp, that is one Normal and
    one Immediate cough-up. Of course there is no point in actually
    dishing out quantities the second time because they will all be
    zero (quantity = amount earnt since last cough up). If the Normal
    cough-up is called first, that's fine since all clients will
    handle the receival as it arrives. If the Immediate cough-up
    occurs first, only the requesting process module will act, the others
    will wait until their next Normal awakening. This is of course
    unacceptable. The solution as i see it is to :
    IGNORE IT :- this situation should not be possible, since a Normal
    cough-up should always be scheduled ahead of time, whereas an
    Immediate cough-up will certainly apply to the current timestamp.
    This implies that a ÒNormalÓ will always preceed an ÓImmediateÓ on
    the message list. */
    long int Process::DoNormalCoughUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        return_value = DoCoughUp(action);

        if (return_value == kNoError)
        {
            SendCoughUps(action);
        }
        else if (return_value == kAlreadyCoughedUpThisTimestamp)
        {
            // DoNothing(), although this should never occur
        }
        else if (return_value == kNoRegisteredRequests)
        {
            // DoNothing()
        }

        return (return_value);
    }

    /* DoImmediateCoughUp
    Called when a 'CuIm' message is received.

    We need to account for the possibility of being called upon to do
    a cough-up twice or more during the same timestamp. This would
    involve at most one Normal (registered, scheduled) cough-up, and
    any number of immediate cough-ups. Having a normal then a run
    immediates is ok : when the client uses AppendIDD() to add the
    receival to m_registeredReceivals, it will/should eliminate them.
    The problem is that the only process module that will act upon it
    immediately is the first "CuIm" requestor. Somehow we need
    to prioritise cough-ups. In fact, this is done using
    InterprocessDescriptorList via AppendRequest() & AppendReceival() */
    long int Process::DoImmediateCoughUp(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        return_value = DoCoughUp(action);

        if (return_value == kNoError)
        {
            SendCoughUps(action);
        }
        else if (return_value == kAlreadyCoughedUpThisTimestamp)
        {
            // DoNothing()
        }
        else if (return_value == kNoRegisteredRequests)
        {
            // DoNothing()
        }

        return (return_value);
    }

    /* DoCoughUp
    Note that this version of the function uses a technique of caching
    relevant characteristic indices, looping through the scoreboard only
    once (per requested characteristic number), looping thru' the requests
    for A PARTICULAR characteristic in each box adding total requested
    amounts and allocating cough-ups there and then.

    A totally different technique would avoid the caching, but would require
    looping through the scoreboard for each request plus one loop for
    allocating cough-ups AND THEN again for filling in each cough-up. */
    long int Process::DoCoughUp(ProcessActionDescriptor* action)
    {
        // shortcut out of here if no requests to cough up
        if ((m_registeredRequests == 0) || (m_registeredRequests->IsEmpty()))
            return (kNoRegisteredRequests);

        // ProcessActionDescriptor variables
        Use_ReturnValue;
        Use_Time;
        Use_Scoreboard;

        // 
        register InterprocessDataDescriptor* request = m_registeredRequests->GetFirst();
        register InterprocessDataDescriptor* first_related_request = 0;
        register long int current_characteristic_number;
        register long int current_characteristic_index;

        // index caching variables
        register long int number_of_current_requests;
        register long int* cached_request_rate_indices = 0;
        register long int* cached_buffer_indices = 0;
        register double* cached_request_amounts = 0; // note that the AMOUNT, not the rate, is cached. This is to speed up buffer
        register long int temporary_index;

        // initialise variables for looping round the scoreboard boxes
        register double proportion_to_be_coughed_up, total_requested_from_this_box;
        const register double days_since_last_cough_up = ((time - m_lastCoughUp) / PROCESS_DAYS(1));

        if (m_lastCoughUp == time) return (kAlreadyCoughedUpThisTimestamp);

        // this outer-most loop is executed once for every different characteristic_number requested
        while (request != 0)
        {
            // intialise some variables
            first_related_request = request;
            current_characteristic_number = (first_related_request != 0) ? first_related_request->GetServerRequestedNumber() : -1;
            current_characteristic_index = GetCharacteristicIndex(current_characteristic_number);
            number_of_current_requests = 0;

            // count consecutive requests with the same characteristic number
            while ((request) && (request->GetServerRequestedNumber() == current_characteristic_number))
            {
                ++number_of_current_requests;
                request = request->GetNext();
            }

            // create an array of characteristic indices. The array is 
            // big enough for indices of both requests & buffers.
            // All request indices come first, then buffer indices.
            request = first_related_request;
            cached_request_rate_indices = (long*)calloc(number_of_current_requests, sizeof(long int));
            cached_buffer_indices = (long*)calloc(number_of_current_requests, sizeof(long int));
            cached_request_amounts = (double*)calloc(number_of_current_requests, sizeof(double));
            for (temporary_index = 0; temporary_index < number_of_current_requests; ++temporary_index)
            {
                cached_request_rate_indices[temporary_index] = request->GetClientRequestRateIndex();
                cached_buffer_indices[temporary_index] = request->GetClientReceiveBufferIndex();
                request = request->GetNext();
            }

            // scoreboard box iterator
            BoxIndex box_index = 0;
            BoxIndex box_max = scoreboard->GetNumberOfBoxes();

            // For each scoreboard box,É
            while (box_index < box_max)
            {
                // reset the total cumulated requested amounts for this box
                total_requested_from_this_box = 0.0;

                // total the requests, which are scaled by the time since last coughing up
                for (temporary_index = 0; temporary_index < number_of_current_requests; ++temporary_index)
                {
                    total_requested_from_this_box += (cached_request_amounts[temporary_index] = (scoreboard->GetCharacteristicValue(cached_request_rate_indices[temporary_index], box_index) * days_since_last_cough_up));
                }

                // Calculate the amount that will actually be coughed up
                proportion_to_be_coughed_up = ProportionAllocated(total_requested_from_this_box, scoreboard->GetCharacteristicValue(current_characteristic_index, box_index));

                for (temporary_index = 0; temporary_index < number_of_current_requests; ++temporary_index)
                {
                    // Give each request the proportion of the amount
                    scoreboard->AddCharacteristicValue(cached_buffer_indices[temporary_index], (cached_request_amounts[temporary_index] * proportion_to_be_coughed_up), box_index);
                    // And take away from whence the amount comes
                    scoreboard->AddCharacteristicValue(current_characteristic_index, -(cached_request_amounts[temporary_index] * proportion_to_be_coughed_up), box_index);
                }
            }
        }

        m_lastCoughUp = time;

        free(cached_request_rate_indices);
        free(cached_buffer_indices);
        free(cached_request_amounts);

        return (return_value);
    }

    /* SendCoughUps
    Used by functions that call DoCoughUp(), to deliver the message to all the
    process modules that are about to receive a cough up */
    long int Process::SendCoughUps(ProcessActionDescriptor* action)
    {
        // shortcut out of here if nothing to do
        if ((m_registeredRequests == 0) || (m_registeredRequests->GetNumItems() < 1))
            return (kNoRegisteredRequests);

        // some action variables
        Use_PostOffice;
        Use_ReturnValue;
        Use_Code;
        Use_Data;
        Use_Time;

        InterprocessDataDescriptor* request = m_registeredRequests->GetFirst();
        InterprocessDataDescriptor* requestor = 0;
        InterprocessDataDescriptor* cough_up = 0;
        bool requestor_was_a_regular = false;

        if (code == kCoughUpImmediateMessage)
        {
            requestor = (InterprocessDataDescriptor*)data;

            while (request != 0)
            {
                if (request != requestor)
                {
                    /* there are two differences between the requestor that caused the
                    immediate    cough up 1) it needs to be sent a different message code, and
                    2) we don't need to make a seperate descriptor, since we can re-use
                    the descriptor that caused the cough up. However since we are reusing
                    the descriptor, we need to refresh its time stamp. */
                    postoffice->sendMessage(time, this, request->GetClient(), requestor, kSelfCoughedUpImmediateMessage);
                    requestor_was_a_regular = true;
                    request->Refresh(time);
                }
                else
                {
                    cough_up = MakeCoughUpDescriptor(request, time);
                    postoffice->sendMessage(time, this, request->GetClient(), cough_up, kOtherCoughedUpImmediateMessage);
                }

                request = request->GetNext();
            }

            if (!requestor_was_a_regular)
            {
                /* then the process module that caused the immediate cough up was not a
                registered requestor. What we don't need to do is make a seperate
                descriptor, since the requestor is not part of our list anyway.
                Although once again remembering to refresh the time stamp. */
                postoffice->sendMessage(time, this, requestor->GetClient(), requestor, kSelfCoughedUpImmediateMessage);
                requestor->Refresh(time);
            }
        }
        else if (code == kNormalMessage)
        {
            while (request != 0)
            {
                cough_up = MakeCoughUpDescriptor(request, time);
                //            Send_Registered_CoughUp(request->GetClient(), cough_up);
                postoffice->sendMessage(time, this, request->GetClient(), cough_up, kRegisteredCoughUpMessage);
                request = request->GetNext();
            }
        }

        return (return_value);
    }

    /* DoImmediateReceival
    Called when a 'SCui' or 'OCui' message is received. */
    long int Process::DoImmediateReceival(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        return_value = DoReceive(action);

        return (return_value);
    }

    /* DoNormalReceival
    Called when a 'RgCu' message is received. */
    long int Process::DoNormalReceival(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        return_value = DoReceive(action);

        return (return_value);
    }

    long int Process::DoUnsolicitedReceival(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        return_value = DoReceive(action);

        return (return_value);
    }

    /* DoReceive
    Common acts for the DoReceiveXXXXXCoughUp() functions */
    long int Process::DoReceive(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_PostOffice;
        Use_Time;

        if (!m_delayedReactionActivated)
        {
            postoffice->sendMessage(time, this, this, 0, kReceiverDelayedReactionMessage);
            m_delayedReactionActivated = true;
        }

        AppendReceival((InterprocessDataDescriptor*)(action->GetData()));

        return (return_value);
    }

    /* DoDelayedReceivalReaction
    Called when a 'ReDR' message is received

    All the receivals for a particular characteristic will have put their information into
    the one same buffer, so we only need to transfer once for each buffer. The tricky bit,
    then, is figuring out where the receivals for one buffer end and the next stop. */
    long int Process::DoDelayedReceivalReaction(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;
        Use_Scoreboard;

        // First ensure we haven't already dealt with delayed
        // receivals already in this timestamp. JUST IN CASE, YOU KNOW?
        if ((m_registeredReceivals == 0) || (m_registeredReceivals->GetNumItems() < 1))
            return (noErr);

        // indices. For obvious reasons
        CharacteristicIndex buffer_index;
        CharacteristicIndex destination_index;

        // a pointer for traversing the list of receivals
        //InterprocessDataDescriptor* next_receival = 0;    // MSA 09.10.07 Unused
        InterprocessDataDescriptor* receival = 0;
        receival = m_registeredReceivals->Pop();

        // this outer loop should, hopefully, be executed once for each
        // receival to be dealt with.  It is done this way because it is anticipated that no more than a few receivals 
        // will be waiting to be dealt with at the end of any one timestamp.
        while (receival != 0)
        {
            destination_index = receival->GetClientRequestedIndex();
            buffer_index = receival->GetClientReceiveBufferIndex();

            BoxIndex box_index = 0;
            BoxIndex box_max = scoreboard->GetNumberOfBoxes();

            while (box_index < box_max)
            {
                scoreboard->TransferAndClearCharacteristicValue(buffer_index, destination_index, box_index);
            }

            TCLForgetObject(receival);
            receival = m_registeredReceivals->Pop();
        } // while (receival != 0)

        m_delayedReactionActivated = false;

        return (return_value);
    }

    //void Process::SetPeriodicWaking(ProcessActionDescriptor *action)
    //{
    //    if (!m_registeredPeriodicTasksActivated)
    //    {
    //        Use_PostOffice;
    //        Use_Time;
    //
    //        long periodic_delay = GetPeriodicDelay(0);
    //        RmAssert( 0<periodic_delay, "Process must override GetPeriodicDelay() to allow periodic tasks to be activated");
    //
    //        Send_WakeUp_Message(time + periodic_delay);
    //
    //        m_registeredPeriodicTasksActivated = true;
    //    }
    //}

    long int Process::DoGlobalPlantSummaryRegistered(ProcessActionDescriptor* /*action*/)
    {
        Use_ReturnValue;

        return (return_value);
    }

    long int Process::DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* /*action*/)
    {
        Use_ReturnValue;

        return (return_value);
    }

    /* DoSpecialInput
    Called when a 'Inpu' message is received.
    */
    long int Process::DoSpecialInput(ProcessActionDescriptor* /*action*/)
    {
        return (kNoError);
    }

    // derived process classes have the option of either overriding this main
    // multiplexer function, or each specific function
    long int Process::DoGeneralPurpose(ProcessActionDescriptor* action)
    {
        long int return_value = kNoError;

        switch (action->GetCode())
        {
        case kGeneralPurposeMessage0:
            return_value = DoGeneralPurpose0(action);
            break;
        case kGeneralPurposeMessage1:
            return_value = DoGeneralPurpose1(action);
            break;
        case kGeneralPurposeMessage2:
            return_value = DoGeneralPurpose2(action);
            break;
        case kGeneralPurposeMessage3:
            return_value = DoGeneralPurpose3(action);
            break;
        case kGeneralPurposeMessage4:
            return_value = DoGeneralPurpose4(action);
            break;
        case kGeneralPurposeMessage5:
            return_value = DoGeneralPurpose5(action);
            break;
        case kGeneralPurposeMessage6:
            return_value = DoGeneralPurpose6(action);
            break;
        case kGeneralPurposeMessage7:
            return_value = DoGeneralPurpose7(action);
            break;
        case kGeneralPurposeMessage8:
            return_value = DoGeneralPurpose8(action);
            break;
        case kGeneralPurposeMessage9:
            return_value = DoGeneralPurpose9(action);
            break;
        default:
            LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " Unexpected";
            return_value = kInvalidMessage;
            break;
        }
        return (return_value);
    }

    long int Process::DoGeneralPurpose0(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose1(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose2(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose3(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose4(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose5(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose6(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose7(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose8(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }

    long int Process::DoGeneralPurpose9(ProcessActionDescriptor* action)
    {
        LOG_WARN << "General Purpose Message " << Utility::OSTypeToString(action->GetCode()) << " not handled";
        return (kNoError);
    }


    /* DoSpecialOutput
    Called when a 'Outp' message is received. The message is sent by a JDataOutputFile
    descendent, when special exporting is to be done. The protocol is : In the "GetData()'
    field of the action is a handle to important information. Like the pointer to the file
    object, and exactly what the two lines of specifications were in the associations file, */
    long int Process::DoSpecialOutput(ProcessActionDescriptor* action)
    {
        OutputRule* orule = dynamic_cast<OutputRule *>(action->GetData());

        // if its ok, just add it to the list - we will send the first alarm
        // during simulation initialisation stage
        if (0 != orule)
        {
            orule->OutputToFile(action);
        }
        else
        {
            LOG_WARN << LOG_LINE << "Process::DoSpecialOutput ProcessActionDescriptor did not contain an OutputRule as its data";
        }

        return (kNoError);
    }

    void Process::SetPeriodicDelay(MessageType_t mType, ProcessTime_t t)
    {
        // sets if already 
        m_periodicWaking[mType] = t;
    }


    ProcessTime_t Process::GetPeriodicDelay(MessageType_t mType) const
    {
        MessageTimeMap::const_iterator found(m_periodicWaking.find(mType));
        if (m_periodicWaking.end() != found)
        {
            return found->second;
        }

        return 0;
    }

    void Process::SetTimeDeltaTracking(MessageType_t mType)
    {
        m_timeDeltaTracking[mType] = 0;
    }

    ProcessTime_t Process::GetTimeDelta(MessageType_t mType) const
    {
        MessageTimeMap::const_iterator found(m_timeDeltaTracking.find(mType));
        if (m_timeDeltaTracking.end() != found)
        {
            return found->second;
        }

        return 0;
    }

    ProcessTime_t Process::dT() const
    {
        return GetTimeDelta(kNormalMessage);
    }

    /* MakeDescriptor
    This version of the function is for when only the name is known.
    It transfers the name to the relevant process module variables
    NOTE : assumes "this" object is the client */
    InterprocessDataDescriptor* Process::MakeRequestDescriptor
    (const std::string& requested_characteristic_name,
        long int client_requested_number,
        long int client_request_rate_number,
        long int client_receive_buffer_number,
        ProcessActionDescriptor* action
    )
    {
        Use_ProcessCoordinator;

        long int server_requested_number = 0;
        Process* server_process = Find_Process_By_Characteristic_Name(requested_characteristic_name, server_requested_number);

        return (MakeRequestDescriptor(server_process, server_requested_number, client_requested_number, client_request_rate_number, client_receive_buffer_number, action));
    }

    /* NOTE : assumes "this" object is the client */
    InterprocessDataDescriptor* Process::MakeRequestDescriptor
    (Process* server_process,
        long int server_requested_number,
        long int client_requested_number,
        long int client_request_rate_number,
        long int client_receive_buffer_number,
        ProcessActionDescriptor* action
    )
    {
        InterprocessDataDescriptor* descriptor = 0;

        descriptor = new InterprocessDataDescriptor
        (
            server_process,
            server_requested_number,
            server_process->GetCharacteristicIndex(server_requested_number),
            this,
            client_requested_number,
            GetCharacteristicIndex(client_requested_number),
            client_request_rate_number,
            GetCharacteristicIndex(client_request_rate_number),
            client_receive_buffer_number,
            GetCharacteristicIndex(client_receive_buffer_number),
            action->GetTime()
        );

        return (descriptor);
    }

    /*
    server_source_number        - The number of the source characteristic
    client_destination_name    - The name of the characteristic where the substance ends up
    client_buffer_name            - The characteristic nominated as the intermediate destination
    action*/
    InterprocessDataDescriptor* Process::MakeCoughUpDescriptor
    (long server_source_number,
        const std::string& client_destination_name,
        const std::string& client_buffer_name,
        ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;

        Process* client_process = 0;
        long client_buffer_number;
        long client_destination_number;

        client_process = Find_Process_By_Characteristic_Name(client_destination_name, client_destination_number);
        client_process = Find_Process_By_Characteristic_Name(client_buffer_name, client_buffer_number);

        InterprocessDataDescriptor* descriptor = 0;

        descriptor = new InterprocessDataDescriptor
        (
            this,
            server_source_number,
            GetCharacteristicIndex(server_source_number),
            client_process,
            client_destination_number,
            client_process->GetCharacteristicIndex(client_destination_number),
            client_buffer_number,
            client_process->GetCharacteristicIndex(client_buffer_number),
            action->GetTime()
        );

        return (descriptor);
    }

    InterprocessDataDescriptor* Process::MakeCoughUpDescriptor(InterprocessDataDescriptor* idd, long time)
    {
        InterprocessDataDescriptor* descriptor = 0;

        descriptor = new InterprocessDataDescriptor(idd, time);

        return (descriptor);
    }

    void Process::SendRequest
    (InterprocessDataDescriptor* request,
        bool is_registered_not_immediate,
        ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_PostOffice;

        long message_type = (is_registered_not_immediate) ? kRegisterRequestMessage : kCoughUpImmediateMessage;

        postoffice->sendMessage(time, this, request->GetServer(), request, message_type);
    }

    void Process::SendUnsolicitedCoughUp
    (InterprocessDataDescriptor* coughup,
        ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_PostOffice;

        InterprocessDataDescriptor* new_cough_up = 0;

        new_cough_up = MakeCoughUpDescriptor(coughup, time);
        postoffice->sendMessage(time, this, new_cough_up->GetClient(), new_cough_up, kUnsolicitedCoughUpMessage);
    }

    void Process::AppendRequest(InterprocessDataDescriptor* new_request)
    {
        if (m_registeredRequests == 0)
            m_registeredRequests = new InterprocessDescriptorList;

        m_registeredRequests->AddRequest(new_request);
        new_request->SetRegistered(true);
    }

    void Process::AppendReceival(InterprocessDataDescriptor* new_receival)
    {
        if (m_registeredReceivals == 0)
            m_registeredReceivals = new InterprocessDescriptorList;

        m_registeredReceivals->AddReceival(new_receival);
        new_receival->SetInUse(true);
    }


    /* ProportionAllocated
    Called by DoCoughUp for each box*/
    double Process::ProportionAllocated(double amount_requested, double amount_available)
    {
        if ((amount_available > 0.0) && (amount_requested > 0.0))
            return (Utility::CSMin(1.0, (double)amount_available / (double)amount_requested));
        //    else
        return (0.0);
    }

    void Process::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* /* scoreboard */)
    {
        // not all processes draw, so this can't be made pure virtual, however we
        // do want to enforce that any process that *does* draw must override
        // this function
        RmAssert((!DoesDrawing()), "Process that DoesDrawing() must override DrawScoreboard()");
    }


    ProcessIdentifier Process::GetProcessID() const
    {
        return (m_id);
    }


    long int Process::Register(ProcessActionDescriptor* /*action*/)
    {
        //#pragma unused (action)
        return (0);
    }

    long int Process::Initialise(ProcessActionDescriptor* /*action*/)
    {
        //#pragma unused (action)
        return (0);
    }

    bool Process::DoesSaveProcessData() const
    {
        return (false);
    }

    bool Process::DoesDrawing() const
    {
        return (false);
    }

    bool Process::DoesRaytracerOutput() const
    {
        return (false);
    }

    long int Process::DoRaytracerOutput(RaytracerData* /* raytracerData */)
    {
        RmAssert(false, "Process that DoesRaytracerOutput() must override DoRaytracerDataOutput()");
        return kInvalidMessage;
    }

    long int Process::DoActivateRaytracerOutput()
    {
        // MSA 11.04.04 It is generally OK for this base class method to be called.
        // Only a few derived classes - most importantly Plant - 
        // actually require pre-activation. 
        // Most classes (e.g. ScoreboardRenderer) just take a snapshot when DoRaytracerOutput() is called,
        // and don't have any other Raytracer-specific functionality that needs to be activated in advance.
        return kNoError;
    }

    // Override this if your process does raytracer output
    DoubleCoordinate Process::GetOrigin() const
    {
        return DoubleCoordinate(0.0, 0.0, 0.0);
    }

    // Override this if your process does barrier modelling
    void Process::SetVolumeObjectCoordinator(const VolumeObjectCoordinator&)
    {
    }
} /* namespace rootmap */


