/////////////////////////////////////////////////////////////////////////////
// Name:        NameLocationData.cpp
// Purpose:     Implementation of the NameLocationData class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/NameLocationData.h"


namespace rootmap
{
    NameLocationData::NameLocationData(const char* name, const char* location)
        : myName(name)
        , myLocation(location)
    {
    }

    NameLocationData::NameLocationData(const std::string& name, const std::string& location)
        : myName(name)
        , myLocation(location)
    {
    }

    NameLocationData::~NameLocationData()
    {
    }
} /* namespace rootmap */

