/////////////////////////////////////////////////////////////////////////////
// Name:        OutputStrategy.cpp
// Purpose:     Implementation of of various OutputStrategy classes
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "simulation/common/SimulationEngine.h"
#include "simulation/file/output/OutputStrategy.h"
#include "simulation/file/output/ScoreboardDataOutputStrategy.h"
#include "simulation/file/output/RaytracerDataOutputStrategy.h"
#include "simulation/file/output/NonSpatialDataOutputStrategy.h"
#include "simulation/file/output/ViewCaptureOutputStrategy.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/common/Process.h"
#include "simulation/data_access/interface/OutputRuleDAI.h"
#include "simulation/process/raytrace/RaytracerDataAccessor.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/process/shared_attributes/SharedAttributeRegistrar.h"
#include "simulation/process/shared_attributes/ProcessSharedAttribute.h"
#include "simulation/process/shared_attributes/SharedAttributeSearchHelper.h"

#include "core/common/ExtraStuff.h"
#include "core/common/RmAssert.h"
#include "core/utility/StringParserUtility.h"
#include "core/scoreboard/Dimension.h"

#include "boost/scoped_ptr.hpp"


namespace rootmap
{
    // Utility class for adding all Raytracing processes to a ProcessList.
    class RaytracingProcessAdder : public IProcessIterationUser
    {
        RaytracerDataOutputStrategy::ProcessContainer& m_processContainer;
    public:
        RaytracingProcessAdder(RaytracerDataOutputStrategy::ProcessContainer& processContainer)
            : m_processContainer(processContainer)
        {
        }

        ~RaytracingProcessAdder()
        {
        }

        void UseProcess(Process* process)
        {
            if (process->DoesRaytracerOutput())
            {
                m_processContainer.insert(process);
            }
        }

        void UseProcess(Process* process, const ScoreboardStratum& stratum)
        {
            if (stratum == process->GetProcessStratum() && process->DoesRaytracerOutput())
            {
                m_processContainer.insert(process);
            }
        }
    };

    using namespace StringParserUtility;

    RootMapLoggerDefinition(OutputStrategyFactory);

