#ifndef SharedAttribute_H
#define SharedAttribute_H
//
// Filename:
//  SharedAttribute.h
//
// Author:
//  Robert van Hugten
//
// Description:
//  A SharedAttribute is one that is possible to be shared (used) by more than
//  one class, and more than one process. A SharedAttribute may be owned by,
//  or defined in (which in this case means "stored in") any one of the classes
//  that share it.
//
//  A SharedAttribute can be used to make one or more Characteristics,
//  depending on how it varies, ie. what "variations" it is defined by.  A
//  SharedAttribute's variations are 
//
//  Subclasses of this base class implement storage for each class that can own
//  a SharedAttribute.
//
//  A SharedAttributeVariation is used to decide how many SharedAttribute
//  instances to create, however once created a SharedAttribute itself does
//  not need to know how many variations of itself there are.
//
//  The SharedAttribute class is intended as a facilitator/representative for
//  Characteristics to be used at any level, such as scoreboard or process.
//  As such, it should only contain enough information for that purpose.
//
//
// Working Notes:
//  The destructor is NOT virtual, so don't allocate anything in derived
//  classes, or become owner of anything you'll need to delete.  This is quite
//  a reasonable requirement, as the SharedAttributeSupplier should be doing
//  all the allocation and the SharedAttributeOwner should be doing all the
//  owning.
//
// Modifications:
// 20030401 RvH In order to support iterating, we need to support adding
//          SharedAttributes to lists key'd by 'VariationString'. For that,
//          we need to add more information to the SharedAttribute.
// 20030720 RvH made destructor public, so that attributes can be delete'd. Doh!
//
#include "simulation/process/shared_attributes/SharedAttributeCommon.h"
#include "core/scoreboard/ScoreboardBox.h"

namespace rootmap
{
    struct CharacteristicDescriptor;

    class SharedAttribute
    {
    public:

        //
        // There are three functions which can be used to access the value of an
        // attribute.  Which is used in the code for accessing any particular attribute
        // depends partly on the implementation of the owner class, although using
        // the second form is recommended.  The BoxIndex can be given as
        // "InvalidBoxIndex" (declared in "ScoreboardBox.h")
        //
        // The meaning of the "variant" is dependent on the SharedAttributeVariation
        // used to construct the instance of the SharedAttribute.  At the moment, the
        // only meaningful variant is the RootOrder.
        // MSA 11.01.19 There is a new meaningful variant: VolumeObject.
        // Adding another overload of GetValue(), SetValue() etc. to support this.
        double GetValue();
        double GetValue(const BoxIndex& box_index);
        double GetValue(const BoxIndex& box_index, long variant);
        double GetValue(const BoxIndex& box_index, const long& variant1, const long& variant2);


        // 
        void SetValue(const double& value);
        void SetValue(const double& value, const BoxIndex& box_index);
        void SetValue(const double& value, const BoxIndex& box_index, long variant);
        void SetValue(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2);

        //    //
        //    // 
        //    double GetConcentration();
        //    double GetConcentration(BoxIndex box_index);
        //    double GetConcentration(BoxIndex box_index, const long & variant);
        //
        //    // 
        //    void SetConcentration(double value);
        //    void SetConcentration(double value, BoxIndex box_index);
        //    void SetConcentration(double value, BoxIndex box_index, const long & variant);

        CharacteristicDescriptor* GetCharacteristicDescriptor();

        const CharacteristicDescriptor& GetCharacteristicDescriptor() const;

        //TODO this would be really freaking useful in the IteratorAssistant
        //const VariationNameArray & GetVariationNames() const;

        ~SharedAttribute();

        //
        // IMPLEMENTATION DETAILS
    protected:
        //
        // GetValue
        typedef double(*SharedAttributeGetValueFunction0)(SharedAttribute* instance);
        typedef double(*SharedAttributeGetValueFunction1)(const BoxIndex& box_index, SharedAttribute* instance);
        typedef double(*SharedAttributeGetValueFunction2)(const BoxIndex& box_index, long variant, SharedAttribute* instance);
        typedef double(*SharedAttributeGetValueFunction3)(const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance);

        void RegisterGetValueFunction0(SharedAttributeGetValueFunction0 f0);
        void RegisterGetValueFunction1(SharedAttributeGetValueFunction1 f1);
        void RegisterGetValueFunction2(SharedAttributeGetValueFunction2 f2);
        void RegisterGetValueFunction3(SharedAttributeGetValueFunction3 f3);


        //
        // SetValue
        typedef void(*SharedAttributeSetValueFunction0)(const double& value, SharedAttribute* instance);
        typedef void(*SharedAttributeSetValueFunction1)(const double& value, const BoxIndex& box_index, SharedAttribute* instance);
        typedef void(*SharedAttributeSetValueFunction2)(const double& value, const BoxIndex& box_index, long variant, SharedAttribute* instance);
        typedef void(*SharedAttributeSetValueFunction3)(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2, SharedAttribute* instance);

        void RegisterSetValueFunction0(SharedAttributeSetValueFunction0 f0);
        void RegisterSetValueFunction1(SharedAttributeSetValueFunction1 f1);
        void RegisterSetValueFunction2(SharedAttributeSetValueFunction2 f2);
        void RegisterSetValueFunction3(SharedAttributeSetValueFunction3 f3);


