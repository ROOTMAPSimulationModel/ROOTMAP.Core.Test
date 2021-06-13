#ifndef ScoreboardDataOutputStrategy_H
#define ScoreboardDataOutputStrategy_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataOutputStrategy.h
// Purpose:     Declaration of the ScoreboardDataOutputStrategy class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/file/output/OutputStrategy.h"
#include "simulation/scoreboard/DimensionOrder.h"
#include "core/scoreboard/ScoreboardBox.h"

namespace rootmap
{
    class Scoreboard;
    class Process;

    //class ProcessActionDescriptor;

    class ScoreboardDataOutputStrategy : public OutputStrategy
    {
    public:
        /**
         *
         */
        ScoreboardDataOutputStrategy(Scoreboard* scoreboard,
            Process* p,
            CharacteristicIndex cindex,
            const DimensionOrder& dim_order);

        /**
         *
         */
        virtual void ArrangeOutput(ProcessActionDescriptor* action, OutputRule* output_rule);


        /**
         *
         */
        virtual void OutputToFile(DataOutputFile& file);


    private:
        ///
        ///
        Scoreboard* m_scoreboard;

        ///
        ///
        Process* m_process;

        ///
        ///
        CharacteristicIndex m_characteristicIndex;

        ///
        ///
        DimensionOrder m_dimensionOrder;
    }; // class ScoreboardDataOutputStrategy
} /* namespace rootmap */

#endif // #ifndef ScoreboardDataOutputStrategy_H
