/*
    SharedAttribute.h

    Author:         Robert van Hugten
    Description:    <describe the SharedAttribute class here>
*/

#ifndef ScoreboardSharedAttribute_H
#define ScoreboardSharedAttribute_H
#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/process/common/CharacteristicDescriptor.h"

namespace rootmap
{
    struct CharacteristicDescriptor;

    class Scoreboard;

    class ScoreboardSharedAttribute : public SharedAttribute
    {
    public:
        ScoreboardSharedAttribute(CharacteristicDescriptor* cd,
            bool own,
            Scoreboard* s,
            const long& variant_max,
            const long& variant_2_max);

    private:
        //
        // Static (ie. non-member).  Registered with the base class
        //
        // GetValue
        static double sGetValue1(const BoxIndex& box_index, SharedAttribute* instance);
        static double sGetValue2(const BoxIndex& box_index, long variant, SharedAttribute* instance);
        static double sGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance);

        //
        // SetValue
        static void sSetValue1(const double& value, const BoxIndex& box_index, SharedAttribute* instance);
        static void sSetValue2(const double& value, const BoxIndex& box_index, long variant, SharedAttribute* instance);
        static void sSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance);

        //    //
        //    // GetConcentration
        //    static double sGetConcentration1(const BoxIndex & box_index, SharedAttribute * instance);
        //    static double sGetConcentration2(const BoxIndex & box_index, const long & variant, SharedAttribute * instance);
        //
        //    //
        //    // SetConcentration
        //    static void sSetConcentration1(double value, const BoxIndex & box_index, SharedAttribute * instance);
        //    static void sSetConcentration2(double value, const BoxIndex & box_index, const long & variant, SharedAttribute * instance);


        //
        // Member functions
        //
        // GetValue
        double DoGetValue1(const BoxIndex& box_index);
        double DoGetValue2(const BoxIndex& box_index, long variant);
        double DoGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2);

        //
        // SetValue
        void DoSetValue1(const double& value, const BoxIndex& box_index);
        void DoSetValue2(const double& value, const BoxIndex& box_index, long variant);
        void DoSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2);

        //    //
        //    // GetConcentration
        //    double DoGetConcentration1(const BoxIndex & box_index);
        //    double DoGetConcentration2(const BoxIndex & box_index, const long & variant);
        //    
        //    //
        //    // SetConcentration
        //    void DoSetConcentration1(double value, const BoxIndex & box_index);
        //    void DoSetConcentration2(double value, const BoxIndex & box_index, const long & variant);


        Scoreboard* myScoreboard;
        CharacteristicIndex myScoreboardIndex;

        ///
        /// myVariant1Max represents the maximum *value* of variant of this
        /// shared attribute, not the maximum *number* of variations. For example,
        /// for branch order this would be 3 where the max number of branch order
        /// variations is 4.
        const long myVariant1Max; // MSA 11.01.19 This is essentially "myVariant1Max"

        const long myVariant2Max;
    };


    inline ScoreboardSharedAttribute::ScoreboardSharedAttribute
    (CharacteristicDescriptor* cd,
        bool own,
        Scoreboard* s,
        const long& variant_max,
        const long& variant_2_max)
        : SharedAttribute(cd, own)
        , myScoreboard(s)
        , myVariant1Max(variant_max)
        , myVariant2Max(variant_2_max)
    {
        RegisterGetValueFunction1(sGetValue1);
        RegisterGetValueFunction2(sGetValue2);
        RegisterGetValueFunction3(sGetValue3);
        RegisterSetValueFunction1(sSetValue1);
        RegisterSetValueFunction2(sSetValue2);
        RegisterSetValueFunction3(sSetValue3);
        //    RegisterGetConcentrationFunction1(sGetConcentration1);
        //    RegisterGetConcentrationFunction2(sGetConcentration2);
        //    RegisterSetConcentrationFunction1(sSetConcentration1);
        //    RegisterSetConcentrationFunction2(sSetConcentration2);

        myScoreboardIndex = cd->ScoreboardIndex;
    }

    inline double ScoreboardSharedAttribute::sGetValue1(const BoxIndex& box_index, SharedAttribute* instance)
    {
        return ((ScoreboardSharedAttribute *)instance)->DoGetValue1(box_index);
    }

    inline double ScoreboardSharedAttribute::sGetValue2(const BoxIndex& box_index, long variant, SharedAttribute* instance)
    {
        return ((ScoreboardSharedAttribute *)instance)->DoGetValue2(box_index, variant);
    }

    inline double ScoreboardSharedAttribute::sGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance)
    {
        return ((ScoreboardSharedAttribute *)instance)->DoGetValue3(box_index, variant1, variant2);
    }

    inline void ScoreboardSharedAttribute::sSetValue1(const double& value, const BoxIndex& box_index, SharedAttribute* instance)
    {
        ((ScoreboardSharedAttribute *)instance)->DoSetValue1(value, box_index);
    }

    inline void ScoreboardSharedAttribute::sSetValue2(const double& value, const BoxIndex& box_index, long variant, SharedAttribute* instance)
    {
        ((ScoreboardSharedAttribute *)instance)->DoSetValue2(value, box_index, variant);
    }

    inline void ScoreboardSharedAttribute::sSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance)
    {
        ((ScoreboardSharedAttribute *)instance)->DoSetValue3(value, box_index, variant1, variant2);
    }

    //inline double ScoreboardSharedAttribute::sGetConcentration1(const BoxIndex & box_index, SharedAttribute * instance)
    //{
    //    return ((ScoreboardSharedAttribute *)instance)->DoGetConcentration1(box_index);
    //}
    //inline double ScoreboardSharedAttribute::sGetConcentration2(const BoxIndex & box_index, const long & variant, SharedAttribute * instance)
    //{
    //    return ((ScoreboardSharedAttribute *)instance)->DoGetConcentration2(box_index, variant);
    //}
    //
    //inline void ScoreboardSharedAttribute::sSetConcentration1(double value, const BoxIndex & box_index, SharedAttribute * instance)
    //{
    //    ((ScoreboardSharedAttribute *)instance)->DoSetConcentration1(value, box_index);
    //}
    //inline void ScoreboardSharedAttribute::sSetConcentration2(double value, const BoxIndex & box_index, const long & variant, SharedAttribute * instance)
    //{
    //    ((ScoreboardSharedAttribute *)instance)->DoSetConcentration2(value, box_index, variant);
    //}

    //
    // this function should do whatever necessary with whatever data members
    // are at its disposal, to accomplish the accessing of the value.
    //
    // for instance:
    inline double ScoreboardSharedAttribute::DoGetValue1(const BoxIndex& box_index)
    {
        return (myScoreboard->GetCharacteristicValue(myScoreboardIndex, box_index));
    }

    //
    // this function should do whatever necessary with whatever data members
    // are at its disposal, to accomplish the accessing of the value.
    //
    // for instance:

    // in the case of a plant related scoreboard attribute, the variant is
    // be synonymous with "root order".  In which case we make sure it is
    // 3 or less, then add that to the ScoreboardIndex of the CharacteristicDescriptor,
    // The SharedAttribute relating to root order zero should have been
    // assigned to the relevant plant.
    //
    // To make this a generic scoreboard routine, what we've done is added a
    // data member "myVariant1Max".  That's because i'm guessing that any
    // process that wants to have a shared attribute that wants to use the
    // variant to access something from the scoreboard will probably want to
    // use the variant to vary the ScoreboardIndex, in turn because the
    // characteristic has been formed per-something, which the variant
    // represents.
    inline double ScoreboardSharedAttribute::DoGetValue2(const BoxIndex& box_index, long variant)
    {
        if (variant > myVariant1Max) variant = myVariant1Max;

        const long idxOffset = myVariant2Max < 0 ? variant : (variant * (myVariant2Max + 1));

        RmAssert(idxOffset >= 0, "Variant out of range");

        return (myScoreboard->GetCharacteristicValue((myScoreboardIndex + idxOffset), box_index));
    }

    inline double ScoreboardSharedAttribute::DoGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2)
    {
        RmAssert(variant1 <= myVariant1Max, "Variant 1 out of range");
        RmAssert(variant2 <= myVariant2Max, "Variant 2 out of range");

        long idx = myScoreboardIndex + variant1 * (myVariant2Max + 1) + variant2;

        return (myScoreboard->GetCharacteristicValue(idx, box_index));
    }

    inline void ScoreboardSharedAttribute::DoSetValue1(const double& value, const BoxIndex& box_index)
    {
        myScoreboard->SetCharacteristicValue(myScoreboardIndex, value, box_index);
    }

    inline void ScoreboardSharedAttribute::DoSetValue2(const double& value, const BoxIndex& box_index, long variant)
    {
        if (variant > myVariant1Max) variant = myVariant1Max;

        const long idxOffset = myVariant2Max < 0 ? variant : (variant * (myVariant2Max + 1));

        RmAssert(idxOffset >= 0, "Variant out of range");

        myScoreboard->SetCharacteristicValue((myScoreboardIndex + idxOffset), value, box_index);
    }

    inline void ScoreboardSharedAttribute::DoSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2)
    {
        RmAssert(variant1 <= myVariant1Max, "Variant 1 out of range");
        RmAssert(variant2 <= myVariant2Max, "Variant 2 out of range");

        long idx = myScoreboardIndex + variant1 * (myVariant2Max + 1) + variant2;

        myScoreboard->SetCharacteristicValue(idx, value, box_index);
    }
} /* namespace rootmap */

#endif // #ifndef ScoreboardSharedAttribute_H
