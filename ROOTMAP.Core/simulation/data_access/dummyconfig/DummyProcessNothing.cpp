//Heritage
#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessDrawing.h"
#include "simulation/data_access/dummyconfig/DummyProcessNothing.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/utility/Utility.h"


namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(DummyProcessNothing, Process)

        DummyProcessNothing::DummyProcessNothing()
        : Process()
    {
    }

    DummyProcessNothing::~DummyProcessNothing()
    {
    }

    bool DummyProcessNothing::DoesOverride() const { return (true); }
    bool DummyProcessNothing::DoesDrawing() const { return (true); }

    long int DummyProcessNothing::Initialise(ProcessActionDescriptor* action)
    {
        Use_Scoreboard;

        double x_min = scoreboard->GetTop(X);
        double x_max = scoreboard->GetBottom(X);
        double y_min = scoreboard->GetTop(Y);
        double y_max = scoreboard->GetBottom(Y);
        double z_min = scoreboard->GetTop(Z);
        double z_max = scoreboard->GetBottom(Z);

        for (int item_index = 0; item_index < 500; ++item_index)
        {
            double x_ = Utility::RandomRange(x_max, x_min);
            double y_ = Utility::RandomRange(y_max, y_min);
            double z_ = Utility::RandomRange(z_max, z_min);

            DummyItemType item(x_, y_, z_);
            m_items.push_back(item);
        }

        // invoke regular awakenings every 1 day
        SetPeriodicDelay(kNormalMessage, PostOffice::CalculateTimeSpan(0, 0, 1, 0, 0, 0));

        return (Process::Initialise(action));
    }

    long int DummyProcessNothing::Terminate(ProcessActionDescriptor* action)
    {
        return (Process::Terminate(action));
    }

    /****
    **    StartUp
    **
    **    This function is called at the beginning of each ÒGoÓ run to establish that
    **    activity is to commence.
    **
    **    In the case of Organic Matter, we check to see if we have any organic inputs and
    **    if so, organise to wake up in the near future to act upon them
    ****/
    long int DummyProcessNothing::StartUp(ProcessActionDescriptor* action)
    {
        return (Process::StartUp(action));
    }

    /****
    **    Ending
    **
    **    This function is called at the end of each ÒGoÓ run, to signify that activity is halting.
    **    Messages can still be sent, and scoreboard processing may still occur, but messages
    **    will not be acted upon until the next run (at the earliest).
    ****/
    long int DummyProcessNothing::Ending(ProcessActionDescriptor* action)
    {
        return (Process::Ending(action));
    }

    long int DummyProcessNothing::WakeUp(ProcessActionDescriptor* action)
    {
        return (Process::WakeUp(action));
    }

    /****
    **    DoNormalWakeUp
    **
    **    This function is called each time the PostOffice finds a message addressed to this
    **    process. This is the main means of communication between processes and/or characteristics.
    **
    **    Of course, each process will have its own properties and idiosyncrasies which will
    **    determine its actions when it receives a message. Let us inspect this DummyProcessNothing
    **    process as an example.
    **
    **    It may have received a WakeUp() call for one of a number of reasons. Those known
    **    or envisaged at the moment are:
    **        ¥ self-requested (kNormalMessage), in which case the action to take should be already known.
    **        ¥ to receive input from another process (kOrganicInputMessage).
    **        ¥ to arbitrate the giving out of an amount of any of the characteristics it owns
    **
    **    Thus, a good start would be to see what type of message has been received then
    **    call more specific functions to handle those messages.
    ****/
    long int DummyProcessNothing::DoNormalWakeUp(ProcessActionDescriptor* action)
    {
        Use_Scoreboard;

        double z_max = scoreboard->GetBottom(Z);

        // Move 5% of the items, each 10% downwards, each time stamp
        size_t num_items = m_items.size();
        size_t num_to_change = num_items * 0.05;

        BEGIN_DRAWING
            for (size_t index = 0; index < num_to_change; ++index)
            {
                // pick a random item to change
                long item_to_change = Utility::RandomRange((long)(num_items - 1));
                DummyItemType item(m_items[item_to_change]);

                // un-draw the dot. For this process that means drawing in white
                // MSA 09.10.16 Dot un-drawing is now handled (in non-dummy processes) by RemoveDot().
                // This function erases a dot from the 3D view buffer, and just draws a white dot for the 2D view.
                wxColour c = *wxWHITE;
                drawing.DrawDot(item, c);

                DummyItemType item_copy(item);

                // move it downwards 10%
                item.z += (z_max - item.z) / 10;
                item.z = Utility::CSMin(item.z, z_max);

                //wxLogMessage(wxT("Time=%d Changed item[%d] from %s to %s"), action->GetTime(), item_to_change, ToString(item_copy).c_str(), ToString(item).c_str());

                // change the item!
                m_items[item_to_change] = item;

                // draw the dot. For this process that means drawing in the prevailing
                // colour. We could choose to explicitly colour it black.
                drawing.DrawDot(item);
            }
        END_DRAWING


            return (Process::DoNormalWakeUp(action));
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
    long int DummyProcessNothing::DoDelayedReceivalReaction(ProcessActionDescriptor* /* action */)
    {
        return (kNoError);
    }

    long int DummyProcessNothing::TransferReceivalsToInputs(ProcessActionDescriptor* /* action */)
    {
        return (kNoError);
    }

    long int DummyProcessNothing::DoSpecialInput(ProcessActionDescriptor* /* action */)
    {
        return (kNoError);
    }

    void DummyProcessNothing::DisposeSpecialInformation(Scoreboard* /* scoreboard */)
    {
    }

    long int DummyProcessNothing::GetPeriodicDelay(long int /* characteristic_number */)
    {
        return (PostOffice::CalculateTimeSpan(0, 0, 1, 0, 0, 0));
    }

    void DummyProcessNothing::DrawScoreboard(const DoubleRect& /* area */, Scoreboard* scoreboard)
    {
        // no need to draw if we're not interested in this
        if (scoreboard->GetScoreboardStratum() != this->GetProcessStratum())
        {
            return;
        }

        BEGIN_DRAWING
            for (DummyItemCollection::iterator iter = m_items.begin();
                iter != m_items.end(); ++iter)
        {
            drawing.DrawDot((*iter));
        }
        END_DRAWING
    }

    void DummyProcessNothing::DrawItem(const DummyItemType& /* item */)
    {
    }
} /* namespace rootmap */