    OutputStrategy* OutputStrategyFactory::createOutputStrategy
    (const OutputRuleDAI& data,
        SimulationEngine& engine)
    {
        RootMapLoggerInitialisation("rootmap.OutputStrategyFactory");

        std::string typestring = data.GetType();

        if ("ScoreboardData" == typestring)
        {
            long characteristic_number = -1;
            Process* p = engine.GetProcessCoordinator().FindProcessByCharacteristicName(data.GetName(), &characteristic_number);

            Scoreboard* scoreboard = engine.GetScoreboardCoordinator().GetScoreboard(data.GetStratum());

            DimensionOrder dim_order;
            std::string dimension_order = data.GetSpecification1();
            dim_order.Set(dimension_order);

            if ((0 != p) && (-1 < characteristic_number))
            {
                CharacteristicIndex cindex = p->GetCharacteristicIndex(characteristic_number);
                OutputStrategy* ostrategy = new ScoreboardDataOutputStrategy(scoreboard, p, cindex, dim_order);
                LOG_INFO << LOG_LINE << "Created ScoreboardData outputStrategy for " << data.GetProcessName() << ":" << data.GetName();
                return ostrategy;
            }
            else
            {
                LOG_ERROR << LOG_LINE << "Failed to create ScoreboardData outputStrategy for "
                    << data.GetProcessName() << ":" << data.GetName()
                    << " - process:" << p << ", number:" << characteristic_number;
                throw RmException("Failed to create ScoreboardDataOutputStrategy, invalid data");
            }
        } // end if("ScoreboardData"==typestring)

        else if ("RaytracerData" == typestring)
        {
            // MSA Construct RaytracerDataOutputStrategy

            // Retrieve a pointer to the main Process.
            // Any valid Process name will yield a valid pointer, but
            // it must do raytracer output, or this OutputStrategy will be abandoned.
            Process* process_ptr = engine.GetProcessCoordinator().FindProcessByProcessName(data.GetProcessName());
            if (__nullptr == process_ptr)
            {
                LOG_ERROR << LOG_LINE << "Failed to create RaytracerData outputStrategy for invalid process {Name:"
                    << data.GetProcessName() << "}";
                throw RmException("Failed to create RaytracerDataOutputStrategy, invalid <source> element name");
            }
            else if (!process_ptr->DoesRaytracerOutput())
            {
                LOG_ERROR << LOG_LINE << "Failed to create RaytracerData outputStrategy for process that does not perform Raytrace output {Name:"
                    << data.GetProcessName() << "}";
                throw RmException("Failed to create RaytracerDataOutputStrategy, invalid <source> element name");
            }

            RaytracerDataOutputStrategy::ProcessContainer processes;

            // If <specification1> == "all" , we draw everything
            // ie. No need to specify anything, as processes are all drawn by default
            const std::string& data_spec_1 = data.GetSpecification1();
            if ((!data_spec_1.empty()) && ("all" != data_spec_1))
            {
                std::vector<std::string> process_names;
                ParseCommaSeparatedStrings(data_spec_1.c_str(), process_names);

                for (std::vector<std::string>::iterator iter(process_names.begin());
                    iter != process_names.end(); ++iter)
                {
                    Process* proc = engine.GetProcessCoordinator().FindProcessByProcessName(*iter);
                    if (__nullptr != proc)
                    {
                        processes.insert(proc);
                    }
                }
                processes.insert(process_ptr);
            }
            else if ("all" == data_spec_1) //add all processes
            {
                boost::scoped_ptr<RaytracingProcessAdder> rpa(new RaytracingProcessAdder(processes));
                engine.GetProcessCoordinator().IterateOverProcesses(rpa.get());
            }
            else
            {
                processes.insert(process_ptr);
            }

            // Retrieve or calculate the total number of frames
            size_t frameCount = data.GetWhen().m_count;

            if (0 == frameCount) // "Repeat ad infinitum" - work out exactly how long that is
            {
                frameCount = OutputStrategy::CalculateNumberOfOutputEvents(data, engine);
            }

            bool useDF3 = false;
            wxString spec2 = data.GetSpecification2();
            if (spec2.IsSameAs("DF3", false)) { useDF3 = true; }

            // process_ptr may still be __nullptr; this is OK
            //issue109msg118
            OutputStrategy* ostrategy = new RaytracerDataOutputStrategy(process_ptr, engine, processes, frameCount, useDF3);
            LOG_INFO << LOG_LINE << "Created RaytracerData OutputStrategy for " << data.GetProcessName();

            return ostrategy;
        } // end if("RaytracerData"==typestring)

        else if ("NonSpatialData" == typestring)
        {
            // MSA Construct NonSpatialDataOutputStrategy

            // MSA 09.09.25 Dangerous bug found - calling GetValue(InvalidCharacteristicIndex) on a SharedAttribute
            // that is a ScoreboardSharedAttribute ends up negatively subscripting an array. This does NOT cause
            // a run-time (or compile-time, of course) error - it just returns garbage.
            // NonSpatialDataOutputStrategy checks to ensure its parameter SharedAttributes aren't ProcessSharedAttributes at construction (if applicable),
            // but other users of GetValue(InvalidCharacteristicIndex) are still at risk.

            Process* p = engine.GetProcessCoordinator().FindProcessByProcessName(data.GetProcessName());
            SharedAttributeSearchHelper searchHelper(&(engine.GetSharedAttributeManager()), SharedAttributeRegistrar::FindOwner(data.GetProcessName()));

            std::vector<std::string> variation_names;
            ParseCommaSeparatedStrings(data.GetSpecification1().c_str(), variation_names);

            std::vector<SharedAttribute*> sharedAttributesByVariation;

            if (variation_names.empty())
            {
                try
                {
                    sharedAttributesByVariation.push_back(searchHelper.SearchForAttribute(data.GetName().c_str()));
                }
                catch (RmException& /*ex*/)
                {
                    LOG_ERROR << "Could not locate a SharedAttribute for " << data.GetName().c_str();
                }
            }
            else
                for (size_t i = 0; i < variation_names.size(); ++i)
                {
                    try
                    {
                        CharacteristicIndex idx = searchHelper.SearchForClusterIndex(data.GetName().c_str(), variation_names[i].c_str(), __nullptr, true);

                        const SharedAttributeCluster& cluster = engine.GetSharedAttributeManager().GetCluster(variation_names[i].c_str());

                        // Effectively a "for each variation (Plant, RootOrder, etc) do..." loop
                        for (SharedAttributeCluster::const_iterator cluster_iter = cluster.begin();
                            cluster_iter != cluster.end();
                            ++cluster_iter
                            )
                        {
                            const SharedAttributeList& sa_list = (*cluster_iter).second;
                            sharedAttributesByVariation.push_back(sa_list[idx]);
                        }
                    }
                    catch (RmException& /*ex*/)
                    {
                        LOG_ERROR << "Could not locate a SharedAttributeCluster for " << variation_names[i];
                    }
                }

            if ((!sharedAttributesByVariation.empty()) && (0 != p))
            {
                // MSA filetype is determined by the filename extension.
                // $FILENAME.csv (case insensitive) will produce comma-delimited data;
                // any other extension will result in tab-delimited data.
                wxString filename = data.GetFileName();
                filename = filename.MakeUpper();
                wxString delim;
                if (filename.compare(filename.length() - 4, 4, ".CSV")) { delim = "\t"; }
                else { delim = ","; }
                OutputStrategy* ostrategy = new NonSpatialDataOutputStrategy(p, sharedAttributesByVariation, delim);
                LOG_INFO << LOG_LINE << "Created NonSpatialData OutputStrategy for " << data.GetProcessName();
                return ostrategy;
            }
            else
            {
                LOG_ERROR << "Failed to create NonSpatialDataOutputStrategy for " << data.GetProcessName() << ": Process and/or SharedAttribute not found";
            }
        } // end if("NonSpatialData"==typestring)

        else if ("ViewCapture" == typestring)
        {
            const std::string procName = data.GetProcessName();
            wxString name = data.GetFileName();
            const wxString& fileExt = name.AfterLast('.');
            const size_t viewIndex = atoi(data.GetSpecification1().c_str());

            if ("ViewCoordinator" == procName)
            {
                OutputStrategy* ostrategy = new ViewCaptureOutputStrategy(&engine, ViewCaptureOutputStrategy::VIEW_2D, viewIndex, fileExt);
                LOG_INFO << LOG_LINE << "Created ViewCapture OutputStrategy for " << procName;
                return ostrategy;
            }
            else if ("View3DCoordinator" == procName)
            {
                OutputStrategy* ostrategy = new ViewCaptureOutputStrategy(&engine, ViewCaptureOutputStrategy::VIEW_3D, viewIndex, fileExt);
                LOG_INFO << LOG_LINE << "Created ViewCapture OutputStrategy for " << procName;
                return ostrategy;
            }
            else
            {
                LOG_ERROR << "Failed to create ViewCaptureOutputStrategy for " << procName << " (neither ViewCoordinator nor View3DCoordinator)";
            }
        } // end if("ViewCapture"==typestring)
        else
        {
            LOG_WARN << LOG_LINE << "Output Strategy type string \"" << typestring << "\" not recognised";
        }
        return __nullptr;
    }


