#ifndef IScoreboardIterationUser_H
#define IScoreboardIterationUser_H

/////////////////////////////////////////////////////////////////////////////
// Name:        IScoreboardIterationUser.h
// Purpose:     Declaration of the IScoreboardIterationUser class
// Created:     06/06/2006
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
//
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class Scoreboard;

    class IScoreboardIterationUser
    {
    public:
        IScoreboardIterationUser();
        virtual ~IScoreboardIterationUser();

        /**
         * UseScoreboard
         *
         * Called by the ScoreboardCoordinator for each scoreboard iteration.
         *
         * @param scoreboard - the current iteration of scoreboard to be used
         */
        virtual void UseScoreboard(Scoreboard* scoreboard) = 0;
    }; // class IScoreboardIterationUser
} /* namespace rootmap */

#endif // #ifndef IScoreboardIterationUser_H
