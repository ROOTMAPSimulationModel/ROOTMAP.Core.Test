/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessAttributeSupplier.cpp
// Purpose:     Implementation of the ClassTemplate class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/shared_attributes/ProcessAttributeSupplier.h"
#include "simulation/process/shared_attributes/ProcessSharedAttribute.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"


namespace rootmap
{
    RootMapLoggerDefinition(ProcessAttributeSupplier);

    SharedAttribute* ProcessAttributeSupplier::CreateSharedAttribute
    (CharacteristicDescriptor* cd,
        long int variant1Max,
        long int variant2Max
    )
    {
        //
        // Register with the Process first, so that when the attribute is
        // created it can grab the AttributeIndex of the CharacteristicDescriptor
        cd->ScoreboardIndex = GetNewAttributeIndex();

        //
        //  Create that SharedAttribute, yeah.
        SharedAttribute* newattribute = new ProcessSharedAttribute
        (cd,
            true,
            this,
            variant1Max,
            variant2Max
        );

        LOG_DEBUG << "Created ProcessSharedAttribute {Name:" << cd->Name
            << "} {SupplierIndex:" << cd->ScoreboardIndex << "}";
        //
        // allocate storage for it.  Easy!! Just add its default value to the vector.
        myAttributeValues.push_back(cd->Default);

        //
        // add it to the list of our SharedAttributes


        return newattribute;
    }

    //
    // Function:
    //  GetNewAttributeIndex
    //
    // Description:
    //  Used by CreateSharedAttribute to allocate a new index to the
    //  ScoreboardIndex field of the CharacteristicDescriptor.
    //
    // Working Notes:
    //  Could have relied on side effects of the ++ postincrement operator,
    //  but thought i'd go for something a *little* more readable.
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    long ProcessAttributeSupplier::GetNewAttributeIndex()
    {
        long tmp_idx = myNextUnusedAttributeIndex;
        ++myNextUnusedAttributeIndex;
        return (tmp_idx);
    }

    ProcessAttributeSupplier::ProcessAttributeSupplier(Process* p, const char* name)
        : mySupplierName(name)
        , myProcess(p)
        , myNextUnusedAttributeIndex(0)
    {
        RootMapLoggerInitialisation("rootmap.ProcessAttributeSupplier");
        // UNFORTUNATELY, this class is usually constructed at the time of construction
        // of the Process class that this is a supplier for. That, in turn, means that
        // the pName of the base class has not yet been initialised (that is done during
        // one of the overloaded IProcess methods).  THUS, we can't get the name here
        // hence it is passed in to the constructor

        SharedAttributeRegistrar::RegisterSupplier(this);
    }


    ProcessAttributeSupplier::~ProcessAttributeSupplier()
    {
    }


    const std::string& ProcessAttributeSupplier::GetSharedAttributeSupplierName()
    {
        return mySupplierName;
    }
} /* namespace rootmap */


