/////////////////////////////////////////////////////////////////////////////
// Name:        DirectoryData.cpp
// Purpose:     Implementation of the DirectoryData class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_persistence/common/DirectoryData.h"


namespace rootmap
{
    DirectoryData::DirectoryData(const char* name, const char* location)
        : myName(name)
        , myLocation(location)
    {
    }

    DirectoryData::DirectoryData(const std::string& name, const std::string& location)
        : myName(name)
        , myLocation(location)
    {
    }

    DirectoryData::~DirectoryData()
    {
    }
} /* namespace rootmap */

