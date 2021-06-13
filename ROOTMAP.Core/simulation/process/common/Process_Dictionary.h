#ifndef Process_Dictionary_H
#define Process_Dictionary_H

#include "core/macos_compatibility/macos_compatibility.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/interprocess_communication/ProcessActionDescriptor.h"
#include "simulation/process/interprocess_communication/PostOffice.h"

#include "wx/datetime.h"

namespace rootmap
{
    /* Message Types

    Comments included here are summary notes.
    For more accurate/verbose/informative programming
    information, refer to the appropriate documentation */
    enum
    {
        /*    ***********************************
            "Core Suite"

            These messages are understood by default
            by all modules that are ancestors of the
            Process class.
        *********************************** */

        /* Null
        Not currently used for anything */
        kNullMessage = 'Null',

        /* Register
        Sent by the PostOffice at the creation and opening of a simulation, before
        Initialisation.  The purpose of the registration is to send
        PlantSummaryRegistrations before trying to search for the subsequently created
        Summaries in Initialise. */
        kRegisterMessage = 'Rego',

        /* Initialise
        Sent by the PostOffice at the creation and openning of a simulation*/
        kInitialiseMessage = 'Init',

        /* Terminate
        Sent by the PostOffice at the beginning of the Simulation destruction */
        kTerminateMessage = 'Term',

        /* Start
        Sent by the PostOffice to all active processes at the commencement of a run */
        kStartMessage = 'Strt',

        /* PauseToggle
        MSA TODO write something about how it works here    */
        kPauseMessage = 'Paus',

        /* End
        Sent by the PostOffice to all active modules when running is attempting to cease.
        Any process may send further messages, but they will not be sent (except for
        those to occur earlier in the same timestamp) */
        kEndMessage = 'End ',

        /* PreNormal
         * Just before normal, when a process wants to wake another just before it
         * The receiving process still just gets a DoNormalWakeUp
         */
        kPreNormalMessage = 'Pnml',

        /* Normal
        Sent when the process has nominated to wake itself up at a certain time to do regular/periodic work */
        kNormalMessage = 'Nrml',


        /*    ***********************************
            "Standard Interprocess Communications Suite"

            This set of message types is for communication between modules
            that wish to give and/or get some of their own or othersÕ ÒstuffÓ.
            The  functionality required for a module to act upon these
            messages is built in to the ÒProcessÓ class, and so is standard,
            however in order to Òcough-upÓ and ÒreceiveÓ stuff, they must
            have extra characteristics (invisible buffers) declared, so as to
            cope with the receival of stuff.

            Note that anything to do with Registered
            requests has 'Rg' in it, and anything
            to do with coughing up has 'Cu' in it
        *********************************** */

        /* External WakeUp Alarm
        Sent by another process module, to wake "this" one up to do
        some processing based on the other process' action[s] */
        kExternalWakeUpMessage = 'XWak', // eXternal WAKe up

        /* Register Request
        Sent by client to server processes to
        notify of request for a characteristic */
        kRegisterRequestMessage = 'RgRq', // ReGister ReQuest

        /* Registered Cough-Up
        Sent by the server to all clients when periodical
        coughing up of registered requests is happenning */
        kRegisteredCoughUpMessage = 'RgCu', // ReGistered Cough Up

        /* Normal Cough-Up
        Sent as a message, setup in DoRegisterRequest to invoke regular/periodic
        coughing up of registered requests */
        kNormalCoughUpMessage = 'NmCu', // NorMal Cough Up

        /* Cough-Up Immediate
        Sent by a client to a server process when it
        wants IMMEDIATELY all of what it is due. That
        is, all that it has accrued since it last received */
        kCoughUpImmediateMessage = 'CuIm',

        /* Self Coughed-Up Immediate
        Sent by the server to the client that
        requested an immediate cough up */
        kSelfCoughedUpImmediateMessage = 'SCui',

