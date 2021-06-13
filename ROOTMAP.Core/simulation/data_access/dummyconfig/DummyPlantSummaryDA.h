#ifndef DummyPlantSummaryDA_H
#define DummyPlantSummaryDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPlantSummaryDA.h
// Purpose:     Declaration of the DummyPlantSummaryDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/PlantSummaryDAI.h"

namespace rootmap
{
    class DummyPlantSummaryDA : public PlantSummaryDAI
    {
    public:

        virtual std::string getName() const;
        virtual ScoreboardStratum getStratum() const;
        virtual PlantSummaryIdentifier getIdentifier() const;

        virtual WrapDirection getWrapDirection() const;
        virtual SummaryRoutine getSummaryType() const;

        virtual std::vector<std::string> getPlants() const;

        DummyPlantSummaryDA(const std::string& name,
            const ScoreboardStratum& stratum,
            PlantSummaryIdentifier identifier,
            WrapDirection wrap_direction,
            SummaryRoutine summary_type,
            const std::vector<std::string>& plants);
        virtual ~DummyPlantSummaryDA();

    private:

        ///
        /// member declaration
        std::string m_name;
        ScoreboardStratum m_stratum;
        PlantSummaryIdentifier m_identifier;
        WrapDirection m_wrapDirection;
        SummaryRoutine m_summaryType;
        std::vector<std::string> m_plants;
    }; // class DummyPlantSummaryDA
} /* namespace rootmap */

#endif // #ifndef DummyPlantSummaryDA_H
