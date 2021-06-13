#ifndef OrganicMatter_H
#define OrganicMatter_H

// other files whose definitions we use in THIS FILE
#include "simulation/process/common/Process.h"
//#include "DoubleCoordinates.h"

#if defined FORCE_DYNAMIC_CLASS_USAGE_MACROS
#define DECLARE_FORCE_DYNAMIC_CLASS_USAGE(cname) \
    void RmForceDynamicClassUsage_##cname();
#define IMPLEMENT_FORCE_DYNAMIC_CLASS_USAGE(cname) \
    void RmForceDynamicClassUsage_##cname() \
    { cname * dummy_##cname = NEW cname; delete dummy_##cname; }
#else
#define DECLARE_FORCE_DYNAMIC_CLASS_USAGE(cname)
#define IMPLEMENT_FORCE_DYNAMIC_CLASS_USAGE(cname)
#endif


namespace rootmap
{
    enum
    {
        ORGANIC_CARBON_NUMBER,
        ORGANIC_NITROGEN_NUMBER,
        ORGANIC_NITROGEN_REQUESTED_NUMBER,
        ORGANIC_NITROGEN_RECEIVED_BUFFER,
        RESIDUE_COVER_NUMBER
    };

    class OrganicMatterInput;

    class OrganicMatter : public Process
    {
    public:
        DECLARE_DYNAMIC_CLASS(OrganicMatter)
        DECLARE_DYNAMIC_CLASS_FORCE_USE(OrganicMatter)

        // DECLARE_FORCE_DYNAMIC_CLASS_USAGE(OrganicMatter)
        // static const OrganicMatter ForceClassUsageOrganicMatter;

        OrganicMatter();
        ~OrganicMatter();

        virtual bool DoesOverride() const;

        virtual long int Initialise(ProcessActionDescriptor* start);
        virtual long int StartUp(ProcessActionDescriptor* start);
        virtual long int Ending(ProcessActionDescriptor* end);
        virtual long int WakeUp(ProcessActionDescriptor* action);

        virtual long int DoDelayedReceivalReaction(ProcessActionDescriptor* action);
        long int TransferReceivalsToInputs(ProcessActionDescriptor* action);
        long int DecayInputs(ProcessActionDescriptor* action);

        virtual long int DoSpecialInput(ProcessActionDescriptor* action); // 'Inpu'
        long int DoOrganicInputMessage(ProcessActionDescriptor* action);

        virtual void DisposeSpecialInformation(Scoreboard* scoreboard);

        void AddInput(Scoreboard* scoreboard, OrganicMatterInput* input);

    private:
        long int number_of_inputs;
        long int mineral_nitrogen_index;
        InterprocessDataDescriptor* mineralisation_cough_up;
        InterprocessDataDescriptor* evolved_carbon_cough_up;
        InterprocessDataDescriptor* immobilisation_request;
        long int time_of_previous_decay;

        long NUM_X, NUM_Y, NUM_Z; // number of layers in each dimension
        BoxIndex BOX_COUNT; // How many boxes do we have?
    };
} /* namespace rootmap */

#endif // #ifndef OrganicMatter_H