        /* Other Coughed-Up Immediate
        Sent by the server to all other clients when
        an immediate cough up is performed */
        kOtherCoughedUpImmediateMessage = 'OCui',

        /* Unsolicited Cough-Up
        Sent by an unknown server to an unregistered
        (although listed) client when an unsolicited cough up occurs */
        kUnsolicitedCoughUpMessage = 'UnCu',

        /* Receiver Delayed Reaction
        Sent by a client to itself when it receives a
        characteristic, to enable it to shift the received
        stuff into permanent storage */
        kReceiverDelayedReactionMessage = 'ReDR',

        /*    ***********************************
            "Standard PlantCoordinator Communications Suite"

            Message Types used to communicate with
            PlantCoordinator, usually to request summary info
            about plants. Usage of these message types
            involves the class PlantSummaryRegistration,
            and the Process functions :
            MakePlantSummaryRegistration()
            SendPlantSummaryRegistration()
        *********************************** */
        /* Register Global Plant Summary
        Sent by any other module to the PlantCoordinator
        to request a/some per-plant characteristics */
        kRegisterGlobalPlantSummaryMessage = 'RgPs',

        /* Global Plant Summary Registered
        Sent by the PlantCoordinator as its answer to
        ÒRegister Global Plant SummaryÓ requests */
        kGlobalPlantSummaryRegisteredMessage = 'GpSr',

        /* Register Scoreboard Plant Summary
        Sent by any other module to the PlantCoordinator
        to request a/some per-box scoreboard characteristic[s] */
        kRegisterScoreboardPlantSummaryMessage = 'RsPs',

        /* Scoreboard Plant Summary Registered
        Sent by the PlantCoordinator as its answer to
        ÒRegister Scoreboard Plant SummaryÓ requests */
        kScoreboardPlantSummaryRegisteredMessage = 'SpSr',

        /* Special Input Data
        Sent to any Process*/
        kSpecialInputDataMessage = 'Inpu',

        /*
        */
        kSpecialOutputDataMessage = 'Outp',

        /*    ***********************************
            "Special Interprocess Communications Suite"
        *********************************** */
        /* The sorts of message types you will find here are
        those that are absolutely necessary to serve some
        unprecedented interprocess communication that
        can't be handled in one way or another by any of
        the already defined message types.

        An example of this, although it would be outside
        of the current scope of interprocess understanding,
        would be if two (or more) concurrently designed
        modules needed to communicate process-
        specific information */
        //
        // 20030328 RvH I've defined 10 "General Special" interprocess message types.
        //          These are intended to be a reasonable replacement for the currently
        //          overloaded kExternalWakeUpMessage ('XWak') message type. They are
        //          most appropriately used between consenting process modules.
        //
        //          In fact, feel free to #define these to other 
        //
        kGeneralPurposeMessage0 = 'GPm0',
        kGeneralPurposeMessage1 = 'GPm1',
        kGeneralPurposeMessage2 = 'GPm2',
        kGeneralPurposeMessage3 = 'GPm3',
        kGeneralPurposeMessage4 = 'GPm4',
        kGeneralPurposeMessage5 = 'GPm5',
        kGeneralPurposeMessage6 = 'GPm6',
        kGeneralPurposeMessage7 = 'GPm7',
        kGeneralPurposeMessage8 = 'GPm8',
        kGeneralPurposeMessage9 = 'GPm9'
    };

    const int kNumberOfMessageTypes = 30;

    // Message action or response errors
    enum
    {
        kNoError = noErr,
        kAlreadyCoughedUpThisTimestamp,
        kNoRegisteredRequests,
        kDataCorrupted,
        kInvalidMessage
    };

    //#pragma mark Basic Equates and Macros
    // roughly the number of seconds in a month
    // = (365.24998x24x60x60)/12
    const static double SECONDS_IN_A_MONTH = 2629799.856;

#define PROCESS_YEARS(x) (x*365.24998*24*60*60)
#define PROCESS_MONTHS(x) (x*SECONDS_IN_A_MONTH)
#define PROCESS_DAYS(x) (x*24*60*60)
#define PROCESS_HOURS(x) (x*60*60)
#define PROCESS_MINUTES(x) (x*60)

