/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataOutputStrategy.cpp
// Purpose:     Implementation of the ScoreboardDataOutputStrategy class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/ScoreboardDataOutputStrategy.h"
#include "simulation/file/output/DataOutputFile.h"
#include "simulation/file/output/OutputRule.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/DimensionIterator.h"


//#define ADDRESSINDICES 1

namespace rootmap
{
    ScoreboardDataOutputStrategy::ScoreboardDataOutputStrategy
    (Scoreboard* scoreboard,
        Process* p,
        CharacteristicIndex cindex,
        const DimensionOrder& dim_order)
        : m_scoreboard(scoreboard)
        , m_process(p)
        , m_characteristicIndex(cindex)
        , m_dimensionOrder(dim_order)
    {
    }


    void ScoreboardDataOutputStrategy::ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule)
    {
        Use_PostOffice;

        postoffice->sendMessage(m_process, m_process, kSpecialOutputDataMessage, output_rule);
    }

    void ScoreboardDataOutputStrategy::OutputToFile(DataOutputFile& file)
    {
        //
        //        // index counters for scoreboard access
        //        long x_index, y_index, z_index;
        //        long *inner, *middle, *outer;
        //        m_dimensionOrder.MatchDimensions(
        //                        &x_index, &y_index, &z_index,
        //                        &inner, &middle, &outer);
        //
        //        // maximum scoreboard indices
        //        long x_max, y_max, z_max;
        //        long *inner_max, *middle_max, *outer_max;
        //        m_scoreboard->GetNumLayers(x_max, y_max, z_max);
        //        m_dimensionOrder.MatchDimensions(
        //                        &x_max, &y_max, &z_max,
        //                        &inner_max, &middle_max, &outer_max);
        //
        //        signed int inner_increment, middle_increment, outer_increment;
        //        m_dimensionOrder.MatchIncrements(inner_increment, middle_increment, outer_increment);
        //

        BoxCoordinate box_coord;
        DimensionIterator iter(m_dimensionOrder, m_scoreboard, box_coord);

        iter.outer_init();
        while (iter.outer_do_continue())
        {
            iter.middle_init();
            while (iter.middle_do_continue())
            {
                iter.inner_init();
                while (iter.inner_do_continue())
                {
                    //file << m_scoreboard->GetCharacteristicValue(m_characteristicIndex, x_index, y_index, z_index) << ',';
                    file << m_scoreboard->GetCharacteristicValue(m_characteristicIndex, box_coord) << ',';

                    iter.inner_next();
                } // while (iter.inner_do_continue())

                // Goto the next line
                file << '\n'; // would have preferred std::endl but isn't overloaded for this stream

                iter.middle_next();
            } // while (iter.middle_do_continue())

            // Goto the next line
            file << '\n'; // would have preferred std::endl but isn't overloaded for this stream

            iter.outer_next();
        } // while (iter.outer_do_continue())
    }
} /* namespace rootmap */

