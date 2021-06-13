/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Constants.h"
#include "core/common/Structures.h"
#include "core/utility/Utility.h"

#include "simulation/process/plant/PlantType.h"


namespace rootmap
{
    /* PlantType
    Main constructor */
    PlantType::PlantType
    (const std::string& new_name,
        long new_id,
#ifdef CS_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
        double roots_or_foliage,
        double vegetate_or_reproduce,
        double structure_or_photosynthesize,
#endif // #ifdef CS_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
        double initial_seminal_deflection,
        double germination_lag,
        double first_seminal_probability,
        double temperature_of_zero_growth)
    {
        m_id = new_id;

        m_name = new_name;

#ifdef CS_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
        m_roots_or_foliage = roots_or_foliage;
        m_vegetate_or_reproduce = vegetate_or_reproduce;
        m_structure_or_photosynthesize = structure_or_photosynthesize;
#endif // #ifdef CS_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT

        m_initial_seminal_deflection = initial_seminal_deflection;
        m_germination_lag = germination_lag;
        m_first_seminal_probability = first_seminal_probability;
        m_temperature_of_zero_growth = temperature_of_zero_growth;
    }

    /* Destructor
    When this class goes, all the Plants that depend upon it need to go also. */
    PlantType::~PlantType()
    {
        // Some iterative loop to remove collaboratorship of all plants of this plant type and
        // then destroy them.
    }
} /* namespace rootmap */


