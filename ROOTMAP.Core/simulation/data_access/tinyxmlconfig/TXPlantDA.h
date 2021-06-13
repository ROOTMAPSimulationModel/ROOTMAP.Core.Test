#ifndef TXPlantDA_H
#define TXPlantDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXPlantDA.h
// Purpose:     Declaration of the TXPlantDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"
#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/data_access/interface/PlantDAI.h"
#include "simulation/common/Types.h"

#pragma warning( disable : 4250 ) // 'rootmap::TXPlantDA' : inherits 'rootmap::TXProcessDA::rootmap::TXProcessDA::XXX' via dominance

namespace rootmap
{
    class DummyCharacteristicDA;

    class TXPlantDA
        : public PlantDAI
        , public TXProcessDA
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

        // If relinquish == true, these getters will hand the pointer off to the caller
        // and erase the local copy.
        virtual PossibleAxis* getPossibleSeminals(bool relinquish = true) const;
        virtual PossibleAxis* getPossibleNodals(bool relinquish = true) const;


        TXPlantDA(const std::string& name,
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

        virtual ~TXPlantDA();


    private:

        ///
        /// member declaration
        std::string m_typeName;

        DoubleCoordinate m_origin;
        DoubleCoordinate m_seedLocation;
        ProcessTime_t m_seedingTime;

        PossibleAxis* m_possibleSeminals;
        PossibleAxis* m_possibleNodals;

        //using TXProcessDA::getIdentifier;
        //using TXProcessDA::getName;
        //using TXProcessDA::getStratum;
        //using TXProcessDA::getActivity;
        //using TXProcessDA::doesOverride;
        //using TXProcessDA::getCharacteristics;
    }; // class TXPlantDA
} /* namespace rootmap */

#endif // #ifndef TXPlantDA_H
