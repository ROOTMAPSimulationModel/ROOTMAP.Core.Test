/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessCoordinator.h
// Purpose:     Declaration of the ProcessCoordinator class
// Created:     27/05/2006
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef ProcessCoordinator_H
#define ProcessCoordinator_H

#include "simulation/common/Types.h"
#include "simulation/process/common/ProcessList.h"


#include "core/scoreboard/ScoreboardStratum.h"

#include <map>
#include <set>
#include <vector>

#include "wx/arrstr.h"

namespace rootmap
{
    class Process;
    class IProcessIterationUser;
    class ScoreboardStratum;
    class ProcessDAI;
    class SimulationEngine;
    class DataAccessManager;
    class VolumeObjectCoordinator;

    class ProcessCoordinator
    {
    public:
        ProcessCoordinator(SimulationEngine& engine, DataAccessManager& dam);
        virtual ~ProcessCoordinator();

        void createProcess(const ProcessDAI& process_data);

        /**
         * Creates a new ProcessList for the specified stratum, and returns the
         * new list. If a processlist already exists for that stratum, just returns
         * the one there.
         *
         * @param
         * @return
         */
        ProcessList* MakeProcessList(const ScoreboardStratum& stratum);

        /**
         *
         */
        ProcessList* GetProcessList(const ScoreboardStratum& stratum);

        /**
         *
         */
        long TotalNumberOfProcesses() const;

        /**
         *
         */
        Process* FindProcessByCharacteristicID(long characteristic_id);
        Process* FindProcessByCharacteristicID(long characteristic_id, const ScoreboardStratum& stratum);
        Process* FindProcessByCharacteristicID(long characteristic_id, ProcessList* proclist);

        /**
         *
         */
        Process* FindProcessByCharacteristicName(const std::string& characteristic_name, long* characteristic_number);
        Process* FindProcessByCharacteristicName(const std::string& characteristic_name, const ScoreboardStratum& stratum, long* characteristic_number);
        Process* FindProcessByCharacteristicName(const std::string& characteristic_name, ProcessList* proclist, long* characteristic_number);

        Process* FindProcessByVariantCharacteristicName(const std::string& characteristic_name, long* characteristic_number);
        Process* FindProcessByVariantCharacteristicName(const std::string& characteristic_name, const ScoreboardStratum& stratum, long* characteristic_number);
        Process* FindProcessByVariantCharacteristicName(const std::string& characteristic_name, ProcessList* proclist, long* characteristic_number);


        /**
         *
         */
        Process* FindProcessByProcessID(ProcessIdentifier process_id);
        Process* FindProcessByProcessID(ProcessIdentifier process_id, const ScoreboardStratum& stratum);
        Process* FindProcessByProcessID(ProcessIdentifier process_id, ProcessList* proclist);

        /**
         *
         */
        Process* FindProcessByProcessName(const std::string& pname);
        Process* FindProcessByProcessName(const std::string& pname, const ScoreboardStratum& stratum);
        Process* FindProcessByProcessName(const std::string& pname, ProcessList* proclist);

        /**
         *
         */
        Process* FindProcessByCharacteristicIndex(long int characteristic_index);
        Process* FindProcessByCharacteristicIndex(long int characteristic_index, const ScoreboardStratum& stratum);
        Process* FindProcessByCharacteristicIndex(long int characteristic_index, ProcessList* proclist);

        /**
         *
         */
        long int FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name);
        long int FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name, const ScoreboardStratum& stratum);
        long int FindCharacteristicIndexByCharacteristicName(const std::string& characteristic_name, ProcessList* proclist);

        /**
         *
         */
        void IterateOverProcesses(IProcessIterationUser* puser);
        void IterateOverProcesses(IProcessIterationUser* puser, const ScoreboardStratum& stratum);

        /**
         * Populates the given string array with the names of all the characteristics.
         */
        void populateCharacteristicStringArray(wxArrayString& characteristics);


        /**
         * Populates the given string array with the names of all the processes.
         */
        void populateProcessStringArray(wxArrayString& processes, std::vector<ProcessIdentifier>& processIds, PopulateProcessStringFlags flags);

        /**
         * Specialised function for View windows process selection update. We
         * could repeatedly call FindProcessByProcessID from the GuiSimulationEngine
         * however that will not scale to many processes.
         */
        void findProcessesByProcessId(const ProcessIdentifierSet& processIds, ProcessArray& processes);


        /**
         * Required for processes that aren't created from DAI, via createProcess(). This
         * applies to the PlantCoordinator.
         * @param p the process
         * @param stratumOverride [optional] specifies the stratum in which to
         *        place the process, rather than the process's getStratum().
         *        If the no stratum is specified, or NONE is given, the process's
         *        getStratum() will be used.
         */
        void AddProcess(Process* p, ScoreboardStratum stratumOverride = StratumNONE /*ScoreboardStratum::NONE*/);

    private:

        /**
         *
         */
        Process* MakeProcess(const ProcessDAI& process_data);

    private:
        ///
        /// This is a list of ALL processes. *NOT* the list where processes in
        /// the ScoreboardStratum::ALL are kept
        ProcessList m_processListALL;

        ///
        /// We maintain the process lists as a map keyed on the scoreboardstratum
        typedef std::map<long, ProcessList *> ProcessListCollection;

        ///
        /// 
        /// Note that processes can be in the special ScoreboardStratum::ALL
        ProcessListCollection m_processLists;

        ///
        /// 
        SimulationEngine& m_engine;

        // A bit ugly, having this here.
        const VolumeObjectCoordinator* m_volumeObjectCoordinator;
    }; // class ProcessCoordinator
} /* namespace rootmap */

#endif // #ifndef ProcessCoordinator_H
