#include "simulation/process/common/ProcessList.h"
#include "simulation/process/common/Process.h"


namespace rootmap
{
    ProcessList::ProcessList()
    {
    }


    /*
    19990902 remove the menu item associated with any characteristics*/
    ProcessList::~ProcessList()
    {
        // IMPORTANT NOTE: Processes all delete'd in ProcessCoordinator.

        // Forget all the processes
        //Process * p;
        //for ( ProcessListType::iterator iter = m_processes.begin() ;
        //        iter != m_processes.end() ;
        //        ++iter
        //    )
        //{
        //    p = *iter;
        //    delete p;
        //}
    }

    /*
    This member function is the primary motivation for this subclass of
    CVoidPtrArray. When adding a process to this list, we do two things.

    The first is that we keep track of the indices to where the characteristics summarise
    to in the scoreboardÕ boxes. This is so we know the index to where the next characteristic
    will summarise to. For this, we -
    1. Set the process' first characteristic's index to what our presently used space equals
    2. Add that process' characteristics' requirements to the running total

    The second is that we insert the processes in alphabetical order, except for the process
    called "PlantCoordinator", which comes first. This basically involves iterating through the list until
    we find a process which should come after us.

    Modifications
    99.01.23    unfortunately, if the while() loop finishes because of the 2nd clause,
            ie. it comes to the end of the current list before finding a slot, it places
            the new process second last. This is, of course undesired. The fix, was
            to check for whether a slot was found, before assigning "insert_at"
            more appropriately
    */
    void ProcessList::AddTo(Process* aProcess)
    {
#if defined ROOTMAP_TODO // need to insert alphabetically ?
        ProcessListType::iterator iter = m_processes.begin(); // for scope
        for (; iter != m_processes.end(); ++iter)
        {
            Process * checkProcess = *iter;
            Str255 aName, checkName;
            StringPtr an = aName; StringPtr cn = checkName;

            while ((!found_slot) && (processIterator.Next(&checkProcess)))
            {
                aProcess->GetProcessName(an, 0);
                checkProcess->GetProcessName(cn, 0);
                found_slot = (RelString(aName, checkName, true, true) < 0);
            }

            insert_at = processIterator.GetCursor();
            if (!found_slot) ++insert_at;
        }

        // Finally, add the process where we want
        m_processes.insert(iter, aProcess);
#endif // #if defined ROOTMAP_TODO

        // MSA 10.08.26 Sort additions so that Water comes before Nitrate and Phosphorus.
        // This is kludgy and nasty, but as Water, Nitrate and Phosphorus are integral parts of the model
        // and Water will always need to be processed (woken up) before Nitrate and Phosphorus,
        // as they depend on it. Right?
        const static std::string waterName = "Water";
        const static std::string pName = "Phosphorus";
        const static std::string nName = "Nitrate";

        if (aProcess->GetProcessName().compare(waterName))
        {
            // Process is not Water. Tack it on the end.
            m_processes.push_back(aProcess);
            return;
        }

        ProcessListType::iterator iter = m_processes.begin(); // for scope
        for (; iter != m_processes.end(); ++iter)
        {
            const std::string& thisProcessName = (*iter)->GetProcessName();
            // If this is Phosphorus or Nitrate, place the new (Water) Process before this one
            if (!thisProcessName.compare(pName) || !thisProcessName.compare(nName))
            {
                m_processes.insert(iter, aProcess);
                return;
            }
        }

        // If neither Nitrate nor Phosphorus were found, it's safe to tack Water on the end.
        m_processes.push_back(aProcess);
    }


    long ProcessList::GetNumProcesses() const
    {
        return (m_processes.size());
    }


    long ProcessList::CountSavableCharacteristics()
    {
        long total_number_of_characteristics = 0;

        for (ProcessListType::iterator iter = m_processes.begin();
            iter != m_processes.end();
            ++iter)
        {
            total_number_of_characteristics += (*iter)->GetNumSavedCharacteristics();
        }

        return (total_number_of_characteristics);
    }


    long ProcessList::CountVisibleCharacteristics()
    {
        long total_number_of_characteristics = 0;

        for (ProcessListType::iterator iter = m_processes.begin();
            iter != m_processes.end();
            ++iter)
        {
            total_number_of_characteristics += (*iter)->GetNumVisibleCharacteristics();
        }

        return (total_number_of_characteristics);
    }


    long ProcessList::CountScoreboardCharacteristics()
    {
        long total_number_of_characteristics = 0;

        for (ProcessListType::iterator iter = m_processes.begin();
            iter != m_processes.end();
            ++iter)
        {
            total_number_of_characteristics += (*iter)->GetNumCharacteristics();
        }

        return (total_number_of_characteristics);
    }
} /* namespace rootmap */


