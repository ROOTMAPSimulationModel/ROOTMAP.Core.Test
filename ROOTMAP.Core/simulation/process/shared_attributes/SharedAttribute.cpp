/*
    SharedAttribute.cpp

    Author:            Robert van Hugten
    Description:    <describe the SharedAttribute class here>
*/

#include "simulation/process/shared_attributes/SharedAttribute.h"
#include "simulation/process/common/CharacteristicDescriptor.h"


namespace rootmap
{
    void SharedAttribute::RegisterGetValueFunction0(SharedAttributeGetValueFunction0 f0)
    {
        myGetValueFunction0 = f0;
    }

    void SharedAttribute::RegisterGetValueFunction1(SharedAttributeGetValueFunction1 f1)
    {
        myGetValueFunction1 = f1;
    }

    void SharedAttribute::RegisterGetValueFunction2(SharedAttributeGetValueFunction2 f2)
    {
        myGetValueFunction2 = f2;
    }

    void SharedAttribute::RegisterGetValueFunction3(SharedAttributeGetValueFunction3 f3)
    {
        myGetValueFunction3 = f3;
    }


    void SharedAttribute::RegisterSetValueFunction0(SharedAttributeSetValueFunction0 f0)
    {
        mySetValueFunction0 = f0;
    }

    void SharedAttribute::RegisterSetValueFunction1(SharedAttributeSetValueFunction1 f1)
    {
        mySetValueFunction1 = f1;
    }

    void SharedAttribute::RegisterSetValueFunction2(SharedAttributeSetValueFunction2 f2)
    {
        mySetValueFunction2 = f2;
    }

    void SharedAttribute::RegisterSetValueFunction3(SharedAttributeSetValueFunction3 f3)
    {
        mySetValueFunction3 = f3;
    }


    //void SharedAttribute::RegisterGetConcentrationFunction0(SharedAttributeGetConcentrationFunction0 f0)
    //{
    //    myGetConcentrationFunction0 = f0;
    //}
    //void SharedAttribute::RegisterGetConcentrationFunction1(SharedAttributeGetConcentrationFunction1 f1)
    //{
    //    myGetConcentrationFunction1 = f1;
    //}
    //void SharedAttribute::RegisterGetConcentrationFunction2(SharedAttributeGetConcentrationFunction2 f2)
    //{
    //    myGetConcentrationFunction2 = f2;
    //}


    //void SharedAttribute::RegisterSetConcentrationFunction0(SharedAttributeSetConcentrationFunction0 f0)
    //{
    //    mySetConcentrationFunction0 = f0;
    //}
    //void SharedAttribute::RegisterSetConcentrationFunction1(SharedAttributeSetConcentrationFunction1 f1)
    //{
    //    mySetConcentrationFunction1 = f1;
    //}
    //void SharedAttribute::RegisterSetConcentrationFunction2(SharedAttributeSetConcentrationFunction2 f2)
    //{
    //    mySetConcentrationFunction2 = f2;
    //}

    SharedAttribute::SharedAttribute(CharacteristicDescriptor* cd, bool own)
        : myGetValueFunction0(0)
        , myGetValueFunction1(0)
        , myGetValueFunction2(0)
        , mySetValueFunction0(0)
        , mySetValueFunction1(0)
        , mySetValueFunction2(0)
        , myCharacteristicDescriptor(cd)
        , ownsDescriptor(own)
    {
    }


    SharedAttribute::~SharedAttribute()
    {
        //TODO: determine if it is safe to delete the CharacteristicDescriptor
        // 20030720 RvH all scoreboard characteristic descriptors are owned by the
        //          ProcessModuleStorage.
        // 20030721 RvH delete the descriptor if we own it.
        if (ownsDescriptor)
        {
            delete myCharacteristicDescriptor;
        }
    }

    std::ostream& operator<<(std::ostream& ostr, const SharedAttribute& attribute)
    {
        const CharacteristicDescriptor& cd = attribute.GetCharacteristicDescriptor();
        ostr << "{Characteristic {Name:" << cd.Name
            << "} {ID:" << cd.id
            << "} {Index:" << cd.ScoreboardIndex
            << "} }";

        return ostr;
    }
} /* namespace rootmap */


