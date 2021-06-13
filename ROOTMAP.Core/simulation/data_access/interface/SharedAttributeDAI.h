#ifndef SharedAttributeDAI_H
#define SharedAttributeDAI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        SharedAttributeDAI.h
// Purpose:     Declaration of the SharedAttributeDAI class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-11-03 03:19:37 +0900 (Mon, 03 Nov 2008) $
// $Revision: 22 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/CharacteristicDAI.h"

namespace rootmap
{
    class SharedAttributeDAI : public virtual CharacteristicDAI
    {
    public:
        virtual const std::string& getDefaultsName() const = 0;
        virtual const std::vector<double>& getDefaultsValueArray() const = 0;

        virtual ~SharedAttributeDAI()
        {
        }

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
    protected:
        SharedAttributeDAI()
        {
        }
    }; // class SharedAttributeDAI
} /* namespace rootmap */

#endif // #ifndef SharedAttributeDAI_H