    const int kPrimary = 0;
    const int kFirstOrder = 1;
    const int kSecondOrder = 2;
    const int kThirdOrder = 3;
    const int kFourthOrder = 4;

    const int kOnly = 0;
    const int kFirst = 1;
    const int kSecond = 2;
    const int kThird = 3;
    const int kFourth = 4;

#define BEGIN_DRAWING try { ProcessDrawing & drawing = getDrawing(); 
#define END_DRAWING } catch (ProcessDrawingException & /*pde*/) { throw; }

    // _______________________________________________________________________________________________________
    //#pragma mark Declaring

#define Use_Scoreboard          Scoreboard * scoreboard = action->GetScoreboard()
#define Use_ScoreboardCoordinator \
                                ScoreboardCoordinator * scoreboardcoordinator = action->GetScoreboardCoordinator()
#define Use_ProcessCoordinator  ProcessCoordinator * processcoordinator = action->GetProcessCoordinator()
#define Use_PostOffice          PostOffice * postoffice = action->GetPostOffice()
#define Use_SharedAttributeManager \
                                SharedAttributeManager * sharedattributemanager = action->GetSharedAttributeManager()
#define Use_VolumeObjectCoordinator \
                                VolumeObjectCoordinator * volumeobjectcoordinator = action->GetVolumeObjectCoordinator();\
                                const size_t VO_COUNT = volumeobjectcoordinator->GetVolumeObjectList().size()
#define Use_Source              Process* source = action->GetSource()
#define Use_Time                const wxDateTime startTime = wxDateTime::UNow();\
                                ProcessTime_t time = action->GetTime()
#define Use_Code                MessageType_t code = action->GetCode()
#define Use_Data                SpecialProcessData * data = action->GetData()
#define Use_ReturnValue         long int return_value = 0

#define Use_Objects             Use_Scoreboard; \
                                Use_ScoreboardCoordinator; \
                                Use_ProcessCoordinator; \
                                Use_PostOffice; \
                                Use_SharedAttributeManager; \
                                Use_VolumeObjectCoordinator

#define Use_Values              Use_Time; \
                                Use_Code; \
                                Use_Data;

#define Use_Boundaries          const BoundaryArray & X_boundaries = scoreboardcoordinator->GetBoundaryArray(X, GetProcessStratum()); \
                                const BoundaryArray & Y_boundaries = scoreboardcoordinator->GetBoundaryArray(Y, GetProcessStratum()); \
                                const BoundaryArray & Z_boundaries = scoreboardcoordinator->GetBoundaryArray(Z, GetProcessStratum())

#define Use_PlantSummaryRegistration \
                                PlantSummaryRegistration * psr = dynamic_cast<PlantSummaryRegistration*>(action->GetData()); \
                                RmAssert(psr!=0, "ProcessActionDescriptor data was not a PlantSummaryRegistration")

    // _______________________________________________________________________________________________________
    //#pragma mark Scoreboard Verbs
    // Scoreboard verbs
    // Finding out the scoreboard's size
#define Get_Number_Of_Layers(x,y,z)                                                            (scoreboard->GetNumLayers(x,y,z))
#define Get_Box_Volume(x, y, z)                                                             (scoreboard->GetBoxVolume(x, y, z))
#define Get_BoxIndex_Volume(box_index)                                                      (scoreboard->GetBoxVolume(box_index))

