/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeFamily.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     16/10/2002
// Author:      RvH
// $Date: 2008-11-06 00:38:15 +0900 (Thu, 06 Nov 2008) $
// $Revision: 24 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/shared_attributes/SharedAttributeFamily.h"
#include "simulation/process/shared_attributes/SharedAttributeVariation.h"
#include "simulation/process/shared_attributes/SharedAttributeOwner.h"
#include "simulation/process/shared_attributes/SharedAttributeSupplier.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "simulation/common/IdentifierUtility.h"
#include "core/utility/Utility.h"


namespace rootmap
{
    using Utility::CSMin;

    RootMapLoggerDefinition(SharedAttributeFamily);


    SharedAttributeVariation* SharedAttributeFamily::SearchForVariation(const std::string& variation_name)
    {
        std::string vname = variation_name;
        SharedAttributeVariationList::iterator var_iter = myVariations.begin();
        SharedAttributeVariation* found_variation = 0;

        while (var_iter != myVariations.end())
        {
            SharedAttributeVariation* this_variation = *var_iter;
            if (vname == this_variation->GetName())
            {
                found_variation = this_variation;
            }

            ++var_iter;
        }

        return found_variation;
    }


    // MSA 11.01.18 What is happening here? Why do these methods expect single-variation families only?
    // TODO Ensure it's safe to rewrite these so they behave sanely

    /*
    bool SharedAttributeFamily::MatchesVariation(const SharedAttributeVariation & variation) const
    {
        if (myVariations.size()!=1)
        {
            return false;
        }

        return (variation == (*(myVariations[0])) );
    }


    bool SharedAttributeFamily::MatchesVariation(const std::string & variation) const
    {
        if (myVariations.size()!=1)
        {
            return false;
        }

        return (variation == (myVariations[0]->GetName()) );
    }
    */

    bool SharedAttributeFamily::MatchesVariation(const SharedAttributeVariation& variation) const
    {
        SharedAttributeVariationList::const_iterator iter = myVariations.begin();
        while (iter != myVariations.end())
        {
            if (variation == *(*iter)) return true;
            ++iter;
        }
        return false;
    }


    bool SharedAttributeFamily::MatchesVariation(const std::string& variation) const
    {
        SharedAttributeVariationList::const_iterator iter = myVariations.begin();
        while (iter != myVariations.end())
        {
            if (variation == (*iter)->GetName()) return true;
            ++iter;
        }
        return false;
    }


    bool SharedAttributeFamily::MatchesAllVariations(const SharedAttributeVariationList& variations) const
    {
        // MSA 11.01.19 Is there any reason why myVariations couldn't be larger than the parameter VariationNameArray?
        /*
        if (myVariations.size()!=variations.size())
        {
            return false;
        }
        */
        if (myVariations.size() < variations.size())
        {
            return false;
        }

        std::set<SharedAttributeVariation *> savset(variations.begin(), variations.end());

        // now, this is pretty tedious. Need to loop over all this class variations,
        // checking there is one for each of those in the specified list
        size_t number_matched = 0;
        for (SharedAttributeVariationList::const_iterator var_iter = myVariations.begin();
            var_iter != myVariations.end(); ++var_iter)
        {
            if (savset.end() != savset.find(*var_iter))
            {
                ++number_matched;
            }
        }

        // all variations match if the number matched is equal to the number of
        // variations
        return (number_matched == savset.size());
    }


    bool SharedAttributeFamily::MatchesAllVariations(const VariationNameArray& variations) const
    {
        // MSA 11.01.18 Is there any reason why myVariations couldn't be larger than the parameter VariationNameArray?
        /*
        if (myVariations.size()!=variations.size())
        {
            return false;
        }
        */
        if (myVariations.size() < variations.size())
        {
            return false;
        }

        std::set<std::string> savset(variations.begin(), variations.end());

        // now, this is pretty tedious. Need to loop over all this class variations,
        // checking there is one for each of those in the specified list
        size_t number_matched = 0;
        for (SharedAttributeVariationList::const_iterator var_iter = myVariations.begin();
            var_iter != myVariations.end(); ++var_iter)
        {
            if (savset.end() != savset.find((*var_iter)->GetName()))
            {
                ++number_matched;
            }
        }

        // all variations match if the number matched is equal to the number of
        // variations
        return (number_matched == savset.size());
    }


    SharedAttributeSupplier* SharedAttributeFamily::GetSupplier()
    {
        return mySupplier;
    }


    CharacteristicDescriptor* SharedAttributeFamily::GetCharacteristicDescriptor()
    {
        return myCharacteristicDescriptor;
    }

