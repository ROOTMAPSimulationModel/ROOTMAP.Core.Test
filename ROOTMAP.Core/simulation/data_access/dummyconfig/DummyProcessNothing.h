#ifndef DummyProcessNothing_H
#define DummyProcessNothing_H

#include "simulation/process/common/Process.h"

namespace rootmap
{
    class DummyProcessNothing : public Process
    {
        DECLARE_DYNAMIC_CLASS(DummyProcessNothing)

    public:
        DummyProcessNothing();
        ~DummyProcessNothing();

        /**
         *
         */
        virtual bool DoesOverride() const;

        /**
         *
         */
        virtual long int Initialise(ProcessActionDescriptor* start);
        /**
         *
         */
        virtual long int Terminate(ProcessActionDescriptor* action);

        /**
         *
         */
        virtual long int StartUp(ProcessActionDescriptor* start);
        /**
         *
         */
        virtual long int Ending(ProcessActionDescriptor* end);

        /**
         *
         */
        virtual long int WakeUp(ProcessActionDescriptor* action);

        /**
         *
         */
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action);

        /**
         *
         */
        virtual long int DoDelayedReceivalReaction(ProcessActionDescriptor* action);

        /**
         *
         */
        virtual long int DoSpecialInput(ProcessActionDescriptor* action); // 'Inpu'

        /**
         *
         */
        virtual void DisposeSpecialInformation(Scoreboard* scoreboard);

        /**
         *
         */
        virtual long int GetPeriodicDelay(long int characteristic_number);


        // ///////////////
        // Drawing
        virtual bool DoesDrawing() const;

        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);

    private:
        long int TransferReceivalsToInputs(ProcessActionDescriptor* action);


        typedef DoubleCoordinate DummyItemType;
        typedef std::vector<DummyItemType> DummyItemCollection;

        DummyItemCollection m_items;

        void DrawItem(const DummyItemType& item);
    };
} /* namespace rootmap */

#endif // #ifndef DummyProcessNothing_H