    // Scoreboard Characteristics
    //
#define Set_Value_Point_In_Soil(characteristic_number,amount,x,y,z)                         scoreboard->SetCharacteristicPoint(GetCharacteristicIndex(characteristic_number), amount, x, y, z)
#define Add_Value_Point_In_Soil(characteristic_number,amount,x,y,z)                         scoreboard->AddCharacteristicPoint(GetCharacteristicIndex(characteristic_number), amount, x, y, z)
#define Subtract_Value_Point_In_Soil(characteristic_number,amount,x,y,z)                    scoreboard->AddCharacteristicPoint(GetCharacteristicIndex(characteristic_number), -amount, x, y, z)

#define Set_Value_Point_In_Box(characteristic_number,amount,box_coordinate1)                scoreboard->SetCharacteristicPoint(GetCharacteristicIndex(characteristic_number), amount, &box_coordinate1)
#define Add_Value_Point_In_Box(characteristic_number,amount,box_coordinate1)                scoreboard->AddCharacteristicPoint(GetCharacteristicIndex(characteristic_number), amount, &box_coordinate1)
#define Subtract_Value_Point_In_Box(characteristic_number,amount,box_coordinate1)           scoreboard->AddCharacteristicPoint(GetCharacteristicIndex(characteristic_number), -amount, &box_coordinate1)


    // When the characteristicÕs scoreboard index is used.
#define Get_Concentration_Of_Box(characteristic_index,x,y,z)                                (scoreboard->GetCharacteristicValue(characteristic_index, x, y, z))
#define Set_Concentration_Of_Box(characteristic_index,amount,x,y,z)                            scoreboard->SetCharacteristicValue(characteristic_index, amount, x, y, z)
#define Add_Concentration_Of_Box(characteristic_index,amount,x,y,z)                            scoreboard->AddCharacteristicValue(characteristic_index, amount, x, y, z)

    // When the characteristicÕs scoreboard index and the scoreboard's box index is used.
#define Get_Concentration_Of_BoxIndex(characteristic_index,box_index)                        (scoreboard->GetCharacteristicValue(characteristic_index, box_index))
#define Set_Concentration_Of_BoxIndex(characteristic_index,amount,box_index)                scoreboard->SetCharacteristicValue(characteristic_index, amount, box_index)
#define Add_Concentration_Of_BoxIndex(characteristic_index,amount,box_index)                scoreboard->AddCharacteristicValue(characteristic_index, amount, box_index)

    // When the processÕs characteristic number is used
#define Get_ConcentrationX_Of_Box(characteristic_number,x,y,z) (scoreboard->GetCharacteristicValue(GetCharacteristicIndex(characteristic_number), x, y, z))
#define Set_ConcentrationX_Of_Box(characteristic_number,amount,x,y,z) scoreboard->SetCharacteristicValue(GetCharacteristicIndex(characteristic_number), amount, x, y, z)
#define Add_ConcentrationX_Of_Box(characteristic_number,amount,x,y,z) scoreboard->AddCharacteristicValue(GetCharacteristicIndex(characteristic_number), amount, x, y, z)


    // Using the Custom per-box Information.
#define Get_Custom_Info_Pointer(x,y,z) (scoreboard->GetCustomPointer(pCustomPerBoxInformationIndex, x, y, z))
#define Set_Custom_Info_Pointer(x,y,z,info) scoreboard->SetCustomPointer(pCustomPerBoxInformationIndex, x, y, z, (void*)info)


    //#pragma mark PostOffice
    // _______________________________________________________________________________________________________
    // PostOffice verbs
#define Send_WakeUp_Message(time) postoffice->sendMessage(time, this)
#define Send_External_WakeUp(time, dest, data) postoffice->sendMessage(time, this, dest, data, kExternalWakeUpMessage)
#define Send_Immediate_Message(dest,descriptor) postoffice->sendMessage(this,dest,descriptor)
#define Send_Interprocess_Message(time, dest, data, mess_type) postoffice->sendMessage(time, this, dest, data, mess_type)
#define Send_GeneralPurpose(time, dest, data, type) postoffice->sendMessage(time, this, dest, data, type)

