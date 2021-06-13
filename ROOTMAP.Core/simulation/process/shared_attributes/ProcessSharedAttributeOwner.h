/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessSharedAttributeOwner.h
// Purpose:     Declaration of the ProcessSharedAttributeOwner class
// Created:     25/10/2002
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef ProcessSharedAttributeOwner_H
#define ProcessSharedAttributeOwner_H

#include "simulation/process/shared_attributes/SharedAttributeOwner.h"
#include "core/log/Logger.h"
#include <string>

namespace rootmap
{
    class Process;

    /**
     * Provides a Process class with the ability to own SharedAttributes. Can own any
     * derived type, including ProcessSharedAttribute and ScoreboardSharedAttribute
     */
    class ProcessSharedAttributeOwner
        : public SharedAttributeOwner
    {
    public:
        ProcessSharedAttributeOwner(const char* name, Process* p);

        const std::string& GetSharedAttributeOwnerName() const;

        //
        // overridden to make sure AddCharacteristic() gets called
        void RegisterSharedAttribute(SharedAttribute* sa,
            SharedAttributeSupplier* supplier,
            const VariationNameArray& savKeyList,
            const SharedAttributeVariationList& savList
        );


    private:
        RootMapLoggerDeclaration();

        std::string mySharedAttributeOwnerName;

        Process* myProcess;
    };
} /* namespace rootmap */

#endif // #ifndef ProcessSharedAttributeOwner
