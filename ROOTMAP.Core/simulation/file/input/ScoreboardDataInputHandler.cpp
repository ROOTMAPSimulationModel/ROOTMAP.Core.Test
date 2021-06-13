#include "BoostPrecomp.h"


#include "ScoreboardDataInputHandler.h"
#include "Scoreboard.h"
#include "core/utility/Utility.h"

#include <cstdlib>
#include "simulation/common/BoostPrecomp.h"

namespace rootmap
{

    ScoreboardDataInputHandler::ScoreboardDataInputHandler()
        : myScoreboard(NULL)
    {
        inputfile_characteristicindex = 0;
        inputfile_scoreboard = NULL;

        TCL_END_CONSTRUCTOR
    }

    void ScoreboardDataInputHandler::IScoreboardDataInputHandler(long target_index, Scoreboard * target_scoreboard)
    {
        inputfile_characteristicindex = target_index;
        inputfile_scoreboard = target_scoreboard;
    }

    void ScoreboardDataInputHandler::ParseNextPortion()
    {
        char* token;

        token = strtok(inputfile_original_buffer, inputfile_delimiters);

        if (strcmp(token, "XYZ") == 0)
            ParseAsXYZ(NULL);
        else if (strcmp(token, "XZY") == 0)
            ParseAsXZY(NULL);
        else // no dimension order has been specified in the file.
        // Thus we assume XZY, but backtrack to the start of the file. This is because instead of reading
        // in the "XZY" or "XYZ" or whatever, we have already gone past the first number
            ParseAsXZY(inputfile_original_buffer);

        inputfile_finished = true;
    } // ParseNextPortion()

    void ScoreboardDataInputHandler::ParseAsXYZ(char* strtok_starter)
    {
        // index counters for scoreboard access
        long x_index, y_index, z_index;
        // maximum scoreboard indices
        long x_max, y_max, z_max;
        // the value itself
        double_cs value;
        
        char* token;

        inputfile_scoreboard->GetNumLayers(x_max, y_max, z_max);

        token = strtok(strtok_starter, inputfile_delimiters);

        z_index= 1;
        while (z_index<= z_max)
        {
            y_index= 1;
            while (y_index<= y_max)
            {
                x_index= 1;
                while ((x_index<= x_max) && (token != NULL))
                {
                    value = StringToDouble(token);

                    inputfile_scoreboard->SetCharacteristicValue(inputfile_characteristicindex, value, x_index, y_index, z_index);
                    
                    token = strtok(NULL, inputfile_delimiters);

                    x_index++;
                } // while (x_index<= x_max)

                y_index++;
            } // while (y_index<= y_max)

            z_index++;
        } // while (z_index<= z_max)
    }

    void ScoreboardDataInputHandler::ParseAsXZY(char* strtok_starter)
    {
        // index counters for scoreboard access
        long x_index, y_index, z_index;
        // maximum scoreboard indices
        long x_max, y_max, z_max;
        // the value itself
        double value;
        
        char* token;

        inputfile_scoreboard->GetNumLayers(x_max, y_max, z_max);

        token = strtok(strtok_starter, inputfile_delimiters);

        y_index= 1;
        while (y_index<= y_max)
        {
            z_index= 1;
            while (z_index<= z_max)
            {
                x_index= 1;
                while ((x_index<= x_max) && (token != NULL))
                {
                    value = StringToDouble(token);

                    inputfile_scoreboard->SetCharacteristicValue(inputfile_characteristicindex, value, x_index, y_index, z_index);
                    
                    token = strtok(NULL, inputfile_delimiters);

    //                DoCheckMemoryBlockErrors("\pScoreboardDataInputHandler::ParseAsXZY");

                    x_index++;
                } // while (x_index<= x_max)

                z_index++;
            } // while (z_index<= z_max)

            y_index++;
        } // while (y_index<= y_max)
    }

} // namespace rootmap

