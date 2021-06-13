// 
// Filename:
//  SharedAttributeManager.h
//
// Author:
//  Robert van Hugten
//
// Description:
//  The SharedAttributeManager 


#ifndef SharedAttributeManager_H
#define SharedAttributeManager_H

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/log/Logger.h"

namespace rootmap
{
    class SimulationEngine;
    class CharacteristicDAI;
    class DataAccessManager;
    struct CharacteristicDescriptor;
    class SharedAttributeIteratorAssistant;

    class SharedAttributeManager
    {
    public:
        /**
         * construction and destruction.
         */
        SharedAttributeManager(SimulationEngine* engine,
            DataAccessManager& dam,
            const char* name);

        virtual ~SharedAttributeManager();

        /**
         *
         */
        void Initialise();


        /**
         * Creates all the attributes described by the CharacteristicDescriptor
         * and variation_names
         */
        void MakeAttributes(CharacteristicDescriptor* cd,
            const std::string& owner_name,
            const std::string& supplier_name,
            const std::vector<std::string>& variation_names,
            const std::string& defaults_name,
            const std::vector<double>& defaults_values
        );


        /**
         * Creates all the attributes described by the CharacteristicDAI
         * and variation_names
         */
        void MakeAttributes(CharacteristicDAI& data,
            const std::string& owner_name,
            const std::string& supplier_name,
            const std::vector<std::string>& variation_names,
            const std::string& defaults_name,
            const std::vector<double>& defaults_values
        );


        /**
         *
         */
        void SetSharedAttributeInitialValue(const std::string& owner_name,
            const std::string& attr_name,
            double value
        );


        /**
         *
         * Called by an Owner when a condition that changes the number of variants
         * (subject of a variation) occurs.  For instance, the PlantCoordinator
         * would call this when a new Plant or PlantType is created.
         *
         * variation_name : the name of the variation.  Must be on the registered
         *                  list, eg. "Plant" or "PlantType"
         * change         : the number of changes.  Can really only be ±1, although
         *                  -1 (deletion) is not yet supported.
         */
        void VariationChange(const char* variation_name,
            long int change);


        /**
         *
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            SharedAttributeOwner* owner);


        /**
         *
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::string& variation_name,
            SharedAttributeOwner* owner);


        /**
         *
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::string& variation_name,
            const std::string& variation_string,
            SharedAttributeOwner* owner);

        /**
         *
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::vector<std::string>& variation_names,
            SharedAttributeOwner* owner);


        /**
         *
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::vector<std::string>& variation_names,
            const std::string& variation_string,
            SharedAttributeOwner* owner);

        /**
         *
         */
        virtual const std::string& GetSharedAttributeManagerName();


        /**
         * Search for the index of an Attribute within the special "No variation" cluster
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name);


        /**
         * Search for the index of an Attribute within the variation cluster
         *
         * @param attribute_name the base name of the characteristic
         * @param cluster_name the variation cluster to search in. This should also
         *          be the only variation for this attribute
         *
         * @return the index of the attribute within the cluster of the given name
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name,
            const char* cluster_name);


        /**
         * Search for the index of an Attribute within the variation cluster
         *
         * @param attribute_name the base name of the characteristic
         * @param cluster_name the variation cluster to search in. This should also
         *          be the only variation for this attribute
         * @param variation_names all the variations for the attribute
         *
         * @return the index of the attribute within the cluster of the given name
         */
        CharacteristicIndex SearchForClusterIndex(const char* attribute_name,
            const char* cluster_name,
            const std::vector<std::string>& variation_names);


        /**
         *
         */
        const SharedAttributeCluster& GetCluster(const char* variation_name) const;


        /**
         *
         */
        void ClusteriseSharedAttribute(SharedAttribute* sa,
            const VariationNameArray& varKeyList,
            const SharedAttributeVariationList& varList
        );


        /**
         * Grandiose named function to ensure that it is not used by mistake.
         */
        void LogSharedAttributeIteratorAssistantContents();

    protected:
        /**
         *
         */
        SharedAttribute* MakeAttribute(CharacteristicDescriptor* cd,
            SharedAttributeOwner* owner,
            SharedAttributeSupplier* supplier,
            long variation
        );

    private:
        RootMapLoggerDeclaration();
        /**
         *  Identifier of the group of classes that are managed by this instance.
         */
        std::string myName;

        /**
         *
         */
        SimulationEngine* myEngine;

        /**
         *  Owners that have called MakeAttributes(), used for Broadcasting the
         *  VariationChange()
         */
        SharedAttributeOwnerSet myOwners;


        /**
         *
         */
        SharedAttributeIteratorAssistant* myAttributeIteratorAssistant;
    };
} /* namespace rootmap */

#endif    // SharedAttributeManager_H
