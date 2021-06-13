#ifndef ProcessList_H
#define ProcessList_H
/*
Process List

This class deals with keeping a list of, obviously, the processes in a certain volume.
The main reason we overrode CVoidPtrArray is to alphabetically add the processes.
Along the way, function members have been added for characteristics :
Count[Savable/Visible/Scoreboard]Characteristics.

Modifications
99.01.06    GetNumItems(). Comments there.
99.01.23    AddTo(). Fixed alphabetical ordering. Comments there
RvH 20060527 now "has a" rather than "is a" list
 */
#include "core/common/Structures.h"

#include <list>

namespace rootmap
{
    class Process;

    class ProcessList
    {
    public:
        ///
        /// MSA 10.08.26 Converted ProcessListType from vector to list.
        /// This is because AddTo() now (may) do insertions, which are much faster in a linked list than in a vector.
        /// HOWEVER - if there are other places where a ProcessList is used in ways which are faster vector-wise, 
        /// feel free to change it back. The amount of time spent in AddTo() is trivial.
        typedef std::list<Process *> ProcessListType;

        /**
         */
        ProcessList();

        /**
         */
        ~ProcessList();

        /**
         */
        void AddTo(Process* aProcess);

        /**
         */
        long GetNumProcesses() const;

        /**
         */
        long CountSavableCharacteristics();

        /**
         */
        long CountVisibleCharacteristics();

        /**
         */
        long CountScoreboardCharacteristics();

        /**
         *
         */
        ProcessListType& getPrivateList();

    private:

        ///
        /// 
        ProcessListType m_processes;
    };

    inline ProcessList::ProcessListType& ProcessList::getPrivateList()
    {
        return m_processes;
    }
} /* namespace rootmap */

#endif // #ifndef ProcessList_H
