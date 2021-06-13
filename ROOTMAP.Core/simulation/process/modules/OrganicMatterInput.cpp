#include "simulation/process/modules/OrganicMatterInput.h"
#include <math.h>


namespace rootmap
{
    OrganicMatterInput::OrganicMatterInput()
        : SpecialScoreboardData()
        , CfracNonN(1.0)
        , original_carbon(0.0)
        , original_nitrogen(0.0)
        , original_weight(0.0)
        , carbon(0.0)
        , nitrogen(0.0)
        , requested_nitrogen(0.0)
        , position(0, 0, 0)
        , time_of_arrival(0)
        , next_organic_matter_input(0)
    {
    }

    /*OrganicMatterInput::OrganicMatterInput(double weight, double initial_n_fraction, double non_n_frac_c, long t, BoxCoordinate *pos)*/
    OrganicMatterInput::OrganicMatterInput(double org_C, double org_N, double current_C, double current_N, long t, BoxCoordinate* pos)
        : CfracNonN(0.3)
        , original_carbon(org_C)
        , original_nitrogen(org_N)
        , original_weight(0.0)
        , carbon(current_C)
        , nitrogen(current_N)
        , requested_nitrogen(0.0)
        , position(*pos)
        , time_of_arrival(t)
        , next_organic_matter_input(0)
    {
        original_weight = original_nitrogen + (original_carbon / CfracNonN);

        /*    original_weight = weight;
            InitNfrac = initial_n_fraction;
            CfracNonN = non_n_frac_c;
            nitrogen = original_nitrogen = weight * InitNfrac;
            carbon = original_carbon = (weight - original_nitrogen) * CfracNonN;*/
    }

    OrganicMatterInput::~OrganicMatterInput()
    {
    }

    void OrganicMatterInput::GetPosition(BoxCoordinate* pos)
    {
        *pos = position;
    }

    // Decay
    // I don't know whether this is the best name for this function or
    // not. It has three main values it needs to calculate :
    // 1.    requested nitrogen. This is the amount of immobilised nitrogen that
    //     this input is requesting to carry out its actions.
    // 2.    mineralisation
    // 3.    respiration, or evolved carbon.
    // 
    // The other things (eg. current nitrogen & carbon) are calculated elsewhere
    // 
    void OrganicMatterInput::Decay
    (double* requested_immobilisation, // a rate !!
        double* mineralisation, // value, ie. scaled by dT
        double* respiration, // value, ie. scaled by dT
        double dT)
    {
        double current_weight = nitrogen + (carbon / CfracNonN);
        double N_fraction = nitrogen / current_weight;
        double reactivity = current_weight / original_weight;

        // NOTE : in the STELLA ArtMod version, mineralisation and respiration are rates.
        // Since in this model we are coughing them up, they need to be expressed as actual
        // amounts. This is why dT is included in these equations instead of the two that
        // calculate the current amount of
        *mineralisation = reactivity * nitrogen * 2 * pow(N_fraction, 1.2) * dT;

        *respiration = (carbon > 0) ? (reactivity * 0.05 * nitrogen * dT) : 0.0;

        carbon -= *respiration;

        nitrogen -= *mineralisation;


        // Now is the time to calculate an update of our nitrogen demands.
        N_fraction = nitrogen / (nitrogen + carbon / CfracNonN);

        // AS PER STELLA ArtMod
        // requested_immobilisation = carbon * reactivity * 0.1 * N_fraction / (N_fraction + 1.39);
        // BUT SINCE reactivity * N_fraction = nitrogen / original_weight, i use... */
        *requested_immobilisation =
            requested_nitrogen =
            carbon * (nitrogen / original_weight) * 0.1 / (N_fraction + 1.39);
        //    to remove error due to rounding off. While we can.
    }

    /* SetNextInput
    Recursive routine to add a "input" to the end of the linked list */
    void OrganicMatterInput::SetNextInput(OrganicMatterInput* next_input)
    {
        if (GetNextInput() != 0)
        {
            GetNextInput()->SetNextInput(next_input);
        }
        else
        {
            next_organic_matter_input = next_input;
        }
    }
} /* namespace rootmap */