    const CharacteristicDescriptor* SharedAttributeFamily::GetCharacteristicDescriptor() const
    {
        return myCharacteristicDescriptor;
    }


    std::string SharedAttributeFamily::MakeSharedAttributeName
    (const SharedAttributeVariationList& variations,
        const SharedAttributeVariationListIndices& permutations,
        const std::string& characteristic_name,
        VariationNameArray& variationNames
    )
    {
        std::string name_str = characteristic_name;

        const long variation_index_max = variations.size() - 1;
        for (long variation_index = 0;
            variation_index <= variation_index_max;
            ++variation_index)
        {
            name_str.append(1, ' ');

            SharedAttributeVariation* the_variation = variations[variation_index];

            the_variation->AppendVariationString(permutations[variation_index], name_str);

            std::string s;
            the_variation->GetVariationString(permutations[variation_index], s);
            variationNames.push_back(s);

            SharedAttributeRegistrar::RegisterVariationValue(s, the_variation);
        }

        if (characteristic_name == "Root Density Wrap None")
        {
            LOG_INFO << "WARNING: using Root Density Wrap None";
        }

        LOG_DEBUG << "Created SharedAttribute name '" << name_str.c_str()
            << "' for characteristic '" << characteristic_name.c_str() << "'";

        return name_str;
    }


    static long MakeVariationAttributes_InnerLoop(long variationIndexMax, long aboutVariationIndex, long& mostSignificantVariationIndex, SharedAttributeFamily::SharedAttributeVariationListIndices& variationIndices, const SharedAttributeFamily::SharedAttributeVariationListIndices& indexMaximums);

    static long MakeVariationAttributes_InnerLoop
    (long variationIndexMax,
        long aboutVariationIndex,
        long& mostSignificantVariationIndex,
        SharedAttributeFamily::SharedAttributeVariationListIndices& variationIndices,
        const SharedAttributeFamily::SharedAttributeVariationListIndices& indexMaximums)
    {
        //VariationInterface * vi = myVariations[variation_index];
        //long number_of = vi->GetNumberOf();
        //const char * n = vi->GetName().c_str();
        //const char * s = vi->GetString(variationIndices[mostSignificantVariationIndex]).c_str();
        long variation_index = variationIndexMax;

        //
        // "Tick over" the counter
        // is the index > max ?
        while ((variation_index >= 0) &&
            //( variationIndices[variation_index] >= ((myVariations[variation_index])->GetNumberOf()) )
            (variationIndices[variation_index] >= indexMaximums[variation_index])
            )
        {
            // yes !  Reset the index
            variationIndices[variation_index] = 0;

            variationIndices[aboutVariationIndex] = indexMaximums[aboutVariationIndex] - 1;

            // use the next-most significant variation
            variation_index--;

            // don't increment the index of the given variation.  We are
            // always using the last value in that variation.
            if (variation_index == aboutVariationIndex)
            {
                variation_index--;
            }

            // increment the variationIndices's index of that variation
            variationIndices[variation_index] += 1;

            // track the most Significant Variation that we've
            // covered the variationIndices of so far.  This is not
            // currently used.
            if (variation_index < mostSignificantVariationIndex)
            {
                mostSignificantVariationIndex = variation_index;
            }
        }

        return variation_index;
    }


