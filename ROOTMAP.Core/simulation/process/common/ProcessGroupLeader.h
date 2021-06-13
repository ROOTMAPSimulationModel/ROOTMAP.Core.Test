#ifndef ProcessGroupLeader_H
#define ProcessGroupLeader_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessGroupLeader.h
// Purpose:     Declaration of the ProcessGroupLeader class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-11-18 12:05:38 +0900 (Tue, 18 Nov 2008) $
// $Revision: 30 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/Process.h"

namespace rootmap
{
    class ProcessGroupLeader
        : public Process
    {
    public:
        DECLARE_ABSTRACT_CLASS(ProcessGroupLeader)

        ProcessGroupLeader();
        virtual ~ProcessGroupLeader();

        virtual void Initialise(SimulationEngine& engine);

        virtual Process* FindProcessByCharacteristicID(long characteristic_id);
        virtual Process* FindProcessByCharacteristicName(const std::string& characteristic_name, long* characteristic_number);
        virtual Process* FindProcessByCharacteristicIndex(long int characteristic_index);
        virtual long int FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name);

        virtual Process* FindProcessByProcessID(ProcessIdentifier process_id) = 0;
        virtual Process* FindProcessByProcessName(const std::string& pname) = 0;
    private:

        ///
        /// member declaration
    }; // class ProcessGroupLeader
} /* namespace rootmap */

#endif // #ifndef ProcessGroupLeader_H
