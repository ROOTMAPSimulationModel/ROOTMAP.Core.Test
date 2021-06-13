//Heritage
#include "simulation/process/common/Process.h"
#include "simulation/process/modules/OrganicMatter.h"
#include "simulation/process/modules/OrganicMatterInput.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/modules/VolumeObject.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/utility/Utility.h"

#include "simulation/process/interprocess_communication/InterprocessDataDescriptor.h"


#include <stdlib.h>
#include <math.h>

namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(OrganicMatter, Process)

        // IMPLEMENT_FORCE_DYNAMIC_CLASS_USAGE(OrganicMatter)
        //const OrganicMatter OrganicMatter::ForceClassUsageOrganicMatter;

        OrganicMatter::OrganicMatter()
        : Process()
        , number_of_inputs(0)
        , mineral_nitrogen_index(InvalidCharacteristicIndex)
        , mineralisation_cough_up(0)
        , evolved_carbon_cough_up(0)
        , immobilisation_request(0)
        , time_of_previous_decay(0)
    {
    }

    OrganicMatter::~OrganicMatter()
    {
        delete mineralisation_cough_up;
        delete evolved_carbon_cough_up;
    }

    bool OrganicMatter::DoesOverride() const { return (true); }

    long int OrganicMatter::Initialise(ProcessActionDescriptor* action)
    {
        Use_ProcessCoordinator;
        Use_Scoreboard;

        // MSA 09.11.09 Determine the Scoreboard dimensions here, save doing it repeatedly
        Get_Number_Of_Layers(NUM_X, NUM_Y, NUM_Z); // number of layers in each dimension
        BOX_COUNT = scoreboard->GetNumberOfBoxes(); // How many boxes do we have?

        /*    Game Plan #1 :
            Make a request for immobilisation (ammonium, our source of mineral nitrogen) */
        immobilisation_request = MakeRequestDescriptor
        ("Ammonium",
            ORGANIC_NITROGEN_NUMBER,
            ORGANIC_NITROGEN_REQUESTED_NUMBER,
            ORGANIC_NITROGEN_RECEIVED_BUFFER,
            action);

        /*    Game Plan #2 :
            Make a descriptor ready to send/cough-up (unsolicited) mineralisation */
        mineralisation_cough_up = MakeCoughUpDescriptor
        (ORGANIC_NITROGEN_NUMBER,
            "Ammonium",
            "Ammonium Received Buffer",
            action);

        /*    Game Plan #3 :
            Make a descriptor ready to send/cough-up (unsolicited) respiration */
        evolved_carbon_cough_up = MakeCoughUpDescriptor
        (ORGANIC_CARBON_NUMBER,
            "Cumulative Evolved CO2",
            "CO2 Received Buffer",
            action);

        /*    Game Plan #4 :
            Figure out the scoreboard index of the mineral nitrogen characteristic */
        mineral_nitrogen_index = Find_Characteristic_Index_By_Characteristic_Name_Same_Volume("Ammonium");

        /*    Game Plan #5 :
            Send the request for immobilisation */
        if (immobilisation_request != 0)
        {
            SendRequest(immobilisation_request, true, action);
        }

        return (Process::Initialise(action));
    }

    /****
    **    StartUp
    **
    **    This function is called at the beginning of each “Go” run to establish that
    **    activity is to commence.
    **
    **    In the case of Organic Matter, we check to see if we have any organic inputs and
    **    if so, organise to wake up in the near future to act upon them
    ****/
    long int OrganicMatter::StartUp(ProcessActionDescriptor* action)
    {
        Process::StartUp(action);
        return (0);
    }

    /****
    **    Ending
    **
    **    This function is called at the end of each “Go” run, to signify that activity is halting.
    **    Messages can still be sent, and scoreboard processing may still occur, but messages
    **    will not be acted upon until the next run (at the earliest).
    ****/
    long int OrganicMatter::Ending(ProcessActionDescriptor* action)
    {
        Process::Ending(action);
        return (0);
    }

    /****
    **    WakeUp
    **
    **    This function is called each time the PostOffice finds a message addressed to this
    **    process. This is the main means of communication between processes and/or characteristics.
    **
    **    Of course, each process will have its own properties and idiosyncrasies which will
    **    determine its actions when it receives a message. Let us inspect this OrganicMatter
    **    process as an example.
    **
    **    It may have received a WakeUp() call for one of a number of reasons. Those known
    **    or envisaged at the moment are:
    **        • self-requested (kNormalMessage), in which case the action to take should be already known.
    **        • to receive input from the POrganicInput process (kOrganicInputMessage).
    **        • to receive input from the PAmmonium or PNitrate processes
    **        • to arbitrate the giving out of an amount of any of the characteristics it owns
    **
    **    Thus, a good start would be to see what type of message has been received then
    **    call more specific functions to handle those messages.
    ****/
    long int OrganicMatter::WakeUp(ProcessActionDescriptor* action)
    {
        Use_Code;
        Use_ReturnValue;

        if (code == kSpecialInputDataMessage)
        {
            return_value = DoOrganicInputMessage(action);
        }

        // ALWAYS make sure to do the inherited stuff before going anywhere else.
        return_value = Process::WakeUp(action);

        return (return_value);
    }

    /* This execution of this function was chosen to trigger the biological actiom
    af this process, because it means that everything is ready for action - we don't
    need to wait for any more stuff to arrive

    There are three (3) things this function needs to calculate for each box
    1. Evolved Carbon (CO2, Respiration)
    2. Mineralisation
    3. The new amount of Nitrogen requested

    After having done those things for each box, it needs to send (unsolicited)
    cough-ups of mineralisation and respiration off */
    long int OrganicMatter::DoDelayedReceivalReaction(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        // First, we hijack the received amounts to put them into out inputs
        return_value = TransferReceivalsToInputs(action);
        if (return_value != kNoError) return (return_value);

        // Then we transfer to the scoreboard via inherited functionality
        return_value = Process::DoDelayedReceivalReaction(action);
        if (return_value != kNoError) return (return_value);

        // Finally, we Act upon the stuff that has just been given to us.
        return_value = DecayInputs(action);

        return (return_value);
    }

    long int OrganicMatter::TransferReceivalsToInputs(ProcessActionDescriptor* action)
    {
        Use_Time;
        Use_Scoreboard;
        Use_ReturnValue;
        Use_VolumeObjectCoordinator;

        // The time units (days) since the last *decay*
        const long dT = ((time - time_of_previous_decay) / PROCESS_DAYS(1));

        // FastTrak out of here if no reason to decay
        if (dT <= 0) return (kNoError);

        // This variable is part of an optimisation technique which works
        // best when there are no "inputs" at the bottom of the scoreboard
        long int inputs_visited = 0;
        OrganicMatterInput* input = 0;

        /* This is for storing the value equal to the actual amount we
        would have received, had we received ALL we asked for. This is
        (requested_rate * dT) */
        const long requested_rate_index = GetCharacteristicIndex(ORGANIC_NITROGEN_REQUESTED_NUMBER);
        const long received_buffer_index = GetCharacteristicIndex(ORGANIC_NITROGEN_RECEIVED_BUFFER);
        const long special_info_index = GetSpecialCharacteristicIndex();
        double proportion_received;
        double amount_requested;


        // In each box, we start by calculating what proportion of the requested
        // immobilisation we actually received. Then we cycle recursively
        // through the inputs in the current box, allocating amounts.
        for (BoxIndex box_index = 0; (inputs_visited < number_of_inputs) && (box_index < BOX_COUNT); ++box_index)
        {
            SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN

                // Its very important to check if we asked for anything, otherwise we
                // end up dividing by zero (even if we received more than zero).
                // An example of this is the first time this function is run. It comes
                // before DecayInputs(), so nothing will have been requested yet.
                if ((amount_requested = scoreboard->GetCharacteristicValue(requested_rate_index, box_index)) == 0.0)
                {
                    proportion_received = 0.0;
                }
                else
                {
                    proportion_received = scoreboard->GetCharacteristicValue(received_buffer_index, box_index) / (amount_requested * dT);
                }

            input = (OrganicMatterInput*)scoreboard->GetSpecialPointer(special_info_index, box_index);
            while (input != 0)
            {
                // Give some immoblised Nitrogen to this input
                input->AddNitrogen(input->GetRequestedNitrogen() * proportion_received);

                // recursion & bookkeeping
                input = input->GetNextInput();
                ++inputs_visited;
            }

            SUBSECTION_ITER_WITH_PROPORTIONS_END
        }

        return (return_value);
    }

    long int OrganicMatter::DecayInputs(ProcessActionDescriptor* action)
    {
        Use_ReturnValue;

        /*    —    —    —    —    —    —    —    —    —    —    —    — *
                    VARIABLE DECLARATIONS
        *    —    —    —    —    —    —    —    —    —    —    —    — */
        // declaration of common simulation items
        Use_Time;
        Use_Scoreboard;

        // local copy of an appropriate value
        const long int organic_nitrogen_requested_index = GetCharacteristicIndex(ORGANIC_NITROGEN_REQUESTED_NUMBER);
        const long int organic_nitrogen_index = GetCharacteristicIndex(ORGANIC_NITROGEN_NUMBER);
        const long int organic_carbon_index = GetCharacteristicIndex(ORGANIC_CARBON_NUMBER);
        const long special_info_index = GetSpecialCharacteristicIndex();
        long int mineralisation_buffer_index = 0;
        long int CO2_buffer_index = 0;

        mineralisation_buffer_index = mineralisation_cough_up->GetClientReceiveBufferIndex();

        CO2_buffer_index = evolved_carbon_cough_up->GetClientReceiveBufferIndex();

        // The time units (days) since the last *decay*
        const long dT = ((time - time_of_previous_decay) / PROCESS_DAYS(1));

        // FastTrak out of here if no reason to decay
        if (dT <= 0) return (kNoError);

        // The things that our organic inputs use & abuse, and their cumulative counterparts
        double nitrogen_requested = 0.0, mineral_N = 0.0, evolved_carbon = 0.0;
        double total_nr, total_mn, total_ec = 0.0;

        // Here we go loop-de-loop,...
        long int inputs_visited = 0;
        OrganicMatterInput* input = __nullptr;

        BoxIndex box_index = 0;

        while ((inputs_visited < number_of_inputs) && (box_index < BOX_COUNT))
        {
            input = (OrganicMatterInput*)scoreboard->GetSpecialPointer(special_info_index, box_index);

            total_nr = total_mn = total_ec = 0.0;

            while (input != __nullptr)
            {
                input->Decay(&nitrogen_requested, &mineral_N, &evolved_carbon, dT);

                total_nr += nitrogen_requested;
                total_mn += mineral_N;
                total_ec += evolved_carbon;

                //if ((isnan(nitrogen_requested)) || (isnan(evolved_carbon)) || (isnan(mineral_N)))
                //    Debugger();
                input = input->GetNextInput();
                ++inputs_visited;
            }

            /****
            **     update the scoreboard with the new summed values
            ****/
            // SET the new rate of nitrogen we want in this box
            scoreboard->SetCharacteristicValue(organic_nitrogen_requested_index, total_nr, box_index);
            // REMOVE the amount of organic nitrogen that has mineralized
            scoreboard->AddCharacteristicValue(organic_nitrogen_index, -total_mn, box_index);
            // REMOVE the amount of carbon that has evolved/respired
            scoreboard->AddCharacteristicValue(organic_carbon_index, -total_ec, box_index);
            // ADD the mineralised nitrogen to that buffer
            scoreboard->AddCharacteristicValue(mineralisation_buffer_index, total_mn, box_index);
            // ADD the evolved carbon to that buffer
            scoreboard->AddCharacteristicValue(CO2_buffer_index, total_ec, box_index);
        }

        //    Send the stuff we have expelled
        SendUnsolicitedCoughUp(mineralisation_cough_up, action);

        SendUnsolicitedCoughUp(evolved_carbon_cough_up, action);

        // refresh decay time
        time_of_previous_decay = time;

        return (return_value);
    }

    long int OrganicMatter::DoSpecialInput(ProcessActionDescriptor* action)
    {
        return (OrganicMatter::DoOrganicInputMessage(action));
    }

    long int OrganicMatter::DoOrganicInputMessage(ProcessActionDescriptor* action)
    {
        /*    —    —    —    —    —    —    —    —    —    —    —    — *
        *            VARIABLE DECLARATIONS
        *    —    —    —    —    —    —    —    —    —    —    —    — */
        // declaration of commonly used things
        Use_ReturnValue;
        Use_Scoreboard;
        Use_Data;

        // a variable that will be the handle to the information
        // that the organic input process has sent us
        OrganicMatterInput* new_input = (OrganicMatterInput*)data;

        AddInput(scoreboard, new_input);

        return (return_value);
    }

    void OrganicMatter::DisposeSpecialInformation(Scoreboard* scoreboard)
    {
        long int inputs_visited = 0;
        OrganicMatterInput* input = __nullptr;
        OrganicMatterInput* next_input = __nullptr;
        const long special_info_index = GetSpecialCharacteristicIndex();

        BoxIndex box_index = 0;

        while ((inputs_visited < number_of_inputs) && (box_index < BOX_COUNT))
        {
            input = (OrganicMatterInput*)scoreboard->GetSpecialPointer(special_info_index, box_index);

            while (input != __nullptr)
            {
                next_input = input->GetNextInput();
                TCLForgetObject(input);
                input = next_input;
                ++inputs_visited;
            }
        }

        ProcessModuleBase::DisposeSpecialInformation(scoreboard);
    }

    /* AddInput
    Caller        : DoOrganicInputMessage().
    Description    : 1. To add the "input" to the appropriate box's linked list of inputs
                  2. To add/transfer the Carbon and Nitrogen information to the scoreboard.
    */
    void OrganicMatter::AddInput(Scoreboard* scoreboard, OrganicMatterInput* input)
    {
        // a variable that will hold the 3D coordinate of the box we are interested in
        BoxCoordinate box_coord;

        // a variable to a handle of other, previous organic matter
        // inputs that will be used for working with
        OrganicMatterInput* previous_input = __nullptr;

        /*    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊
        ◊    BUSINESS Part 1
        ◊    Stick this input in the linked list
        ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    */
        // get the coordinate of the box in which the input resides
        input->GetPosition(&box_coord);

        // if the input file accounted for more boxes than actually
        // exist, we need to ignore this particular "input"
        if ((box_coord.x > NUM_X) || (box_coord.y > NUM_Y) || (box_coord.z > NUM_Z))
            return;

        const long special_info_index = GetSpecialCharacteristicIndex();

        // find any previous inputs stored in this box
        previous_input = (OrganicMatterInput*)scoreboard->GetSpecialPointer(special_info_index, box_coord.x, box_coord.y, box_coord.z);

        // if there was NOT anything previously stored, then…    
        if (previous_input == 0)
        {
            // save the current information into the scoreboard
            scoreboard->SetSpecialPointer(special_info_index, box_coord.x, box_coord.y, box_coord.z, input);
        }
        else
        {
            // otherwise, save the current information with the rest (which currently means at the end of a linked list, 
            // but could just as easily be an indexed list)
            previous_input->SetNextInput(input);
        }

        /*    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊
        ◊    BUSINESS Part 2
        ◊    Store the newly arrived input in scoreboard (public) storage
        ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    ◊    */
        // declare some relevant variables
        double amount_of_nitrogen, amount_of_carbon;

        // find out how much arrived. That is, get the amounts from the input
        amount_of_nitrogen = input->GetNitrogen();
        amount_of_carbon = input->GetCarbon();

        // Add the amounts to the relevant (already known) box
        Add_ConcentrationX_Of_Box(ORGANIC_NITROGEN_NUMBER, amount_of_nitrogen, box_coord.x, box_coord.y, box_coord.z);
        Add_ConcentrationX_Of_Box(ORGANIC_CARBON_NUMBER, amount_of_carbon, box_coord.x, box_coord.y, box_coord.z);

        /*    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !
        !    Alternative Therapies                                                        !
        !    The shortcut, without fiddling with variables                                    !
        !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    */
        //    Add_ConcentrationX_Of_Box(ORGANIC_NITROGEN_NUMBER, input->GetNitrogen(), box_coord);
        //    Add_ConcentrationX_Of_Box(ORGANIC_CARBON_NUMBER  , input->GetCarbon()  , box_coord);

        /*    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !
        !    Or if we did not already know which box in which the input was, just the soil    !
        !    location, we would use a different procedure :                                !
        !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    */
        //    Add_Value_Point_In_Soil(ORGANIC_NITROGEN_NUMBER, amount_of_nitrogen, input_coord.x, input_coord.y, input_coord.z);
        //    Add_Value_Point_In_Soil(ORGANIC_CARBON_NUMBER, amount_of_carbon, input_coord.x, input_coord.y, input_coord.z);

        /*    •    •    •    •    •    •    •    •    •    •    •    •    •    •    •    •
        •                THE END OF THE BUSINESS AT HAND                    •
        •    •    •    •    •    •    •    •    •    •    •    •    •    •    •    •    */

        /*
                        MISCELLANEOUS BUSINESS
        */
        // To make tracking all our inputs a little easier, we remember how many of them there are.
        ++number_of_inputs;
    }
} /* namespace rootmap */