        //    //
        //    // GetConcentration
        //    typedef double (*SharedAttributeGetConcentrationFunction0)(SharedAttribute * instance);
        //    typedef double (*SharedAttributeGetConcentrationFunction1)(BoxIndex box_index, SharedAttribute * instance);
        //    typedef double (*SharedAttributeGetConcentrationFunction2)(BoxIndex box_index, const long & variant, SharedAttribute * instance);
        //
        //    void RegisterGetConcentrationFunction0(SharedAttributeGetConcentrationFunction0 f0);
        //    void RegisterGetConcentrationFunction1(SharedAttributeGetConcentrationFunction1 f1);
        //    void RegisterGetConcentrationFunction2(SharedAttributeGetConcentrationFunction2 f2);
        //
        //
        //    //
        //    // SetConcentration
        //    typedef void (*SharedAttributeSetConcentrationFunction0)(double value, SharedAttribute * instance);
        //    typedef void (*SharedAttributeSetConcentrationFunction1)(double value, BoxIndex box_index, SharedAttribute * instance);
        //    typedef void (*SharedAttributeSetConcentrationFunction2)(double value, BoxIndex box_index, const long & variant, SharedAttribute * instance);
        //
        //    void RegisterSetConcentrationFunction0(SharedAttributeSetConcentrationFunction0 f0);
        //    void RegisterSetConcentrationFunction1(SharedAttributeSetConcentrationFunction1 f1);
        //    void RegisterSetConcentrationFunction2(SharedAttributeSetConcentrationFunction2 f2);


        SharedAttribute(CharacteristicDescriptor* cd, bool own);

    private:

        CharacteristicDescriptor* myCharacteristicDescriptor;
        bool ownsDescriptor;

        SharedAttributeGetValueFunction0 myGetValueFunction0;
        SharedAttributeGetValueFunction1 myGetValueFunction1;
        SharedAttributeGetValueFunction2 myGetValueFunction2;
        SharedAttributeGetValueFunction3 myGetValueFunction3;

        SharedAttributeSetValueFunction0 mySetValueFunction0;
        SharedAttributeSetValueFunction1 mySetValueFunction1;
        SharedAttributeSetValueFunction2 mySetValueFunction2;
        SharedAttributeSetValueFunction3 mySetValueFunction3;

        //    SharedAttributeGetConcentrationFunction0 myGetConcentrationFunction0;
        //    SharedAttributeGetConcentrationFunction1 myGetConcentrationFunction1;
        //    SharedAttributeGetConcentrationFunction2 myGetConcentrationFunction2;
        //
        //    SharedAttributeSetConcentrationFunction0 mySetConcentrationFunction0;
        //    SharedAttributeSetConcentrationFunction1 mySetConcentrationFunction1;
        //    SharedAttributeSetConcentrationFunction2 mySetConcentrationFunction2;
    };

    std::ostream& operator<<(std::ostream& ostr, const SharedAttribute& attribute);

    //
    // GetValue
    inline double SharedAttribute::GetValue()
    {
        return (myGetValueFunction0(this));
    }

    inline double SharedAttribute::GetValue(const BoxIndex& box_index)
    {
        return (myGetValueFunction1(box_index, this));
    }

    inline double SharedAttribute::GetValue(const BoxIndex& box_index, long variant)
    {
        return (myGetValueFunction2(box_index, variant, this));
    }

    inline double SharedAttribute::GetValue(const BoxIndex& box_index, const long& variant1, const long& variant2)
    {
        return (myGetValueFunction3(box_index, variant1, variant2, this));
    }


    //
    // SetValue
    inline void SharedAttribute::SetValue(const double& value)
    {
        mySetValueFunction0(value, this);
    }

    inline void SharedAttribute::SetValue(const double& value, const BoxIndex& box_index)
    {
        mySetValueFunction1(value, box_index, this);
    }

    inline void SharedAttribute::SetValue(const double& value, const BoxIndex& box_index, long variant)
    {
        mySetValueFunction2(value, box_index, variant, this);
    }

    inline void SharedAttribute::SetValue(const double& value, const BoxIndex& box_index, const long& variant1, const long& variant2)
    {
        mySetValueFunction3(value, box_index, variant1, variant2, this);
    }


    ////
    //// GetConcentration
    //inline double SharedAttribute::GetConcentration()
    //{
    //    return (myGetConcentrationFunction0(this));
    //}
    //inline double SharedAttribute::GetConcentration(BoxIndex box_index)
    //{
    //    return (myGetConcentrationFunction1(box_index, this));
    //}
    //inline double SharedAttribute::GetConcentration(BoxIndex box_index, const long & variant)
    //{
    //    return (myGetConcentrationFunction2(box_index, variant, this));
    //}
    //
    //
    ////
    //// SetConcentration
    //inline void SharedAttribute::SetConcentration(double value)
    //{
    //    mySetConcentrationFunction0(value, this);
    //}
    //inline void SharedAttribute::SetConcentration(double value, BoxIndex box_index)
    //{
    //    mySetConcentrationFunction1(value, box_index, this);
    //}
    //inline void SharedAttribute::SetConcentration(double value, BoxIndex box_index, const long & variant)
    //{
    //    mySetConcentrationFunction2(value, box_index, variant, this);
    //}


    inline CharacteristicDescriptor* SharedAttribute::GetCharacteristicDescriptor()
    {
        return (myCharacteristicDescriptor);
    }

    inline const CharacteristicDescriptor& SharedAttribute::GetCharacteristicDescriptor() const
    {
        return (*myCharacteristicDescriptor);
    }
} /* namespace rootmap */

#endif // #ifndef SharedAttribute_H