    const size_t OutputStrategy::CalculateNumberOfOutputEvents(const OutputRuleDAI& data, SimulationEngine& engine)
    {
        const ProcessTime_t runtime = engine.GetPostOffice().GetDefaultRunTime();
        const OutputRuleDAI::BaseWhen& when = data.GetWhen();

        size_t frameCount = 0;

        if (when.GetTypeName() == "Interval")
        {
            try
            {
                const OutputRuleDAI::IntervalWhen& iwhen = dynamic_cast<const OutputRuleDAI::IntervalWhen &>(when);
                ProcessTime_t temp = iwhen.m_initialTime;
                if (temp > runtime)
                {
                    RmAssert(false, "Error in constructing OutputStrategy");
                    return 0;
                }
                while (temp < runtime)
                {
                    ++frameCount;
                    temp += iwhen.m_interval; // Interval==0 has already been checked for in TXDataAccessManager::GenericElementFunction(DataOutputCoordinator * dcoord, TiXmlElement * element, const char * location)
                }
                return frameCount;
            }
            catch (const std::bad_cast&)
            {
                RmAssert(false, "Failed to cast BaseWhen instance that claimed to be 'Interval' to IntervalWhen");
                return 0;
            }
        }
        else if (when.GetTypeName() == "Regular")
        {
            try
            {
                const OutputRuleDAI::RegularWhen& rwhen = dynamic_cast<const OutputRuleDAI::RegularWhen &>(when);
                ProcessTime_t temp = rwhen.GetInitialTime();
                if (temp > runtime)
                {
                    RmAssert(false, "Error in constructing OutputStrategy");
                    return 0;
                }

                while (temp < runtime)
                {
                    ++frameCount;
                    switch (rwhen.m_regularity)
                    {
                    case OutputRuleDAI::RegularWhen::r_year: temp += PROCESS_YEARS(1);
                        break;
                    case OutputRuleDAI::RegularWhen::r_month: temp += PROCESS_MONTHS(1);
                        break;
                    case OutputRuleDAI::RegularWhen::r_day: temp += PROCESS_DAYS(1);
                        break;
                    case OutputRuleDAI::RegularWhen::r_hour: temp += PROCESS_HOURS(1);
                        break;
                    case OutputRuleDAI::RegularWhen::r_minute: temp += PROCESS_MINUTES(1);
                        break;
                    case OutputRuleDAI::RegularWhen::r_second: temp += 1;
                        break;
                    default:
                    {
                        RmAssert(false, "Error in constructing OutputStrategy");
                        return 0;
                    }
                    }
                }
                return frameCount;
            }
            catch (const std::bad_cast&)
            {
                RmAssert(false, "Failed to cast BaseWhen instance that claimed to be 'Regular' to RegularWhen");
                return 0;
            }
        }
        RmAssert(false, "Error in constructing OutputStrategy");
        return 0;
    }
} /* namespace rootmap */

