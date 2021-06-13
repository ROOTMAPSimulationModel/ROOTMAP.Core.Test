/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessGroupLeader.cpp
// Purpose:     Implementation of the ProcessGroupLeader class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-11-18 12:05:38 +0900 (Tue, 18 Nov 2008) $
// $Revision: 30 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/ProcessGroupLeader.h"
#include "simulation/process/common/ProcessExceptions.h"

namespace rootmap
{
    IMPLEMENT_ABSTRACT_CLASS(ProcessGroupLeader, Process);

    ProcessGroupLeader::ProcessGroupLeader()
    { }

    ProcessGroupLeader::~ProcessGroupLeader()
    { }

    void ProcessGroupLeader::Initialise(SimulationEngine& /* engine */)
    { }

    Process* ProcessGroupLeader::FindProcessByCharacteristicID(long /* characteristic_id */)
    {
        throw ProcessException("ProcessGroupLeader::FindProcessByCharacteristicID() default implementation reached", GetProcessName().c_str());
    }

    Process* ProcessGroupLeader::FindProcessByCharacteristicName(const std::string& /* characteristic_name */, long* /* characteristic_number */)
    {
        throw ProcessException("ProcessGroupLeader::FindProcessByCharacteristicName() default implementation reached", GetProcessName().c_str());
    }

    Process* ProcessGroupLeader::FindProcessByCharacteristicIndex(long int /* characteristic_index */)
    {
        throw ProcessException("ProcessGroupLeader::FindProcessByCharacteristicIndex() default implementation reached", GetProcessName().c_str());
    }

    long int ProcessGroupLeader::FindCharacteristicIndexByCharacteristicName(const std::string& /* characteristic_name */)
    {
        throw ProcessException("ProcessGroupLeader::FindCharacteristicIndexByCharacteristicName() default implementation reached", GetProcessName().c_str());
    }
} /* namespace rootmap */
