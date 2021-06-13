#ifndef OrganicMatterInput_H
#define OrganicMatterInput_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OrganicMatterInput.h
// Purpose:     Declaration of the OrganicMatterInput class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Structures.h"
#include "core/common/Types.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "simulation/common/Types.h"
#include "simulation/scoreboard/Scoreboard.h"

namespace rootmap
{
    class OrganicMatterInput
        : public SpecialScoreboardData
    {
    public:
        //TCL_DECLARE_CLASS

    private:
        //    double InitNfrac;
        double CfracNonN;

        double nitrogen;
        double carbon;

        double original_nitrogen;
        double original_carbon;
        double original_weight;

        double requested_nitrogen;

        BoxCoordinate position;

        ProcessTime_t time_of_arrival;

        OrganicMatterInput* next_organic_matter_input;


    public:
        // construction and destruction
        OrganicMatterInput(); // for use with "ReadFrom"
        OrganicMatterInput(double org_C, double org_N, double current_C, double current_N, long t, BoxCoordinate* pos);
        ~OrganicMatterInput();

        // Accessing information
        double GetNitrogen() { return (nitrogen); }
        double GetCarbon() { return (carbon); }

        //    double GetOriginalWeight() { return (original_weight); }
        //    double ResidueAdded() { return (original_weight); }
        //    long int GetArrivalTime() { return (time_of_arrival); }
        //    void GetPosition(DoubleCoordinate* pos);
        void GetPosition(BoxCoordinate* pos);

        //    void SetNitrogen(double nitrogen_amount) { nitrogen = nitrogen_amount; }
        //    void SetCarbon(double carbon_amount) { carbon = carbon_amount; }

        double AddNitrogen(double nitrogen_amount) { return (nitrogen += nitrogen_amount); }
        //    double AddCarbon(double carbon_amount) { return (carbon += carbon_amount); }

        void Decay(double* requested_immobilisation, double* mineralisation, double* respiration, double dT);
        double GetRequestedNitrogen() { return (requested_nitrogen); }

        //    Boolean IsNitrogenLessThan10percent() { return ((nitrogen/original_nitrogen) < 0.1); }
        //    Boolean IsCarbonLessThan10percent() { return ((carbon/original_carbon) < 0.1); }

        OrganicMatterInput* GetNextInput() { return (next_organic_matter_input); }
        void SetNextInput(OrganicMatterInput* next_input);
    };

    struct OrganicMatterInput_file_data
    {
        double omifd_CfracNonN;
        double omifd_nitrogen;
        double omifd_carbon;

        double omifd_original_nitrogen;
        double omifd_original_carbon;
        double omifd_original_weight;

        BoxCoordinate omifd_position;

        long int omifd_time_of_arrival;
    };
} /* namespace rootmap */

#endif // #ifndef OrganicMatterInput_H