    void SharedAttributeFamily::MakeVariationAttributes
    (SharedAttributeVariation* about_variation,
        SharedAttributeOwner* owner
    )
    {
        //
        //
        //  Internal working notes
        //  1.  Check if any variation still has nothing to vary (ie. GetNumberOf()
        //      returns 0 or less.  If that is the case, then there will be no valid
        //      permutations, and creation of attributes is unnecessary.
        //
        //  2.  Search for the index of the given variation in the vector container.
        //

        if (!myVariations.empty())
        {
            //
            // If any of the variations still have nothing to vary,
            // don't bother trying to make anything.
            for (unsigned long variation_idx_2 = 0;
                variation_idx_2 < myVariations.size();
                ++variation_idx_2)
            {
                if (myVariations[variation_idx_2]->GetNumberOf() <= 0)
                {
                    LOG_WARN << "Variation '" << myVariations[variation_idx_2]->GetName() << "' has nothing to vary";
                    return;
                }
            }

            //
            // Ascertain the index of the about_variation
            long about_variation_index = -1;
            for (unsigned long variation_idx_1 = 0;
                variation_idx_1 < myVariations.size();
                ++variation_idx_1)
            {
                if (myVariations[variation_idx_1]->GetName() == about_variation->GetName())
                {
                    about_variation_index = variation_idx_1;
                    LOG_DEBUG << "Found variation '" << about_variation->GetName() << "' with index " << about_variation_index;
                    break;
                }
            }


            //
            // Exit if it has nothing to do with us.
            if (about_variation_index < 0)
            {
                LOG_ERROR << "Invalid variation supplied";
                //TODO: what about it ?
                return;
            }

            //
            // set up a vector of the variations' current indices.
            //std::vector<VariationIndexArray> variation_permutations;

            const long num_variations = myVariations.size();
            const long variation_index_max = num_variations - 1;
            bool finished_this_family = false;
            bool finished_this_middle = false;

            //
            // This is the outer loop, for iterating through the list of
            // SharedAttributeFamilyList.
            while (!finished_this_family)
            {
                //
                //  Set up and initialise (to zero) the list of indices in
                //  the current permutations.
                SharedAttributeVariationListIndices permutations(num_variations, 0);

                //
                //  Set up and initialise (to ...) the list of index maximums
                //  for each variation.  This is mostly for debugging, but helps
                //  in understanding.  A little bit.
                SharedAttributeVariationListIndices index_maximums(num_variations, 0);
                for (long shlop = 0; shlop < num_variations; ++shlop)
                {
                    index_maximums[shlop] = ((myVariations[shlop])->GetNumberOf());
                }

                //
                // Get the Least Significant VariationInterface		// MSA 09.10.07 Unused
                //SharedAttributeVariation * lsvi = myVariations[variation_index_max];
                //long lsvi_number_of = lsvi->GetNumberOf(); 

                //
                // the variation of that we are permutating about is to be the last
                // in that variation
                permutations[about_variation_index] = index_maximums[about_variation_index] - 1;
                //  WAS:
                //  permutations[about_variation_index] = about_variation->GetNumberOf() - 1;

                //
                // the most significant variation index starts off at the least
                // significant end of the scale
                long most_significant_variation_index = variation_index_max;

                // The middle loop, for iterating through the array of variation
                // indices, "counting" the permutations of variations.
                while (!finished_this_middle)
                {
                    long next_variation_index = MakeVariationAttributes_InnerLoop(variation_index_max, about_variation_index, most_significant_variation_index, permutations, index_maximums);

                    // If the next variation is out of bounds, we're through here
                    if (next_variation_index < 0)
                    {
                        finished_this_middle = true;
                        break;
                    }


                    DoMakeTheSharedAttribute(owner, permutations);

                    permutations[variation_index_max] += 1;
                }

                finished_this_family = true;
            }
        }
    }


    void SharedAttributeFamily::DoMakeTheSharedAttribute
    (SharedAttributeOwner* owner,
        SharedAttributeVariationListIndices& permutation)
    {
        VariationNameArray variationNames;
        std::string attribute_name("");

        if (!permutation.empty())
        {
            attribute_name = MakeSharedAttributeName(myVariations, permutation, myCharacteristicDescriptor->Name, variationNames);
        }
        else
        {
            attribute_name = myCharacteristicDescriptor->Name;
        }

        if (false)
        {
            std::ostringstream permutation_str;
            for (SharedAttributeVariationListIndices::iterator piter(permutation.begin()); piter != permutation.end(); ++piter)
            {
                if (piter != permutation.begin())
                {
                    permutation_str << ",";
                }
                permutation_str << (*piter);
            }

            LOG_DEBUG << LOG_LINE << "DoMakeTheSharedAttribute {AttributeName:" << attribute_name
                << ", Permutations:[" << permutation_str.str() << "]}";
        }

        // get a new CharacteristicID for the SharedAttribute permutation                
        //CharacteristicIdentifier new_id = IdentifierUtility::instance().useNextCharacteristicIdentifier(myCharacteristicDescriptor->GetScoreboardStratum());

        if (0 <= myDefaultVariationPermutationIndex)
        {
            myCharacteristicDescriptor->Default = myDefaultValues[permutation[myDefaultVariationPermutationIndex]];
        }

        // 
        // Copy-ish construct a new CharacteristicDescriptor 
        CharacteristicDescriptor* tmp_cd = new CharacteristicDescriptor
        (*myCharacteristicDescriptor,
            IdentifierUtility::instance().useNextCharacteristicIdentifier(myCharacteristicDescriptor->GetScoreboardStratum()),
            -1, // ScoreboardIndex
            attribute_name);


        // MSA 11.01.20 per-Plant Variations aren't used as variants when getting values from SharedAttributes, right?

        long int variant1max = -1; // Flag in case of improper config
        long int variant2max = -1;
        for (SharedAttributeVariationList::const_iterator iter = myVariations.begin(); iter != myVariations.end(); ++iter)
        {
            if ((*iter)->GetName() != "Plant")
            {
                // Remember we use zero-based counts. RootOrder 0, "no VolumeObject" (i.e. 0), etc.

                if (variant1max == -1)
                {
                    variant1max = (*iter)->GetNumberOf() - 1;
                }
                else if (variant2max == -1)
                {
                    variant2max = (*iter)->GetNumberOf() - 1;
                }
            }
        }

        // MSA 11.01.15	This is a little kludgy, but we need to ensure that variant 2 is usable, even if there are no variations of it.
        // This is because we may have a simulation running on this codebase (i.e. with barrier modelling functionality)
        // which does not wish to use barrier modelling. In this case, a single valid variant 2 is required to define a single 
        // spatial subsection per Box.
        if (variant2max < 0) variant2max = 0;

        //
        // Create the darned thing, finally
        SharedAttribute* sa = mySupplier->CreateSharedAttribute(tmp_cd, variant1max, variant2max);

        owner->RegisterSharedAttribute(sa, mySupplier, variationNames, myVariations);
    }


