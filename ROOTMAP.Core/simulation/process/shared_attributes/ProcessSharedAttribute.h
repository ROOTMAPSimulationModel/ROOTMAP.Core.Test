/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessSharedAttribute.h
// Purpose:     Declaration of the ProcessSharedAttribute class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef ProcessSharedAttribute_H
#define ProcessSharedAttribute_H
#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/common/CharacteristicDescriptor.h"
#include "core/common/RmAssert.h"


namespace rootmap
{
    //
    // ProcessSharedAttribute
    //
    class Process;

    /**
     * Derives from SharedAttribute to provide a 0-Dimensional attribute (ie. a single value)
     */
    class ProcessSharedAttribute : public SharedAttribute
    {
    public:
        ProcessSharedAttribute(CharacteristicDescriptor* cd,
            bool own,
            ProcessAttributeSupplier* supplier,
            const long& variant1max,
            const long& variant2max);
    private:
        //
        // Static (ie. non-member).  Registered with the base class
        //
        // GetValue
        static double sGetValue0(SharedAttribute* instance);
        static double sGetValue1(const BoxIndex& box_index, SharedAttribute* instance);
        static double sGetValue2(const BoxIndex& box_index, long variant, SharedAttribute* instance);
        static double sGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance);

        //
        // SetValue
        static void sSetValue0(const double& value, SharedAttribute* instance);
        static void sSetValue1(const double& value, const BoxIndex& box_index, SharedAttribute* instance);
        static void sSetValue2(const double& value, const BoxIndex& box_index, long variant, SharedAttribute* instance);
        static void sSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance);

        //
        // Member functions
        //
        // GetValue
        double DoGetValue0();
        double DoGetValue1(const BoxIndex& box_index);
        double DoGetValue2(const BoxIndex& box_index, long variant);
        double DoGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2);

        //
        // SetValue
        void DoSetValue0(const double& value);
        void DoSetValue1(const double& value, const BoxIndex& box_index);
        void DoSetValue2(const double& value, const BoxIndex& box_index, long variant);
        void DoSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2);

        ProcessAttributeSupplier* myProcessAttributeSupplier;
        const long myVariant1Max;
        const long myVariant2Max;
        CharacteristicIndex myAttributeIndex;
    };


    inline ProcessSharedAttribute::ProcessSharedAttribute
    (CharacteristicDescriptor* cd,
        bool own,
        ProcessAttributeSupplier* plas,
        const long& variant1max,
        const long& variant2max)
        : SharedAttribute(cd, own)
        , myProcessAttributeSupplier(plas)
        , myVariant1Max(variant1max)
        , myVariant2Max(variant2max)
    {
        RegisterGetValueFunction0(sGetValue0);
        RegisterGetValueFunction1(sGetValue1);
        RegisterGetValueFunction2(sGetValue2);
        RegisterGetValueFunction3(sGetValue3);
        RegisterSetValueFunction0(sSetValue0);
        RegisterSetValueFunction1(sSetValue1);
        RegisterSetValueFunction2(sSetValue2);
        RegisterSetValueFunction3(sSetValue3);

        myAttributeIndex = cd->ScoreboardIndex;
    }

    inline double ProcessSharedAttribute::sGetValue0(SharedAttribute* instance)
    {
        return ((ProcessSharedAttribute *)instance)->DoGetValue0();
    }

    inline double ProcessSharedAttribute::sGetValue1(const BoxIndex& box_index, SharedAttribute* instance)
    {
        return ((ProcessSharedAttribute *)instance)->DoGetValue1(box_index);
    }

    inline double ProcessSharedAttribute::sGetValue2(const BoxIndex& box_index, long variant, SharedAttribute* instance)
    {
        return ((ProcessSharedAttribute *)instance)->DoGetValue2(box_index, variant);
    }

    inline double ProcessSharedAttribute::sGetValue3(const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance)
    {
        return ((ProcessSharedAttribute *)instance)->DoGetValue3(box_index, variant1, variant2);
    }


    inline void ProcessSharedAttribute::sSetValue0(const double& value, SharedAttribute* instance)
    {
        ((ProcessSharedAttribute *)instance)->DoSetValue0(value);
    }

    inline void ProcessSharedAttribute::sSetValue1(const double& value, const BoxIndex& box_index, SharedAttribute* instance)
    {
        ((ProcessSharedAttribute *)instance)->DoSetValue1(value, box_index);
    }

    inline void ProcessSharedAttribute::sSetValue2(const double& value, const BoxIndex& box_index, long variant, SharedAttribute* instance)
    {
        ((ProcessSharedAttribute *)instance)->DoSetValue2(value, box_index, variant);
    }

    inline void ProcessSharedAttribute::sSetValue3(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance)
    {
        ((ProcessSharedAttribute *)instance)->DoSetValue3(value, box_index, variant1, variant2);
    }


    inline double ProcessSharedAttribute::DoGetValue0()
    {
        return (myProcessAttributeSupplier->GetAttributeValue(myAttributeIndex));
    }

    inline double ProcessSharedAttribute::DoGetValue1(const BoxIndex& /* box_index */)
    {
        return (myProcessAttributeSupplier->GetAttributeValue(myAttributeIndex));
    }

    inline double ProcessSharedAttribute::DoGetValue2(const BoxIndex& /* box_index */, long variant)
    {
        if (variant > myVariant1Max) variant = myVariant1Max;

        const long idxOffset = myVariant2Max < 0 ? variant : (variant * (myVariant2Max + 1));

        RmAssert(idxOffset >= 0, "Variant out of range");

        return myProcessAttributeSupplier->GetAttributeValue(myAttributeIndex + idxOffset);
    }

    inline double ProcessSharedAttribute::DoGetValue3(const BoxIndex& /* box_index */, const long& variant1, const long& variant2)
    {
        RmAssert(variant1 <= myVariant1Max, "Variant 1 out of range");
        RmAssert(variant2 <= myVariant2Max, "Variant 2 out of range");

        long idx = myAttributeIndex + variant1 * (myVariant2Max + 1) + variant2;

        return (myProcessAttributeSupplier->GetAttributeValue(idx));
    }

    inline void ProcessSharedAttribute::DoSetValue0(const double& value)
    {
        myProcessAttributeSupplier->SetAttributeValue(myAttributeIndex, value);
    }

    inline void ProcessSharedAttribute::DoSetValue1(const double& value, const BoxIndex& /* box_index */)
    {
        myProcessAttributeSupplier->SetAttributeValue(myAttributeIndex, value);
    }

    inline void ProcessSharedAttribute::DoSetValue2(const double& value, const BoxIndex& /* box_index */, long variant)
    {
        if (variant > myVariant1Max) variant = myVariant1Max;

        const long idxOffset = myVariant2Max < 0 ? variant : (variant * (myVariant2Max + 1));

        RmAssert(idxOffset >= 0, "Variant out of range");

        myProcessAttributeSupplier->SetAttributeValue((myAttributeIndex + idxOffset), value);
    }

    inline void ProcessSharedAttribute::DoSetValue3(const double& value, const BoxIndex& /* box_index */, const long& variant1, const long& variant2)
    {
        RmAssert(variant1 <= myVariant1Max, "Variant 1 out of range");
        RmAssert(variant2 <= myVariant2Max, "Variant 2 out of range");

        long idx = myAttributeIndex + variant1 * (myVariant2Max + 1) + variant2;

        myProcessAttributeSupplier->SetAttributeValue(idx, value);
    }
} /* namespace rootmap */

#endif // #ifndef ProcessSharedAttribute_H