    //#pragma mark ScoreboardCoordinator
    // _______________________________________________________________________________________________________
    // ScoreboardCoordinator verbs
#define Get_Scoreboard(s)                       scoreboardcoordinator->GetScoreboard(s)
#define Get_Top_Scoreboard                      scoreboardcoordinator->GetTopScoreboard()
#define Get_Bottom_Scoreboard                   scoreboardcoordinator->GetBottomScoreboard()

    //
    // PRE-REQUISITE: Use_Boundaries
    //
#define Get_Layer_Thickness(dimension,layer)    dimension##_boundaries.GetLayerThickness(layer)
#define Get_Layer_Width(layer)                  Get_Layer_Thickness(X,layer)
#define Get_Layer_Depth(layer)                  Get_Layer_Thickness(Y,layer)
#define Get_Layer_Height(layer)                 Get_Layer_Thickness(Z,layer)

#define Get_Layer_Start(dimension,layer)        dimension##_boundaries.GetLayerTop(layer)
#define Get_Layer_Left(layer)                   Get_Layer_Start(X,layer)
#define Get_Layer_Front(layer)                  Get_Layer_Start(Y,layer)
#define Get_Layer_Top(layer)                    Get_Layer_Start(Z,layer)

#define Get_Layer_End(dimension,layer)          dimension##_boundaries.GetLayerBottom(layer)
#define Get_Layer_Right(layer)                  Get_Layer_End(X,layer)
#define Get_Layer_Back(layer)                   Get_Layer_End(Y,layer)
#define Get_Layer_Bottom(layer)                 Get_Layer_End(Z,layer)


#define Get_Scoreboard_Bounds(double_box) scoreboardcoordinator->GetScoreboardBounds((scoreboard->GetScoreboardStratum()).value(), &float_box)

#define Get_Box_Soil_By_Box(box, x, y, z) scoreboardcoordinator->GetBoxSoil(&box_soil, (scoreboard->GetScoreboardStratum()).value(), x, y, z)
#define Get_Box_Soil_By_Coordinate(box, coordinate) scoreboardcoordinator->GetBoxSoil(&box_soil, (scoreboard->GetScoreboardStratum()).value(), &coordinate)

#define Find_Box_Containing_SoilCoordinate(box_coordinate1, soil_coordinate) (scoreboardcoordinator->FindBoxBySoilCoordinate(&box_coordinate1, pVolumeType, &soil_coordinate))
#define Find_Box_Containing_SoilCoordinate_Anywhere(box_coordinate1, soil_coordinate) (scoreboardcoordinator->FindBoxBySoilCoordinate(&box_coordinate1, &soil_coordinate))

    //#pragma mark ProcessCoordinator
    // ProcessCoordinator verbs
#define Find_Process_By_Process_Name(process_name) (processcoordinator->FindProcessByProcessName(process_name))
#define Find_Process_By_Characteristic_Name(characteristic_name,characteristic_number) (processcoordinator->FindProcessByCharacteristicName(characteristic_name, &characteristic_number))
#define Find_Characteristic_Index_By_Characteristic_Name_Same_Volume(characteristic_name) (processcoordinator->FindCharacteristicIndexByCharacteristicName(characteristic_name))
#define Find_Characteristic_Index_By_Characteristic_Name_Other_Volume(characteristic_name) (processcoordinator->FindCharacteristicIndexByCharacteristicName(characteristic_name))
#define Find_Characteristic_Index_By_Characteristic_Name_All_Volumes(characteristic_name) (processcoordinator->FindCharacteristicIndexByCharacteristicName(characteristic_name))
#define Find_Process_By_ID(id) (processcoordinator->FindProcessByCharacteristicID(id))

    //#pragma mark SharedAttributeManager
    // _______________________________________________________________________________________________________
    // SharedAttributeManager verbs
#define Get_Cluster(vn) (sharedattributemanager->GetCluster(vn))
} /* namespace rootmap */

#endif // #ifndef Process_Dictionary_H
