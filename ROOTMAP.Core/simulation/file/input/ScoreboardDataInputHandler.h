#ifndef ScoreboardDataInputHandler_H
#define ScoreboardDataInputHandler_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataInputHandler.h
//
// Purpose:     Derivative of DataInputHandler, for handling scoreboard data,
//              specifically that which comes from the <data></data> tag/element
//              of the initialisation/scoreboarddata element of an xml file
//
// Created:     01/05/2006
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#include "DataInputHandler.h"

namespace rootmap
{
    class Scoreboard;

    class ScoreboardDataInputHandler : public DataInputHandler
    {
        public:

        // Construction : 
            // default constructor+initialiser
            ScoreboardDataInputHandler();
            ScoreboardDataInputHandler(CharacteristicIndex target_index, Scoreboard * target_scoreboard);
            void IScoreboardDataInputHandler(CharacteristicIndex target_index, Scoreboard *target_scoreboard);

        // overridden to do the usual stuff
            virtual void HandleInput();

            void ParseAsXYZ(char* strtok_starter);
            void ParseAsXZY(char* strtok_starter);
        private:
        // Data members
            CharacteristicIndex myCharacteristicIndex;
            Scoreboard        * myScoreboard;
    };

} // namespace rootmap

#endif // #ifndef ScoreboardDataInputHandler_H
