/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessCoordinator.cpp
// Purpose:     Implementation of the ProcessCoordinator class
// Created:     27/05/2006
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessGroupLeader.h"
#include "simulation/process/common/ProcessList.h"
#include "simulation/process/common/IProcessIterationUser.h"
#include "simulation/process/common/CharacteristicDescriptor.h"

#include "simulation/common/SimulationEngine.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"

#include "simulation/data_access/interface/ProcessDAI.h"
#include "simulation/data_access/common/DataAccessManager.h"

#include "core/common/RmAssert.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/utility/Utility.h"

#include "wx/object.h"

#include "simulation/process/raytrace/ScoreboardRenderer.h"

namespace rootmap
{
    ProcessCoordinator::ProcessCoordinator(SimulationEngine& engine, DataAccessManager& dam)
        : m_engine(engine)
        , m_volumeObjectCoordinator(__nullptr)
    {
        auto p = new ScoreboardRenderer; // Terrible hack to force inclusion of ScoreboardRenderer.
        dam.constructProcesses(this);
        delete p;
    }

    ProcessCoordinator::~ProcessCoordinator()
    {
        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            delete ((*iter).second);
        }

        // m_processListALL ByValue - cannot delete
        ProcessList::ProcessListType& procList = m_processListALL.getPrivateList();
        for (ProcessList::ProcessListType::iterator iter = procList.begin();
            iter != procList.end(); ++iter)
        {
            delete (*iter);
        }
    }


    void ProcessCoordinator::createProcess(const ProcessDAI& process_data)
    {
        Process* proc = MakeProcess(process_data);
        ProcessList* process_list = MakeProcessList(process_data.getStratum());

        process_list->AddTo(proc);
        m_processListALL.AddTo(proc);
    }

    //
    // Function:
    //  FindProcessByCharacteristicID
    //
    // Description:
    //  A unified approach to finding a process given either its ID or an ID of
    //  one of its characteristics.
    //
    // Working Notes:
    //  Used by TDirectors to know which process to change to when another
    //  characteristic is selected from the menu, since the characteristic
    //  have command#'s that match their characteristic id.
    //
    // Modifications:
    //  20020820 RvH - no more volume/stratum-dependent process lists
    Process* ProcessCoordinator::FindProcessByCharacteristicID(long characteristic_id)
    {
        Process* found_process = 0;
        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            found_process = FindProcessByCharacteristicID(characteristic_id, ((*iter).second));
            if (found_process != 0)
            {
                return (found_process);
            }
        }

        // MSA 11.02.10 Modifying this method to search m_processListALL if the process is not found in m_processLists.
        return FindProcessByCharacteristicID(characteristic_id, &m_processListALL);
    }


    Process* ProcessCoordinator::FindProcessByCharacteristicID(long characteristic_id, const ScoreboardStratum& stratum)
    {
        Process* found_process = 0;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_process = FindProcessByCharacteristicID(characteristic_id, ((*fnd).second));
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicID(long characteristic_id, ProcessList* proclist)
    {
        Process* found_process = 0;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            const CharacteristicDescriptor& char_desc = (*iter)->FindCharacteristicID(characteristic_id);
            if (char_desc.operator!=(CharacteristicDescriptor::Null))
                //if ( CharacteristicDescriptor::Null != char_desc )
            {
                return (*iter);
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicName(const std::string& characteristic_name, long* characteristic_number)
    {
        Process* found_process = 0;
        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            found_process = FindProcessByCharacteristicName(characteristic_name, ((*iter).second), characteristic_number);
            if (found_process != 0)
            {
                return (found_process);
            }
        }

        // MSA 11.02.10 Modifying this method to search m_processListALL if the process is not found in m_processLists.
        return FindProcessByCharacteristicName(characteristic_name, &m_processListALL, characteristic_number);
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicName(const std::string& characteristic_name, const ScoreboardStratum& stratum, long* characteristic_number)
    {
        Process* found_process = 0;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_process = FindProcessByCharacteristicName(characteristic_name, ((*fnd).second), characteristic_number);
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicName(const std::string& characteristic_name, ProcessList* proclist, long* characteristic_number)
    {
        Process* found_process = 0;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            const CharacteristicDescriptor& char_desc = (*iter)->FindCharacteristicName(characteristic_name, characteristic_number);
            if (char_desc.operator!=(CharacteristicDescriptor::Null))
            {
                found_process = (*iter);
                break;
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByVariantCharacteristicName(const std::string& characteristic_name, long* characteristic_number)
    {
        // First, check for an exact match
        Process* found_process = FindProcessByCharacteristicName(characteristic_name, characteristic_number);
        if (found_process) return found_process;

        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            found_process = FindProcessByVariantCharacteristicName(characteristic_name, ((*iter).second), characteristic_number);
            if (found_process != 0)
            {
                break; // return (found_process);
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByVariantCharacteristicName(const std::string& characteristic_name, const ScoreboardStratum& stratum, long* characteristic_number)
    {
        // First, check for an exact match
        Process* found_process = FindProcessByCharacteristicName(characteristic_name, stratum, characteristic_number);
        if (found_process) return found_process;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_process = FindProcessByVariantCharacteristicName(characteristic_name, ((*fnd).second), characteristic_number);
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByVariantCharacteristicName(const std::string& characteristic_name, ProcessList* proclist, long* characteristic_number)
    {
        // First, check for an exact match
        Process* found_process = FindProcessByCharacteristicName(characteristic_name, proclist, characteristic_number);
        if (found_process) return found_process;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            const CharacteristicDescriptor& char_desc = (*iter)->FindVariantCharacteristicName(characteristic_name, characteristic_number);
            if (char_desc.operator!=(CharacteristicDescriptor::Null))
                //if ( CharacteristicDescriptor::Null != char_desc )
            {
                found_process = (*iter);
                break;
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByProcessID(ProcessIdentifier process_id)
    {
        Process* found_process = 0;
        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            found_process = FindProcessByProcessID(process_id, ((*iter).second));
            if (found_process != 0)
            {
                return (found_process);
            }
        }
        // MSA 11.02.10 Modifying this method to search m_processListALL if the process is not found in m_processLists.
        return FindProcessByProcessID(process_id, &m_processListALL);;
    }

    Process* ProcessCoordinator::FindProcessByProcessID(ProcessIdentifier process_id, const ScoreboardStratum& stratum)
    {
        Process* found_process = 0;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_process = FindProcessByProcessID(process_id, ((*fnd).second));
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByProcessID(ProcessIdentifier process_id, ProcessList* proclist)
    {
        Process* found_process = 0;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            if ((*iter)->GetProcessID() == process_id)
            {
                found_process = (*iter);
                break;
            }

            ProcessGroupLeader* pgl = wxDynamicCast((*iter), ProcessGroupLeader);
            if (__nullptr != pgl)
            {
                if (0 != (found_process = pgl->FindProcessByProcessID(process_id)))
                {
                    break;
                }
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByProcessName(const std::string& pname)
    {
        Process* found_process = 0;

        ProcessList::ProcessListType& private_list = m_processListALL.getPrivateList();
        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            if ((*iter)->GetProcessName() == pname)
            {
                found_process = (*iter);
                break;
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByProcessName(const std::string& pname, const ScoreboardStratum& stratum)
    {
        Process* found_process = 0;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_process = FindProcessByProcessName(pname, ((*fnd).second));
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByProcessName(const std::string& pname, ProcessList* proclist)
    {
        Process* found_process = 0;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            if ((*iter)->GetProcessName() == pname)
            {
                found_process = (*iter);
                break;
            }
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicIndex(long int characteristic_index)
    {
        Process* found_process = 0;
        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            found_process = FindProcessByCharacteristicIndex(characteristic_index, ((*iter).second));
            if (found_process != 0)
            {
                return (found_process);
            }
        }

        // MSA 11.02.10 Modifying this method to search m_processListALL if the process is not found in m_processLists.
        return FindProcessByCharacteristicIndex(characteristic_index, &m_processListALL);
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicIndex(long int characteristic_index, const ScoreboardStratum& stratum)
    {
        Process* found_process = 0;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_process = FindProcessByCharacteristicIndex(characteristic_index, ((*fnd).second));
        }

        return found_process;
    }

    Process* ProcessCoordinator::FindProcessByCharacteristicIndex(long int characteristic_index, ProcessList* proclist)
    {
        Process* found_process = 0;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            const CharacteristicDescriptor& char_desc = (*iter)->FindCharacteristicIndex(characteristic_index);
            if (char_desc.operator!=(CharacteristicDescriptor::Null))
                //if ( CharacteristicDescriptor::Null != char_desc )
            {
                found_process = (*iter);
                break;
            }
        }

        return found_process;
    }


    long int ProcessCoordinator::FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name)
    {
        long int found_characteristicindex = InvalidCharacteristicIndex;
        for (ProcessListCollection::iterator iter = m_processLists.begin();
            iter != m_processLists.end();
            ++iter)
        {
            found_characteristicindex = FindCharacteristicIndexByCharacteristicName(characteristic_name, ((*iter).second));
            if (found_characteristicindex != InvalidCharacteristicIndex)
            {
                return found_characteristicindex;
            }
        }

        // MSA 11.02.10 Modifying this method to search m_processListALL if the process is not found in m_processLists.
        return FindCharacteristicIndexByCharacteristicName(characteristic_name, &m_processListALL);
    }

    long int ProcessCoordinator::FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name, const ScoreboardStratum& stratum)
    {
        long int found_characteristicindex = InvalidCharacteristicIndex;

        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            found_characteristicindex = FindCharacteristicIndexByCharacteristicName(characteristic_name, ((*fnd).second));
        }
        return found_characteristicindex;
    }

    long int ProcessCoordinator::FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name, ProcessList* proclist)
    {
        long int found_characteristicindex = InvalidCharacteristicIndex;
        long int found_characteristicnumber = InvalidCharacteristicIndex;

        ProcessList::ProcessListType& private_list = proclist->getPrivateList();

        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            const CharacteristicDescriptor& char_desc = (*iter)->FindCharacteristicName(characteristic_name, &found_characteristicnumber);
            if (char_desc.operator!=(CharacteristicDescriptor::Null))
                //if ( CharacteristicDescriptor::Null != char_desc )
            {
                found_characteristicindex = char_desc.ScoreboardIndex;
                break;
            }
        }

        return found_characteristicindex;
    }


    void ProcessCoordinator::AddProcess(Process* p, ScoreboardStratum stratumOverride)
    {
        ScoreboardStratum use_stratum = stratumOverride;
        if (stratumOverride == ScoreboardStratum::NONE)
        {
            use_stratum = p->GetProcessStratum();
        }

        // all processes are added to the ALL list implicitly; don't add it explicitly
        if (use_stratum != ScoreboardStratum::ALL)
        {
            // only makes if required, otherwise re-uses
            ProcessList* process_list = MakeProcessList(use_stratum);
            process_list->AddTo(p);
        }

        m_processListALL.AddTo(p);
    }

    ProcessList* ProcessCoordinator::MakeProcessList(const ScoreboardStratum& stratum)
    {
        ProcessList* proclist = 0;
        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd == m_processLists.end())
        {
            proclist = new ProcessList;
            ProcessListCollection::value_type value(stratum.value(), proclist);
            m_processLists.insert(value);
        }
        else
        {
            proclist = (*fnd).second;
        }

        return (proclist);
    }

    /* GetProcessList
    A unified method of getting hold of a process list. */
    ProcessList* ProcessCoordinator::GetProcessList(const ScoreboardStratum& stratum)
    {
        ProcessList* proclist = 0;
        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            proclist = (*fnd).second;
        }
        return (proclist);
    }


    long ProcessCoordinator::TotalNumberOfProcesses() const
    {
        return (m_processListALL.GetNumProcesses());
    }

    void ProcessCoordinator::IterateOverProcesses(IProcessIterationUser* puser)
    {
        // retrieve the internal list of processes
        ProcessList::ProcessListType& private_list = m_processListALL.getPrivateList();

        // which can now be iterated over
        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            puser->UseProcess(*iter);
        }
    }

    void ProcessCoordinator::IterateOverProcesses(IProcessIterationUser* puser, const ScoreboardStratum& stratum)
    {
        // retrieve this stratum's process list, if one exists
        ProcessListCollection::iterator fnd = m_processLists.find(stratum.value());
        if (fnd != m_processLists.end())
        {
            // retrieve the internal list of processes
            ProcessList::ProcessListType& private_list = ((*fnd).second)->getPrivateList();

            // which can now be iterated over
            for (ProcessList::ProcessListType::iterator iter = private_list.begin();
                iter != private_list.end();
                ++iter)
            {
                puser->UseProcess((*iter), stratum);
            }
        }
    }

    void ProcessCoordinator::populateCharacteristicStringArray(wxArrayString& characteristics)
    {
        ProcessList::ProcessListType& private_list = m_processListALL.getPrivateList();

        // which can now be iterated over
        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            Process* p = *iter;
            long num_characteristics = p->GetNumCharacteristics();
            for (long index = 1; index <= num_characteristics; ++index)
            {
                if (p->CharacteristicIsVisible(index))
                {
                    // wxLogMessage(wxT("Process %s CharacteristicName %s CharacteristicIndex %d"), (p->GetProcessName()).c_str(), (p->GetCharacteristicName(index)).c_str(), p->GetCharacteristicIndex(index));
                    characteristics.Add((p->GetCharacteristicName(index)).c_str());
                }
            }
        }
    }

    void ProcessCoordinator::populateProcessStringArray(wxArrayString& processes, std::vector<ProcessIdentifier>& processIds, PopulateProcessStringFlags flags)
    {
        ProcessList::ProcessListType& private_list = m_processListALL.getPrivateList();

        // which can now be iterated over
        for (ProcessList::ProcessListType::iterator iter = private_list.begin();
            iter != private_list.end();
            ++iter)
        {
            Process* p = *iter;
            bool do_add = false;
            switch (flags)
            {
            case ppsf_DrawingOnly:
                do_add = p->DoesDrawing();
                break;

            case ppsf_All:
            default:
                do_add = true;
                break;
            }

            if (do_add)
            {
                processes.Add((p->GetProcessName()).c_str());
                processIds.push_back(p->GetProcessID());
            }
        }
    }

    void ProcessCoordinator::findProcessesByProcessId(const ProcessIdentifierSet& processIds, ProcessArray& processes)
    {
        for (ProcessIdentifierSet::const_iterator pidit(processIds.begin());
            processIds.end() != pidit; ++pidit)
        {
            Process* p = FindProcessByProcessID(*pidit);
            processes.push_back(p);
        }
    }

    Process* ProcessCoordinator::MakeProcess(const ProcessDAI& process_data)
    {
        Process* p = __nullptr;
        if (process_data.doesOverride())
        {
            wxString pname = process_data.getName().c_str();
            wxString pname_ = Utility::TrimAll(pname);

            wxObject* object = wxCreateDynamicObject(pname_.c_str());
            RmAssert(object != 0, "Failed to create dynamic object by name");

            p = wxDynamicCast(object, Process);
            RmAssert(p != 0, "Class was not derived from Process");

            if (m_volumeObjectCoordinator != __nullptr) // TODO add member to processes to determine which do VO stuff
            {
                p->SetVolumeObjectCoordinator(*m_volumeObjectCoordinator);
            }

            p->Initialise(process_data);

            ProcessGroupLeader* pgl = wxDynamicCast(object, ProcessGroupLeader);
            if (__nullptr != pgl)
            {
                pgl->Initialise(m_engine);
            }
        }
        else
        {
            p = new Process;
            p->Initialise(process_data);
        }

        if (p->GetProcessName() == "VolumeObjectCoordinator")
        {
            m_volumeObjectCoordinator = dynamic_cast<const VolumeObjectCoordinator*>(p);
        }

        return p;
    }
} /* namespace rootmap */
