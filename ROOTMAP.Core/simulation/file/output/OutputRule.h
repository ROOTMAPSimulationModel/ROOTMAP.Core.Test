#ifndef OutputRule_H
#define OutputRule_H
/////////////////////////////////////////////////////////////////////////////
// Name:        OutputRule.h
// Purpose:     Declaration of the OutputRule class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
// 
// This class defines some of the aspects about how a process or other object wants to export its data.
// 
// It works in combination with the classes :
// DataOutputCoordinator (coordinates the action and causes the other classes to sort themselves out)
// and usually OutputFileAlarm
// 
// Uses OutputTimingStrategy to determine if output should occur at a given time.
// Uses OutputStrategy to perform output activity.
// The file naming is performed by this class
//
/////////////////////////////////////////////////////////////////////////////

#include "simulation/common/Types.h"
//#include "simulation/file/output/OutputRuleDefs.h"
#include "simulation/process/interprocess_communication/Message.h"

#include "core/scoreboard/ScoreboardStratum.h"
#include "core/common/Types.h"

#include "wx/string.h"

namespace rootmap
{
    //class Process;
    class DataOutputFile;
    class DataOutputCoordinator;
    class ProcessActionDescriptor;
    class SimulationEngine;
    class OutputTimingStrategy;
    class OutputStrategy;
    class OutputRuleDAI;
    class IDataOutputSource;


    class OutputRule : public SpecialProcessData
    {
    public:

        enum OutputReopenStrategy
        {
            append,
            overwrite
        };

        /**
         *
         */
        OutputRule(const OutputRuleDAI& data, SimulationEngine& engine);

        /**
         *
         */
        ~OutputRule();

        /**
         * Data type determines the class name of the file used to do the exporting
         */
        const wxString& getType() const;

        /**
         * Data source name is the <source> in the xml
         */
        const wxString& getDataSourceName() const;

        /**
         * Data source characteristic applies in the more general non-scoreboard
         * sense here
         */
        const wxString& getDataSourceCharacteristic() const;

        /**
         * TODO: is this just for ScoreboardDataOutputRules ?
         * Quick Answer: Yes, but non-scoreboard data is likely to use this
         * also, and can set to NONE or ALL if not required
         */
        const ScoreboardStratum& getScoreboardStratum() const;

        /**
         * Accessor for output file instance
         */
        DataOutputFile* getOutputFile();

        /**
         * Used by OutputFileAlarm, to indicate that this rule has already been
         * used to perform data output for the given timestamp
         */
        void UpdateTime(ProcessTime_t t);

        /**
         * Determines the next time this rule would be required to perform data
         * output, given the current time.
         */
        ProcessTime_t GetNextTime(ProcessTime_t t);

        /**
         * Determines if this rule is requiring data output during the given timestamp
         */
        bool DoesThisTime(ProcessTime_t t);

        /**
         * Provokes the source to export data into the destination
         */
        virtual void ArrangeOutput(ProcessActionDescriptor* action);

        /**
         * Calls OutputStrategy::OutputToFile(m_destination_file);
         */
        virtual void OutputToFile(ProcessActionDescriptor* action);


    private:

        /**
         *
         */
        void MakeFileName(wxString& s);

        /**
         *
         */
        DataOutputFile* MakeFile(const wxString& s);

    private:

        ///

        /// 
        /// the xml objectdata/type, eg. ScoreboardData. This is used to construct
        /// the name of the output data file class
        wxString m_type;

        /// 
        /// the process/object name
        wxString m_source_name;

        /// 
        /// the characteristic name, or 'Special'
        wxString m_source_variation;

        ///
        /// TODO: only relevant for ScoreboardData ?
        ScoreboardStratum m_source_stratum;

        ///
        /// rules about when to export - see the OutputRelations file for a description
        OutputTimingStrategy* m_outputTimingStrategy;

        ///
        /// 
        wxString m_fileNameFormat;

        ///
        /// 
        wxString m_lastFileName;

        ///
        ///
        OutputStrategy* m_outputStrategy;

        /// 
        ///
        OutputReopenStrategy m_reopen_strategy;

        /// 
        /// the time of export, for MakeFileName
        ProcessTime_t m_now;

        /// 
        /// the current destination of the data.
        DataOutputFile* m_destination_file;

        ///
        ///
        SimulationEngine& m_simulationEngine;

        wxString m_directory;

        ///
        ///
        enum WXFILETYPE
        {
            JustFile,
            FFile
        };

        ///
        ///
        WXFILETYPE m_wxFileType;
    };


    inline const wxString& OutputRule::getType() const
    {
        return (m_type);
    }

    inline const wxString& OutputRule::getDataSourceName() const
    {
        return (m_source_name);
    }

    inline const wxString& OutputRule::getDataSourceCharacteristic() const
    {
        return (m_source_variation);
    }

    inline const ScoreboardStratum& OutputRule::getScoreboardStratum() const
    {
        return (m_source_stratum);
    }

    inline DataOutputFile* OutputRule::getOutputFile()
    {
        return (m_destination_file);
    }
} /* namespace rootmap */

#endif // #ifndef OutputRule_H