    void SharedAttributeFamily::MakeAllAttributes(SharedAttributeOwner* owner)
    {
        // 
        // 
        //
        if (!myVariations.empty())
        {
            // Check if any variation still has nothing to vary (ie. GetNumberOf()
            // returns 0 or less.  If that is the case, then there will be no valid
            // permutations, and creation of attributes is unnecessary.
            //
            // If any of the variations still have nothing to vary,
            // don't bother trying to make anything.
            for (unsigned long variation_chk_idx = 0;
                variation_chk_idx < myVariations.size();
                ++variation_chk_idx)
            {
                if (myVariations[variation_chk_idx]->GetNumberOf() <= 0)
                {
                    return;
                }
            }

            MakeAllAttributesVaried(owner);
        }
        else
        {
            SharedAttributeVariationListIndices empty_permutations;
            DoMakeTheSharedAttribute(owner, empty_permutations);
        }
    }


    void SharedAttributeFamily::MakeAllAttributesVaried(SharedAttributeOwner* owner)
    {
        //
        // set up a vector of the variations' current indices.
        const long num_variations = myVariations.size();
        const long variation_index_max = num_variations - 1;
        bool finished_this_middle = false;

        //
        //  Set up and initialise (to zero) the list of indices in
        //  the current permutations. The list will have as
        SharedAttributeVariationListIndices permutations(num_variations, 0);

        //
        // Get the Least Significant VariationInterface		// MSA 09.10.07 Unused
        //SharedAttributeVariation * lsvi = myVariations[variation_index_max];
        // long lsvi_number_of = lsvi->GetNumberOf(); 

        //
        // the most significant variation index starts of at the least
        // significant end of the scale
        long most_significant_variation_index = variation_index_max;

        // The middle loop, for iterating through the array of variation
        // indices, "counting" the permutations of variations.
        while (!finished_this_middle)
        {
            long variation_index = variation_index_max;

            //VariationInterface * tmp_vi = myVariations[variation_index];
            //long tmp_number_of = tmp_vi->GetNumberOf();
            //const char * n = tmp_vi->GetName().c_str();
            //const char * s = tmp_vi->GetString(permutations[most_significant_variation_index]).c_str();


            // is the index > max ?
            while ((variation_index >= 0) &&
                (permutations[variation_index] >= ((myVariations[variation_index])->GetNumberOf()))
                )
            {
                // yes !  Reset the index
                permutations[variation_index] = 0;

                // use the next-most significant variation
                variation_index--;

                if (variation_index >= 0)
                {
                    // increment the permutations's index of that variation
                    permutations[variation_index] += 1;

                    // track the most Significant Variation that we've
                    // covered the permutations of so far.  This is not
                    // currently used.
                    if (variation_index < most_significant_variation_index)
                    {
                        most_significant_variation_index = variation_index;
                    }
                }
            }

            // If the next variation is out of bounds, we're through here
            if (variation_index < 0)
            {
                finished_this_middle = true;
                break;
            }

            DoMakeTheSharedAttribute(owner, permutations);

            permutations[variation_index_max] += 1;
        }
    }


