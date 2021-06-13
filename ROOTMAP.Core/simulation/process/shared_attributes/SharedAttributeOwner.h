#ifndef SharedAttributeOwner_H
#define SharedAttributeOwner_H
//
//  Filename:       SharedAttributeOwner.h
//
//  Author:         Robert van Hugten
//
//                  
//
//
//  Created:        20020821 (21 July 2002)
//
//  Modified:


#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/log/Logger.h"

namespace rootmap
{
    struct CharacteristicDescriptor;

    /**
     * A SharedAttributeOwner is a class that is responsible for a SharedAttribute.
     * At this stage, that means that it registers the attribute and stores the
     * CharacteristicDescriptor.
     *
     * A SharedAttributeOwner tracks any class that causes variations of its
     * attributes, and creates new Characteristics when new instances of variation
     * classes are created.
     *
     * For example, if a SharedAttribute varies by Plant, and a new plant is created,
     * the owner will cause a new Characteristic to come into being.
     *
     * If the SharedAttribute also varies by RootOrder, then that many characteristics
     * will be created.
     *
     * Exactly which SharedAttributes a class owns depends on what has been declared
     * in each SharedAttributes XML file
     *
     * Current examples are :
     *   ProcessSharedAttributeOwner
     *
     * TODO: revise the following paragraph
     * For example, the GeotropismIndex might be stored in either of
     * PlantCoordinator or Scoreboard, depending on what the user (modeller/scientist)
     * required for a particular scenario.
     */
    class SharedAttributeOwner
    {
    public:
        virtual const std::string& GetSharedAttributeOwnerName() const = 0;

        /**
         *
         *
         *
         */
        void MakeAttributes(CharacteristicDescriptor* cd,
            SharedAttributeSupplier* supplier,
            const SharedAttributeVariationList& variations,
            SharedAttributeVariation* default_variation,
            const std::vector<double>& defaults_values
        );


        /**
         * Called by the Owner on itself when a condition that changes the number
         * of variable variants/variations occurs.  For instance, the PlantCoordinator
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
         * Called by the Manager on all owners when a condition that changes the
         * number of variable variants/variations occurs.  For instance, the
         * PlantCoordinator would call this when a new Plant or PlantType is created.
         *
         * This invokes a search through all of this owner's AttributeFamilies for
         * any that have a variation that matches the given.  If the SAFamily has
         * the variation, it is called upon to make SharedAttributes based on that
         * variation.
         */
        virtual void DoVariationChange(const char* variation_name,
            long int change
        );


        /**
         * Called by a SharedAttributeFamily during MakeAttributes(), for each
         * permutation of variations which requires a new attribute to be made.
         *
         * The inclusion of the SharedAttributeSupplier in the parameters is for
         * the benefit of subclasses that wish to override this function to
         * provide supplier-specific functionality.
         *
         * Adds the given SharedAttribute to this Owner's list of myAttributes.
         * Calls the SAManager to add it to the SAIteratorAssistant's Cluster(s)
         */
        virtual void RegisterSharedAttribute(SharedAttribute* sa,
            SharedAttributeSupplier* supplier,
            const VariationNameArray& savKeyList,
            const SharedAttributeVariationList& savList
        );


        // /////////////////////////////
        // Search functions
        // "SearchFor" functions are called by the SASearchHelper.
        // "DoSearchFor" functions are called by either this owner or the SAManager
        // /////////////////////////////

        /**
         * Searches first in this owner then the other owners
         * @param attribute_name - main name of the attribute about which it varies,
         *          usually the Name in the CharacteristicDescriptor
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name) const;


        /**
         * Searches first in this owner then the other owners. Optimised for only 1
         * variation, where it would be unnecessarily painful to form a list to pass
         * to the SearchHelper.
         *
         * Note that the attribute must vary ONLY by the specified variation for the
         * attribute search to succeed
         *
         * @param attribute_name - main name of the attribute about which it varies,
         *          usually the Name in the CharacteristicDescriptor
         * @param variation_name - the name of the variation to search for, eg. "Plant"
         *          or "RootOrder"
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::string& variation_name) const;


        /**
         * Note that the attribute must vary ONLY by the specified variation for the
         * attribute search to succeed
         *
         * @param attribute_name - main name of the attribute about which it varies,
         *          usually the Name in the CharacteristicDescriptor
         * @param variation_name - the name of the variation to search for, eg. "Plant"
         *          or "RootOrder"
         * @param variation_string - the name of the actual variation. Eg. the actual
         *          name of the Plant.
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::string& variation_name,
            const std::string& variation_string) const;


        /**
         * Searches first in this owner then the other owners. Optimised for multiple
         * variations, where it would be unnecessarily painful to form a list to pass
         * to the SearchHelper.
         *
         * Note that ALL variations but ONLY these variations must be present for
         * the attribute search to succeed.
         *
         * @param attribute_name - main name of the attribute about which it varies,
         *          usually the Name in the CharacteristicDescriptor
         * @param v_names - the list of names of variations to search for, eg. "Plant"
         *          or "RootOrder"
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::vector<std::string>& v_names) const;


        /**
         * Searches first in this owner then the other owners. Optimised for multiple
         * variations, where it would be unnecessarily painful to form a list to pass
         * to the SearchHelper.
         *
         * Note that ALL variations but ONLY these variations must be present for
         * the attribute search to succeed.
         *
         * @param attribute_name - main name of the attribute about which it varies,
         *          usually the Name in the CharacteristicDescriptor
         * @param v_names - the list of names of variations to search for, eg. "Plant"
         *          or "RootOrder"
         */
        SharedAttribute* SearchForAttribute(const std::string& attribute_name,
            const std::vector<std::string>& v_names,
            const std::string& variation_string) const;


        /**
         * DONE
         */
        SharedAttribute* DoSearchForAttribute(const std::string& attribute_name) const;

        /**
         * DONE
         */
        SharedAttribute* DoSearchForAttribute(const std::string& attribute_name,
            const std::string& variation_name) const;

        /**
         * Used by Plant to find its own attributes (summaries?), eg.
         *
         * saWaterUptake = helper.SearchForAttribute("Water Uptake","Plant",plant_name.c_str());
         */
        SharedAttribute* DoSearchForAttribute(const std::string& attribute_name,
            const std::string& variation_name,
            const std::string& variation_string) const;

        /**
         *
         */
        SharedAttribute* DoSearchForAttribute(const std::string& attribute_name,
            const std::vector<std::string>& v_names) const;

        /**
         *
         */
        SharedAttribute* DoSearchForAttribute(const std::string& attribute_name,
            const std::vector<std::string>& v_names,
            const std::string& variation_string) const;


        /**
         * Sets the "Default" field of the attribute's CharacteristicDescriptor.
         */
        void SetSharedAttributeInitialValue(const std::string& attr_name,
            double value);


        SharedAttributeOwner();
        virtual ~SharedAttributeOwner();


    private:
        RootMapLoggerDeclaration();

        /**
         * The list of SharedAttributeFamilies which use permutations of
         * variations to make actual SharedAttributes.
         */
        SharedAttributeFamilyList myAttributeFamilies;


        /**
         * The collection of actual SharedAttributes that have been created from
         * the permutations of variations in the attribute families.
         *
         * For each of these attributes, one characteristic or attribute has been
         * allocated storage somewhere.  At the moment, that means either in the
         * scoreboard or a ProcessAttributeSupplier.
         */
        SharedAttributeList myAttributes;
    };
} /* namespace rootmap */

#endif    // SharedAttributeOwner_H
