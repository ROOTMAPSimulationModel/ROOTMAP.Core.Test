#ifndef ScoreboardDependencyIterator_H
#define ScoreboardDependencyIterator_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDependencyIterator.h
// Purpose:     Declaration of the ScoreboardDependencyIterator class
// Created:     06/06/2006
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2006 University of Tasmania
//
/////////////////////////////////////////////////////////////////////////////
#include "simulation/scoreboard/IScoreboardIterationUser.h"
#include "simulation/scoreboard/Scoreboard.h"

namespace rootmap
{
    class Scoreboard;

    class ScoreboardDependencyIterator
        : public IScoreboardIterationUser
    {
    private:
        ::CCollaborator* m_collaborator;

        bool m_depend;

    public:
        ScoreboardDependencyIterator(::CCollaborator* collab, bool fDepend)
            : m_collaborator(collab)
            , m_depend(fDepend)
        {
            /* Do Nothing */
        }


        ~ScoreboardDependencyIterator()
        {
            /* Do Nothing */
        }


        /**
         * Called by the ScoreboardCoordinator for each scoreboard iteration.
         *
         * @param scoreboard - the current iteration of scoreboard to be used
         */
        void UseScoreboard(Scoreboard* scoreboard)
        {
            if (m_depend)
            {
                m_collaborator->DependUpon(scoreboard);
            }
            else
            {
                m_collaborator->CancelDependency(scoreboard);
            }
        }

    private:
        /// hidden copy constructor
        ScoreboardDependencyIterator(const ScoreboardDependencyIterator&);
        /// hidden assignment operator
        ScoreboardDependencyIterator& operator=(const ScoreboardDependencyIterator&);
    }; // class ScoreboardDependencyIterator
} /* namespace rootmap */

#endif // #ifndef ScoreboardDependencyIterator_H
