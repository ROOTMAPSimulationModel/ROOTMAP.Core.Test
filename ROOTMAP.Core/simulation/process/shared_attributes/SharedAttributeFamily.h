#ifndef SharedAttributeFamily_H
#define SharedAttributeFamily_H
/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeFamily.h
// Purpose:     Declaration of the ClassTemplate class
// Created:     16/10/2002
// Author:      RvH
// $Date: 2008-11-06 00:38:15 +0900 (Thu, 06 Nov 2008) $
// $Revision: 24 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "core/log/Logger.h"

#define SHAREDATTRIBUTEFAMILY_DOMAKETHESHAREDATTRIBUTE

namespace rootmap
{
    struct CharacteristicDescriptor;

    /**
     * A SharedAttributeFamily encompasses all the SharedAttributes for a given
     * SharedAttribute description. It starts with the base CharacteristicDescriptor,
     * and uses a SharedAttributeSupplier to create SharedAttributes based on a
     * given set of variations.
     */
    class SharedAttributeFamily
    {
    public:
        /**
         *
         */
        SharedAttributeFamily(CharacteristicDescriptor* cd,
            SharedAttributeSupplier* supplier,
            const SharedAttributeVariationList& variations,
            SharedAttributeVariation* default_variation,
            const std::vector<double>& defaults_values);

        /**
         *
         */
        SharedAttributeFamily();

        /**
         *  This class owns its CharacteristicDescriptor, as it creates a new CD (and
         *  Name and Units std::string) for each and every attribute that it makes.
         */
        virtual ~SharedAttributeFamily();

        /**
         * Searches for the Variation in this Family's collection
         */
        SharedAttributeVariation* SearchForVariation(const std::string& variation_name);

        /**
         * returns true if the specified variation matches this class' only
         * variation. If this class has >1 variation, even if the given one
         * matches, this will return false;
         */
        bool MatchesVariation(const SharedAttributeVariation& variation) const;
        bool MatchesVariation(const std::string& variation) const;

        /**
         *
         */
        bool MatchesAllVariations(const SharedAttributeVariationList& variations) const;
        bool MatchesAllVariations(const VariationNameArray& variations) const;

        /**
         *
         */
        SharedAttributeSupplier* GetSupplier();

        /**
         * Accessor for the Descriptor that specifies the main Characteristic
         * about which the Attributes vary
         */
        CharacteristicDescriptor* GetCharacteristicDescriptor();

        const CharacteristicDescriptor* GetCharacteristicDescriptor() const;

        /**
         * Make just those attributes that vary about the given variation
         *
         * Makes SharedAttributes for the variations in this Family, based on the
         * given variation.  That is, for all permutations of variations *other than*
         * the given one, it will make SharedAttributes.  For the given variation,
         * it uses the last value.
         */
        void MakeVariationAttributes(
            SharedAttributeVariation* about_variation,
            SharedAttributeOwner* owner);


        /**
         * Makes SharedAttributes for all the variations in this Family.  That way,
         * if all of the variations have something to vary, something will be made.
         *
         * This is usually called before any of the variable variations invoke the
         * other overloaded version of this function.
         */
        void MakeAllAttributes(SharedAttributeOwner* owner);


        /**
         * Makes a string that is the full attribute name of the first permutation
         * of our variations, except for the "about_variation", for which the
         * "about_variation_string" is used.
         *
         * The subsequent string is used to search through our owner's full list,
         * to find the first attribute for use by a SharedAttributeUser.
         */
        void MakeSearchString(SharedAttributeVariation* about_variation,
            const std::string& about_variation_string,
            std::string& search_string);


        std::string MakeSharedAttributeName0() const;


        /**
         * VariationIndices are signed longs, so that <0 means they are not used
         */
        typedef signed long int VariationIndexType;

        /**
         * type definition for the array of variation indices, ie. an array of
         * the index into each variation
         *
         * eg. if an Attribute varies by Plant and by RootOrder, there will be
         * 2 items in myVariations: PlantSAV and RootOrderSAV. For each of these
         * there will be an index in this list
         *
         * When iterating over the indices in the VariationList, these are
         * usually known as permutations.
         */
        typedef std::vector<VariationIndexType> SharedAttributeVariationListIndices;

    private:
        /**
         * Called by MakeAllAttributes for the case where myVariations is not empty
         */
        void MakeAllAttributesVaried(SharedAttributeOwner* owner);


        void DoMakeTheSharedAttribute
        (SharedAttributeOwner* owner,
            SharedAttributeVariationListIndices& permutation);

        /**
         * Low-level function to construct the full attribute name given the
         * variations and their names, the main characteristic name and the
         * current variation permutation (ie. the indices into the variations)
         * various parts and the place
         *
         * @param variations - ref to myVariations
         * @param permutations - array of indices pointing at current variations
         * @param characteristic_name - ref to myCharacteristicDescriptor->Name
         * @param variationNames - array of names, built by this function, used
         *        when calling our owner RegisterSharedAttribute
         *
         */
        std::string MakeSharedAttributeName
        (const SharedAttributeVariationList& variations,
            const SharedAttributeVariationListIndices& permutation,
            const std::string& characteristic_name,
            VariationNameArray& variationNames);

        RootMapLoggerDeclaration();

        /**
         * The variations, if any
         */
        SharedAttributeVariationList myVariations;

        /**
         * The base Characteristic around which the attributes vary
         */
        CharacteristicDescriptor* myCharacteristicDescriptor;

        /**
         * The attribute creator
         */
        SharedAttributeSupplier* mySupplier;

        /**
         *
         */
        SharedAttributeVariation* myDefaultVariation;

        /**
         *
         */
        VariationIndexType myDefaultVariationPermutationIndex;

        /**
         *
         */
        std::vector<double> myDefaultValues;
    };
} /* namespace rootmap */

#endif    // SharedAttributeFamily_H