    void SharedAttributeFamily::MakeSearchString
    (SharedAttributeVariation* about_variation,
        const std::string& about_variation_string,
        std::string& search_string
    )
    {
        LOG_DEBUG << "MakeSearchString('" << about_variation->GetName().c_str() << "','" << about_variation_string.c_str() << "','" << search_string.c_str() << "')";

        wxString tmp_str = GetCharacteristicDescriptor()->Name;

        const long variation_index_max = myVariations.size() - 1;
        for (long tmp_variation_index = 0;
            tmp_variation_index <= variation_index_max;
            ++tmp_variation_index)
        {
            tmp_str += " ";

            SharedAttributeVariation* the_variation = myVariations[tmp_variation_index];

            if (the_variation->isEqual(*about_variation))
            {
                tmp_str.append(about_variation_string);
            }
            else
            {
                std::string erm;
                the_variation->GetVariationString(0, erm);
                tmp_str.append(erm);
            }
        }

        search_string = tmp_str.Trim();
    }

    std::string SharedAttributeFamily::MakeSharedAttributeName0() const
    {
        LOG_DEBUG << "MakeSharedAttributeName0()";

        std::string tmp_str = GetCharacteristicDescriptor()->Name;

        for (SharedAttributeVariationList::const_iterator iter = myVariations.begin();
            iter != myVariations.end(); ++iter)
        {
            tmp_str.append(1, ' '); //tmp_str += " ";

            const SharedAttributeVariation* the_variation = (*iter);

            SharedAttributeVariation* var = const_cast<SharedAttributeVariation *>(the_variation);

            var->AppendVariationString(0, tmp_str);
        }

        return tmp_str;
    }

    //
    // Function:
    //  SaveToFile
    //
    // Working Notes:
    //  Here, we only need to save as much info as is required to restore the class.
    //  We have good hints from the original xml class as to how much that is:
    //  1) CharacteristicDescriptor
    //  2) Owner Name
    //  3) Supplier Name
    //  4) Variations
    //  5) Initial Values
    //
    //
    //  RvH 2008-01-16 Pre-amble update
    //  WHAT THE HECK.  Item 5 is completely unnecessary. "Initial" values are only
    //  required in the human-readable xml. Here, we save "Current" values (which
    //  admittedly are "Initial" values for the restored state.
    //
    //
    //  
    //  The last of these is by far the trickiest.  How, in fact, do we save initial
    //  values ?  Each SharedAttribute, that is, a permutations of the variations in
    //  a family, can have a different "initial" (ie. default) value.  So we can do
    //  one or more of the following:
    //
    //  1.  Save the Family and its initial values seperately
    //  2.  Save each SharedAttribute seperately.  We would also need to save the
    //      family, since any further creation of Plant/PlantType would require
    //      further SharedAttribute creation.
    //
    //  Well i *did* think of a third alternative, but having documented the first
    //  two i find it pretty plain that the first is the most sensible.  For a start,
    //  that method is how the Values are read from the xml files.
    //
    //  One intricacy is the order in which stuff is read in from xml, and the order
    //  that they'd be read in if they are all saved here and now.
    //
    //  In the former case, the SharedAttributeFamily objects are created, then all
    //  Plants and PlantTypes, then all the default values, then the scoreboard data.
    //
    //  In the latter case, the SharedAttributeFamily objects are created, then all
    //  the default/initial values, then Plants and PlantTypes, the scoreboard
    //  defaults.  The problem, as you can see, is that the initial values would
    //  normally need to be read after the Plants and PlantTypes are created.
    //
    //  The two solutions that come immediately to mind are:
    //   i] break the saving of SharedAttributes into two - the families then their
    //      initial values, the latter being saved AFTER the PlantCoordinator saves its
    //  


    SharedAttributeFamily::SharedAttributeFamily
    (CharacteristicDescriptor* cd,
        SharedAttributeSupplier* supplier,
        const SharedAttributeVariationList& variations,
        SharedAttributeVariation* default_variation,
        const std::vector<double>& defaults_values)
        : myVariations(variations)
        , myCharacteristicDescriptor(cd)
        , mySupplier(supplier)
        , myDefaultVariation(default_variation)
        , myDefaultVariationPermutationIndex(-1)
        , myDefaultValues(defaults_values)
    {
        RootMapLoggerInitialisation("rootmap.SharedAttributeFamily");

        VariationIndexType vit = 0;
        for (SharedAttributeVariationList::iterator savit(myVariations.begin());
            savit != myVariations.end(); ++savit, ++vit)
        {
            if ((*savit) == default_variation)
            {
                myDefaultVariationPermutationIndex = vit;
            }
        }
    }


    SharedAttributeFamily::~SharedAttributeFamily()
    {
        delete myCharacteristicDescriptor;
    }
} /* namespace rootmap */


