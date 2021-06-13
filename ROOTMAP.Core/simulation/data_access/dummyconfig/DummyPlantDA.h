#ifndef DummyPlantDA_H
#define DummyPlantDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DummyPlantDA.h
// Purpose:     Declaration of the DummyPlantDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/dummyconfig/DummyProcessDA.h"
#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/data_access/interface/PlantDAI.h"
#include "simulation/common/Types.h"


namespace rootmap
{
    class DummyCharacteristicDA;

    class DummyPlantDA
        : public PlantDAI
        , public DummyProcessDA
    {
    public:
        // ////////////////////////////
        //      PlantDAI stuff
        /**
         * retrieves the name of the PlantType
         */
        virtual std::string getTypeName() const;

        virtual DoubleCoordinate getOrigin() const;
        virtual DoubleCoordinate getSeedLocation() const;
        virtual ProcessTime_t getSeedingTime() const;

        virtual PossibleAxis* getPossibleSeminals() const;
        virtual PossibleAxis* getPossibleNodals() const;


        DummyPlantDA(const std::string& name,
            const std::string& type_name,
            ProcessIdentifier identifier,
            ScoreboardStratum stratum,
            ProcessActivity activity,
            const DoubleCoordinate& origin,
            const DoubleCoordinate& seed_location,
            ProcessTime_t seeding_time,
            PossibleAxis* possible_seminals,
            PossibleAxis* possible_nodals
        );

        virtual ~DummyPlantDA();


    private:

        ///
        /// member declaration
        std::string m_typeName;

        DoubleCoordinate m_origin;
        DoubleCoordinate m_seedLocation;
        ProcessTime_t m_seedingTime;

        PossibleAxis* m_possibleSeminals;
        PossibleAxis* m_possibleNodals;
    }; // class DummyPlantDA
} /* namespace rootmap */

#endif // #ifndef DummyPlantDA_H
