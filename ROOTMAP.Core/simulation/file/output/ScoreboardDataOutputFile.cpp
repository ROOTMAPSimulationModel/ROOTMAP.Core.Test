#include "BoostPrecomp.h"

#include "JDataOutputFile.h"
#include "ScoreboardDataOutputFile.h"
#include "JOutputAssociation.h"
#include "Simulation.h"
#include "Process.h"
#include "Utilities.h"
#include "ErrorCollection.h"
#include "simulation/common/BoostPrecomp.h"

namespace rootmap
{

Boolean AddressIndices( Str31 spec_str, long* xi, long* yi, long* zi, long* x_max, long* y_max, long* z_max, long* *inner, long* *middle, long* *outer, long* *inner_max, long* *middle_max, long* *outer_max);

ScoreboardDataOutputFile::ScoreboardDataOutputFile()
    : m_succeeded(true)
    , m_scoreboard(NULL)
    , m_characteristicnumber(-1)
    , m_process(NULL)
    // These last 3 seem to be redundant leftovers
    , output_process(0)
    , output_characteristicnumber(-1)
    , output_succeeded(true)
{
    // MSA 09.11.26 Shifted to initialisation list

    TCL_END_CONSTRUCTOR
}

/*
DoProvokeOutput

Called by : OutputFileAlarm::InvokeOutputRules()

Description :
Override to send a kSpecialOutputMessage via the PostOffice to the appropriate process.
Send an OutputFileInformationH as the data (defined in "JOutputAssociation.h"). The
process should then (at the appropriate juncture) call this object's DoOutput() with the
data this class' subclass knows how to export.

For any other object source, this class' special subclass should have some agreement
as to exactly what happens.

Arguments :
ProcessActionDescriptor. Mostly well understood or documented. The padData field
contains the JOutputAssociation that fostered this file instance.

Working Notes :
We don't really need to call upon the Process because we need any extra information.
Its all there, in the scoreboard. The reason we do so is because the kSpecialOutputMessage
is almost the last message processed in any one timestamp (just before the kEndMessage
but after the kDelayedReceivalMessage). Thus, it is the best way of ensuring that the data
exported is an accurate representation of the scoreboard at that instant/timstamp.
*/
void ScoreboardDataOutputFile::DoProvokeOutput(ProcessActionDescriptor* action)
{
    Use_PostOffice;
    Use_Time;

    if (output_process == 0)
    {
        Use_ProcessCoordinator;
        JOutputAssociation* r = (JOutputAssociation*)(action->padData);
        // can't use
        // Find_Process_By_Characteristic_Name(r->output_assoc_origin_name);
        // because it uses this->pVolumeType as the second argument
        output_process = processcoordinator->FindProcessByCharacteristicName(r->OriginVariation(), &output_characteristicnumber);
#ifdef _CS_DEBUG_
        Str255 p_name;
        output_process->GetProcessName(p_name);
        if (!EqualString(r->OriginName(), p_name, true, true))
            Debugger();
#endif
    }

    if (output_process)
        postoffice->sendMessage(time, output_process, output_process, action->padData, kSpecialOutputDataMessage);

    JDataOutputFile::DoProvokeOutput(action);
}

/*
Description :
This is the guts of this class. What we wrote (well, i did anyway) a whole lot of code for.
This queries the scoreboard as to its contents.
*/
void ScoreboardDataOutputFile::DoOutput(ProcessActionDescriptor* action)
{
    Use_Scoreboard;

    // index counters for scoreboard access
    long x_index, y_index, z_index;
    long *inner, *middle, *outer;

    // the value itself
    double value;

    // of the output characteristic, funnily enough
    long characteristic_index;
    if (output_process != 0)
        characteristic_index = output_process->GetCharacteristicIndex(output_characteristicnumber);
    else
#ifdef _CS_DEBUG_
        Debugger();
#else
        return;
#endif
    
    // maximum scoreboard indices
    long x_max, y_max, z_max;
    scoreboard->GetNumLayers(x_max, y_max, z_max);
    long *inner_max, *middle_max, *outer_max;

    // Suss' out which order to export the dimensions in
    Str31 dim_order_spec = "\p";
    JOutputAssociation *r = (JOutputAssociation *)(action->padData);
    TCLpstrcpy(dim_order_spec, r->Specification1());

    if (!output_succeeded) return;

    if (!(output_succeeded =    (CheckSuccess
                                    (AddressIndices
                                        (dim_order_spec,
                                        &x_index, &y_index, &z_index,
                                        &x_max, &y_max, &z_max,
                                        &inner, &middle, &outer,
                                        &inner_max, &middle_max, &outer_max
                                        )
                                    , 0, 0, 0
                                    )
                                )
        ) )
        return;    

    Open(fsWrPerm);

    *outer = 1;
    while (*outer <= *outer_max)
    {
        *middle = 1;
        while (*middle <= *middle_max)
        {
            *inner = 1;
            while (*inner <= *inner_max)
            {
                value = scoreboard->GetCharacteristicValue(characteristic_index, x_index, y_index, z_index);
                
                WriteDoubleTab(value);

                (*inner)++;
            } // while (*inner <= *inner_max)

            // Goto the next line
            WriteLn();

            (*middle)++;
        } // while (*middle <= *middle_max)

        // Goto the next line
        WriteLn();

        (*outer)++;
    } // while (*outer <= *outer_max)

    FlushBuffer();
    DoClose(false);// "false" instructs the TCL not to call FailOSErr()
}

Boolean AddressIndices
(
    Str31 spec_str,
    long* xi, long* yi, long* zi,
    long* x_max, long* y_max, long* z_max,
    long* *inner, long* *middle, long* *outer,
    long* *inner_max, long* *middle_max, long* *outer_max
)
{
    // if we don't have enough
    if (spec_str[0] < 3) return false;

    Boolean success = true;

    // the first character is the inner loop
    switch (spec_str[1])
    {
        case 'X' :
        case 'x' :
            *inner = xi;
            *inner_max = x_max;
            break;

        case 'Y' :
        case 'y' :
            *inner = yi;
            *inner_max = y_max;
            break;

        case 'Z' :
        case 'z' :
            *inner = zi;
            *inner_max = z_max;
            break;

        default:
            success = false;
    }

    // second character : middle loop
    switch (spec_str[2])
    {
        case 'X' : case 'x' : *middle = xi; *middle_max = x_max; break;
        case 'Y' : case 'y' : *middle = yi; *middle_max = y_max; break;
        case 'Z' : case 'z' : *middle = zi; *middle_max = z_max; break;
        default: success = false;
    }

    // third character : outer loop
    switch (spec_str[3])
    {
        case 'X' : case 'x' : *outer = xi; *outer_max = x_max; break;
        case 'Y' : case 'y' : *outer = yi; *outer_max = y_max; break;
        case 'Z' : case 'z' : *outer = zi; *outer_max = z_max; break;
        default: success = false;
    }

    return (success);
}

} // namespace rootmap

