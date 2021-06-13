#ifndef PlantType_H
#define PlantType_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ClassTemplate.h
// Purpose:     Declaration of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/common/Types.h"

#include "core/common/Structures.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "simulation/common/Types.h"

namespace rootmap
{
    class PlantType
    {
    public:

        /**
         *
         */
        PlantType(const std::string& new_name,
            PlantTypeIdentifier new_id,
#ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
            double roots_or_foliage,
            double vegetate_or_reproduce,
            double structure_or_photosynthesize,
#endif // #ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
            double initial_seminal_deflection,
            double germination_lag,
            double first_seminal_probability,
            double temperature_of_zero_growth);


        /**
         *
         */
        ~PlantType();


        //  basic identification
        void GetName(std::string& theName);
        const std::string& GetName();
        long GetID(void);

#ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
        // "partitioning"
        double GetRootsProbability();
        double GetFoliageProbability();
        double GetVegetateProbability();
        double GetReproduceProbability();
        double GetStructureProbability();
        double GetPhotosynthesizeProbability();
#endif // #ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT

        // starting off...
        //    double GetInitialSeminalDeflection() { return ((double)m_initial_seminal_deflection); }
        //    double GetGerminationLag() { return ((double)m_germination_lag); }
        //    double GetFirstSeminalProbability() { return ((double)m_first_seminal_probability); }
        //    double GetTemperatureOfZeroGrowth() { return ((double)m_temperature_of_zero_growth); }

    private:
        // basic identification
        long m_id;
        std::string m_name;

#ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
        // "partitioning"
        double    m_roots_or_foliage;
        double    m_vegetate_or_reproduce;
        double    m_structure_or_photosynthesize;
#endif // #ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT

        // starting off...
        double m_initial_seminal_deflection;
        double m_germination_lag;
        double m_first_seminal_probability;
        double m_temperature_of_zero_growth;
    };

    inline void PlantType::GetName(std::string& theName)
    {
        theName = m_name;
    }

    inline const std::string& PlantType::GetName()
    {
        return m_name;
    }

    inline long PlantType::GetID(void)
    {
        return (m_id);
    }

#ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
    inline double PlantType::GetRootsProbability() { return (m_roots_or_foliage); }
    inline double PlantType::GetFoliageProbability() { return (1 - m_roots_or_foliage); }
    inline double PlantType::GetVegetateProbability() { return (m_vegetate_or_reproduce); }
    inline double PlantType::GetReproduceProbability() { return (1 - m_vegetate_or_reproduce); }
    inline double PlantType::GetStructureProbability() { return (m_structure_or_photosynthesize); }
    inline double PlantType::GetPhotosynthesizeProbability() { return (1 - m_structure_or_photosynthesize); }
#endif // #ifdef ROOTMAP_PLANTTYPE_OBSOLETE_FOR_THE_MOMENT
} /* namespace rootmap */

#endif // #ifndef PlantType_H
