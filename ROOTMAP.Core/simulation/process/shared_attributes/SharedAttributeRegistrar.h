// 
// Filename:
//  SharedAttributeRegistrar.h
//
// Author:
//  Robert van Hugten
//
// Description:
//  The SharedAttributeRegistrar 


#ifndef SharedAttributeRegistrar_H
#define SharedAttributeRegistrar_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"

namespace rootmap
{
    class SharedAttributeRegistrar
    {
    public:
        /**
         * Search for the variation with the given name
         * @return the variation, null if not found.
         */
        static SharedAttributeVariation* FindVariation(const std::string& name);

        /**
         * Search for the variation with the given name
         * @return the variation, null if not found.
         */
        static SharedAttributeOwner* FindOwner(const std::string& name);

        /**
         * Search for the variation with the given name
         * @return the variation, null if not found.
         */
        static SharedAttributeSupplier* FindSupplier(const std::string& name);

        /**
         * Search for the variation with the given name
         * @return the variation, null if not found.
         */
        static SharedAttributeManager* FindManager(const std::string& name);

        /**
         * Search for the variation which has a value with the given name.
         * eg. given "Plant 1" may find the variation "Plant"
         * @return the variation, null if not found.
         */
        static SharedAttributeVariation* FindVariationValue(const std::string& value);


        //
        // Called by subclasses of SharedAttributeVariation during construction
        //
        static void RegisterVariation(SharedAttributeVariation* sav);

        //
        // Called by subclasses of SharedAttributeOwner during construction
        //
        static void RegisterOwner(SharedAttributeOwner* sao);

        //
        // Called by subclasses of SharedAttributeSupplier during construction
        //
        static void RegisterSupplier(SharedAttributeSupplier* sao);

        //
        // Called by subclasses of SharedAttributeManager during construction
        //
        static void RegisterManager(SharedAttributeManager* sam);

        /**
         * Registers the variation value with a variation.
         * eg. will associate "Plant 1" with the variation named "Plant"
         */
        static void RegisterVariationValue(const std::string& vvalue, SharedAttributeVariation* sav);

        //
        // Called by SharedAttributeManager when Saving to file
        //
        static const SharedAttributeOwnerMap& GetRegisteredOwners();


        //
        // Called by the Simulation during destruction
        //
        static void DeregisterAll();


        //
        // Used by SharedAttributeAggregation to initialise
        //
        //    static const SharedAttributeOwnerMap & GetRegisteredOwners();

    private:
        //
        // This is the list of variations that have been declared in the
        // input (xml) file, from which attributes may be created.
        static SharedAttributeVariationMap myRegisteredVariations;


        //
        // This is the list of owners that have registered during construction.
        static SharedAttributeOwnerMap myRegisteredOwners;


        //
        // This is the list of suppliers that have registered during construction.
        static SharedAttributeSupplierMap myRegisteredSuppliers;

        typedef std::map<std::string, std::string> SharedAttributeVariationValueMap;
        //
        // This is the map of variation values that have been created
        static SharedAttributeVariationValueMap myVariationValues;


        //
        // This is the list of managers that have registered during construction.
        //
        static SharedAttributeManagerMap myRegisteredManagers;

        std::vector<std::string> myRegisteredManagersSupported;
    };
} /* namespace rootmap */

#endif    // SharedAttributeRegistrar_H
