#include "simulation/process/raytrace/RaytracerData.h"
#include "wx/wfstream.h"
#include "wx/filename.h"
#include "core/common/Exceptions.h"

const static double CLOCK_MAX_FLAG = 1.0;
const static int MAX_CHARACTERISTICS = 10000;

namespace rootmap
{
    using Utility::CSMax;
    using Utility::CSMin;
    RootMapLoggerDefinition(RaytracerData);

    RaytracerData::RaytracerData(const bool& useDF3)
        : m_centrepoint()
        , m_clock(-1.0)
        , m_frameCount(0)
        , m_useDF3(useDF3)
        , m_isLittleEndian(Utility::IsLittleEndian())
        , m_df3IndexingWritten(false)
        , m_newFrameStarted(true)
        , m_thisFrameFinished(false)
        , m_rootsAllWritten(false)
        , m_anyGeometryWritten(false)
        , m_characteristicSummaries(__nullptr)
        , m_characteristicSummaryCount(0)
    {
        RootMapLoggerInitialisation("rootmap.RaytracerData");
        LOG_INFO << "RaytracerData constructor {default}";
        m_characteristicSummaries = new wxString*[MAX_CHARACTERISTICS];
    }

    RaytracerData::RaytracerData(const DoubleCoordinate& origin, const bool& useDF3)
        : m_centrepoint(origin)
        , m_clock(-1.0)
        , m_frameCount(0)
        , m_useDF3(useDF3)
        , m_isLittleEndian(Utility::IsLittleEndian())
        , m_df3IndexingWritten(false)
        , m_newFrameStarted(true)
        , m_thisFrameFinished(false)
        , m_rootsAllWritten(false)
        , m_anyGeometryWritten(false)
        , m_characteristicSummaries(__nullptr)
        , m_characteristicSummaryCount(0)
    {
        RootMapLoggerInitialisation("rootmap.RaytracerData");
        LOG_INFO << "RaytracerData constructor {CentrePoint:" << ToString(m_centrepoint) << "}";
        m_characteristicSummaries = new wxString*[MAX_CHARACTERISTICS];
    }


    RaytracerData::~RaytracerData()
    {
        for (std::list<root*>::iterator root_iter(m_rootSegments.begin());
            m_rootSegments.end() != root_iter;
            ++root_iter)
        {
            wxDELETE(*root_iter);
        }

        for (std::list<box*>::iterator sbox_iter(m_scoreboardBoxes.begin());
            m_scoreboardBoxes.end() != sbox_iter;
            ++sbox_iter)
        {
            wxDELETE(*sbox_iter);
        }

        for (std::list<nitrate*>::iterator npkt_iter(m_nitratePackets.begin());
            m_nitratePackets.end() != npkt_iter;
            ++npkt_iter)
        {
            wxDELETE(*npkt_iter);
        }

        for (std::list<DoubleBox*>::iterator rp_iter(m_rectangularPrisms.begin());
            m_rectangularPrisms.end() != rp_iter;
            ++rp_iter)
        {
            wxDELETE(*rp_iter);
        }
    }

    void RaytracerData::SetUsingDF3(const bool& b)
    {
        if (b == m_useDF3) return;
        LOG_WARN << "Switching DF3 usage to " << b << " and SB usage to " << !b;
        m_useDF3 = b;
    }

    void RaytracerData::AddBoundary(const Dimension& dim, const double& value)
    {
        switch (dim)
        {
        case X:
            m_xBoundaries.push_back(value);
            break;
        case Y:
            m_yBoundaries.push_back(value);
            break;
        case Z:
            m_zBoundaries.push_back(value);
            break;
        }
    }

    /**
    *    Writes a SCOREBOARD_BOX macro call to the POV-ray data file,
    *  along with an array of Characteristic values pertaining to this box.
    */
    void RaytracerData::AddScoreboardBox(const DoubleCoordinate& vec1, const DoubleCoordinate& vec2, const std::vector<CharacteristicValue>& chVec)
    {
        if (m_useDF3)
        {
            LOG_ERROR << "Attempted to use SB method for rendering Scoreboard Boxes on a RaytracerDataOutputStrategy configured to use the DF3 method";
            return;
        }
        if (m_baseCharacteristics.empty())
        {
            m_baseCharacteristics.assign(chVec.begin(), chVec.end());
        }
        box* bp = new(std::nothrow) box(vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z, chVec);
        if (0 != bp)
        {
            m_scoreboardBoxes.push_back(bp);
        }
    }

    /**
    *    Adds a pointer to a DF3 density field file
    */
    void RaytracerData::AddDensityField(const size_t& characteristicIndex, DF3Array* ptrDF3Array)
    {
        if (!m_useDF3)
        {
            LOG_ERROR << "Attempted to use SB method for rendering Scoreboard Boxes on a RaytracerDataOutputStrategy configured to use the DF3 method";
            return;
        }
        m_df3Volumes[characteristicIndex] = ptrDF3Array;
    }

    /**
    * Add root segment using raw root-order argument
    */
    void RaytracerData::AddRootSegment(const long& plantNumber, const unsigned long& rootOrder, const double& radius, const DoubleCoordinate& vec1, const DoubleCoordinate& vec2)
    {
        root* rp = new(std::nothrow) root(plantNumber, rootOrder, radius, vec1.x, vec1.y, vec1.z, vec2.x, vec2.y, vec2.z);
        if (0 != rp)
        {
            m_rootSegments.push_back(rp);
        }
    }

    void RaytracerData::AddNitrate(const DoubleCoordinate& pt, const double& s)
    {
        nitrate* np = new(std::nothrow) nitrate(pt.x, pt.y, pt.z, s);
        if (0 != np)
        {
            m_nitratePackets.push_back(np);
        }
    }

    void RaytracerData::AddWettingFront(const DoubleCoordinate& bottomLeft, const DoubleCoordinate& topRight, const wxColour& col)
    {
        front* wf = new(std::nothrow) front(bottomLeft.x, bottomLeft.y, topRight.x, topRight.y, topRight.z, col);
        if (0 != wf)
        {
            m_wettingFronts.push_back(wf);
        }
    }

    void RaytracerData::AddRectangularPrism(const DoubleBox& rectangularPrism)
    {
        // copy construct on the heap
        DoubleBox* db = new(std::nothrow) DoubleBox(rectangularPrism);
        if (0 != db)
        {
            m_rectangularPrisms.push_back(db);
        }
    }

    void RaytracerData::AddCylinder(const DoubleCoordinate& topFaceCentre, const double& radius, const double& height)
    {
        // construct on the heap
        Cylinder* c = new(std::nothrow) Cylinder(topFaceCentre, radius, height);
        if (0 != c)
        {
            m_cylinders.push_back(c);
        }
    }

    void RaytracerData::AddCharacteristicSummary(const wxString& str)
    {
        m_characteristicSummaryCount++;
        if (m_characteristicSummaryCount > MAX_CHARACTERISTICS)
        {
            throw new RmException("Too many characteristics to raytrace!");
        }
        m_characteristicSummaries[m_characteristicSummaryCount] = new wxString(str);
    }

    void RaytracerData::WriteFirstFrameDataToString()
    {
        // Write title/header and timestamp to file
        wxDateTime now(wxDateTime::Now());
        m_outputStr << now.Format("///////////////////////////////////////////////////////////////////////////////\n//                                                                           //\n//                       RootMap data for POVray 3.6                         //\n//                       Generated %H:%M:%S, %d/%m/%Y                      // \n//                                                                           //\n///////////////////////////////////////////////////////////////////////////////\n\n", wxDateTime::GMT8);
        m_outputStr << "#declare DATA_FRAME_COUNT = " << m_frameCount << ";    // Number of time-series frames in this data set\n\n";
        m_outputStr << "#declare CENTREPOINT = <" << m_centrepoint.x << ',' << m_centrepoint.y << ',' << m_centrepoint.z << ">;\n\n";

        // MSA 10.10.07 IMPORTANT NOTE:
        // If Scoreboard Characteristics are being output, then by definition they are present at every frame.
        // Hence putting this vital-to-characteristic-rendering information in the first-frame block presents no potential problems.
        const size_t cncount = m_characteristicSummaryCount;
        if (cncount > 0)
        {
            // We don't need to keep these Characteristic summaries around after they're written to string,
            // so we'll delete them as we go.
            wxString* firstString = m_characteristicSummaries[0];
            m_outputStr << "#declare CHARACTERISTIC_SUMMARIES = array[" << cncount << "] {\"" << firstString->c_str();
            delete firstString;
            m_characteristicSummaries[0] = __nullptr;
            size_t i = 1;
            for (size_t i = 1; i < cncount; i++)
            {
                wxString* thisString = m_characteristicSummaries[i];
                m_outputStr << "\"\n,\"" << thisString->c_str();
                delete thisString;
                m_characteristicSummaries[i] = __nullptr;
            }
            m_outputStr << "\"}\n\n// This macro call finds the indices for cyan, magenta and yellow (Process, Characteristic)s by name\nSET_CHARACTERISTIC_INDICES(CHARACTERISTIC_SUMMARIES)\n\n";

            m_characteristicSummaryCount = 0;
        }

        if (!m_xBoundaries.empty() && !m_yBoundaries.empty() && !m_zBoundaries.empty())
        {
            m_anyGeometryWritten = true;
            // Draw a narrow cylinder along each boundary
            m_outputStr << "#if(DRAW_SCOREBOARD_GRID)\nunion {\n";
            size_t newliner = 0;
            for (size_t iz = 0; iz < m_zBoundaries.size(); ++iz)
            {
                for (size_t iy = 0; iy < m_yBoundaries.size(); ++iy)
                {
                    // Horizontal Y-parallel lines
                    m_outputStr << "GL(" << m_minpoint.x << "," << m_yBoundaries[iy];
                    m_outputStr << "," << m_zBoundaries[iz] << "," << m_maxpoint.x << ",";
                    m_outputStr << m_yBoundaries[iy] << "," << m_zBoundaries[iz] << ")\t";
                    if (++newliner % 6 == 0) m_outputStr << "\n";

                    for (size_t ix = 0; ix < m_xBoundaries.size(); ++ix)
                    {
                        if (iy == 0)
                        {
                            // Horizontal X-parallel lines
                            m_outputStr << "GL(" << m_xBoundaries[ix] << "," << m_minpoint.y;
                            m_outputStr << "," << m_zBoundaries[iz] << "," << m_xBoundaries[ix] << ",";
                            m_outputStr << m_maxpoint.y << "," << m_zBoundaries[iz] << ")\t";
                            if (++newliner % 6 == 0) m_outputStr << "\n";
                        }
                        if (iz == 0)
                        {
                            // Vertical lines
                            m_outputStr << "GL(" << m_xBoundaries[ix] << "," << m_yBoundaries[iy];
                            m_outputStr << "," << m_minpoint.z << "," << m_xBoundaries[ix] << ",";
                            m_outputStr << m_yBoundaries[iy] << "," << m_maxpoint.z << ")\t";
                            if (++newliner % 6 == 0) m_outputStr << "\n";
                        }
                    }
                }
            }
            m_outputStr << "\npigment { GRID_COLOUR }\n}\n#end    // End of Scoreboard grid\n\n";
        }

        if (!m_rectangularPrisms.empty())
        {
            m_anyGeometryWritten = true;
            // Draw an RP for each RectangularPrism
            m_outputStr << "#if(DRAW_RECTANGULAR_PRISMS)\nunion {\n";
            for (std::list<DoubleBox*>::iterator iter = m_rectangularPrisms.begin(); iter != m_rectangularPrisms.end(); ++iter)
            {
                m_outputStr << "RP(" << (*iter)->left << "," << (*iter)->front << "," << (*iter)->top << "," << (*iter)->right << "," << (*iter)->back << "," << (*iter)->bottom << ")\n";
                // Don't need to keep this rectangular prism geometry around any more.
                wxDELETE(*iter);
            }
            m_outputStr << "\npigment { RECTANGULAR_PRISM_COLOUR }\n}\n#end    // End of Rectangular Prisms\n\n";
        }

        if (!m_cylinders.empty())
        {
            m_anyGeometryWritten = true;
            // Draw a C for each BoundingCylinder
            m_outputStr << "#if(DRAW_CYLINDERS)\nunion {\n";
            for (std::list<Cylinder*>::iterator iter = m_cylinders.begin(); iter != m_cylinders.end(); ++iter)
            {
                m_outputStr << "C(" << (*iter)->topCentreX << "," << (*iter)->topCentreY << "," << (*iter)->topCentreZ << "," << (*iter)->bottomCentreX << "," << (*iter)->bottomCentreY << "," << (*iter)->bottomCentreZ << "," << (*iter)->radius << ")\n";
                // Don't need to keep this cylinder geometry around any more.
                wxDELETE(*iter);
            }
            m_outputStr << "\npigment { CYLINDER_COLOUR }\n}\n#end    // End of Cylinders\n\n";
        }

        // Write first frame's time-dependent data; remember the POV-ray clock is 0.0 for the first frame
        m_firstFrameLastSection.assign("");
        if (m_frameCount > 1)
        {
            static const char* highPrecisionFormat = "%.12f";

            m_outputStr << "// Begin time-dependent data\n#if(ANIMATION_CLOCK<" << wxString::Format(highPrecisionFormat, m_clockInterval) << ")\n\t";

            // MSA 11.07.14 The frame's general data goes here.
            // Hence we put everything else in another string, which will be written to the main output string
            // once the general data is all done.


            m_firstFrameLastSection << "\n#end\t// End of this frame's data\n";

            // Write time-dependent inclusion of all the other frames
            m_firstFrameLastSection << "\n// Listing of all subsequent frames, if any:\n\n";
            size_t currFrame = 1;
            double workingClockMin = 0;
            while (currFrame < m_frameCount)
            {
                wxString fileStr = "#include ";
                wxString name = wxString::Format(m_filenameNumberFormat, currFrame);
                name.Replace("\\", "\\\\");
                if (m_filenameNumberFormat.Contains(":"))
                {
                    fileStr << "\"" << name << ".inc\"\n";
                }
                else
                {
                    fileStr << "concat(HOME_DIRECTORY, \"";
                    fileStr << name << ".inc\")\n";
                }

                m_firstFrameLastSection << "#if(ANIMATION_CLOCK>" << wxString::Format(highPrecisionFormat, workingClockMin) << ")\n";
                if (currFrame < m_frameCount - 1) // Remember last frame has no max_clock condition
                {
                    m_firstFrameLastSection << "\t#if(ANIMATION_CLOCK<=" << wxString::Format(highPrecisionFormat, workingClockMin + m_clockInterval);
                    m_firstFrameLastSection << ")\n\t\t" << fileStr << "\t#end\n#end\n";
                }
                else m_firstFrameLastSection << "\t" << fileStr << "#end\n";
                workingClockMin += m_clockInterval;
                ++currFrame;
            }
        }
        // Note that if this is a one-frame simulation, there are NO applicable clock conditions
    }

    bool RaytracerData::WriteDataSegmentToString(const size_t frameNum, bool & placeholderAdded, const size_t approximateSegmentSize/* = 0xFFFFF*/)
    {
        if (!m_rootSegments.empty() && !m_rootsAllWritten)
        {
            m_anyGeometryWritten = true;
            if (m_iterToNextRoot == m_rootSegments.begin()) m_outputStr << "#if(DRAW_ROOT_SYSTEM)\n";
            size_t newliner = 0;
            for (; m_rootSegments.end() != m_iterToNextRoot; ++m_iterToNextRoot)
            {
                if (m_outputStr.size() >= approximateSegmentSize) return false;

                (*m_iterToNextRoot)->appendTo(m_outputStr);
                // Note: Do not delete root segments;
                // they are added as they grow and NOT
                // renewed every frame like the others.
                if (++newliner % 2 == 0) m_outputStr << "\n";
            }
            m_outputStr << "\n#end  // End of root cylinders\n\n";
            m_rootsAllWritten = true;
        }

        if (m_useDF3 && !m_df3Volumes.empty() && !m_df3IndexingWritten)
        {
            m_anyGeometryWritten = true;
            // Write all this stuff, regardless of the requested segment size.
            m_outputStr << "#if(DRAW_SCOREBOARD_BOXES)\n\nRV(";
            m_outputStr << m_minpoint.x << "," << m_minpoint.y << "," << m_minpoint.z << "," << m_maxpoint.x << "," << m_maxpoint.y << "," << m_maxpoint.z << ",";
            wxString str = "concat(HOME_DIRECTORY,\"";
            str << wxString::Format(m_filenameNumberFormat, frameNum);
            wxChar sep = wxFileName::GetPathSeparator();
            str << sep;
            if (sep == 92) str << sep; // Need to use double path separator if it's backslash (ASCII 92). Yep, ugly kludge, this
            str << "ProcessCharacteristic_\",str(";
            m_outputStr << str << "CYAN_CHARACTERISTIC_INDEX,0,0),\".df3\"),\n\t\t";
            m_outputStr << str << "MAGENTA_CHARACTERISTIC_INDEX,0,0),\".df3\"),\n\t\t";
            m_outputStr << str << "YELLOW_CHARACTERISTIC_INDEX,0,0),\".df3\"))\n";
            m_outputStr << "#end  // End of Scoreboard data\n\n";
            m_baseCharacteristics.clear();
            m_df3IndexingWritten = true;
        }
        else if (!m_useDF3 && !m_scoreboardBoxes.empty())
        {
            m_anyGeometryWritten = true;
            if (m_iterToNextBox == m_scoreboardBoxes.begin())
            {
                std::list<CharacteristicValue>::iterator iter = m_baseCharacteristics.begin();
                m_outputStr << "#if(DRAW_SCOREBOARD_BOXES)\n\n#declare baseCA = array[" << m_baseCharacteristics.size() << "] {" << *iter;
                for (; m_baseCharacteristics.end() != iter; ++iter)
                {
                    m_outputStr << ',' << *iter;
                }
                m_outputStr << "}\n\n";
            }
            for (; m_scoreboardBoxes.end() != m_iterToNextBox; ++m_iterToNextBox)
            {
                if (m_outputStr.size() >= approximateSegmentSize) return false;

                (*m_iterToNextBox)->appendTo(m_outputStr, m_baseCharacteristics);
                wxDELETE(*m_iterToNextBox);
            }
            m_outputStr << "#end  // End of Scoreboard data\n\n";
            m_baseCharacteristics.clear();
        }

        if (!m_nitratePackets.empty())
        {
            m_anyGeometryWritten = true;
            if (m_iterToNextNitrate == m_nitratePackets.begin()) m_outputStr << "#if(DRAW_NITRATE_PACKETS)\n";
            size_t newliner = 0;
            for (; m_nitratePackets.end() != m_iterToNextNitrate; ++m_iterToNextNitrate)
            {
                if (m_outputStr.size() >= approximateSegmentSize) return false;

                (*m_iterToNextNitrate)->appendTo(m_outputStr);
                if (++newliner % 3 == 0) m_outputStr << "\n";
            }
            m_outputStr << "\n#end  // End of nitrate packets\n\n";

            // Delete nitrate packets, once they're done with.
            m_iterToNextNitrate = m_nitratePackets.begin();
            for (; m_nitratePackets.end() != m_iterToNextNitrate; ++m_iterToNextNitrate)
            {
                wxDELETE(*m_iterToNextNitrate);
            }
        }

        if (!m_wettingFronts.empty())
        {
            m_anyGeometryWritten = true;
            if (m_iterToNextWettingFront == m_wettingFronts.begin()) m_outputStr << "#if(DRAW_WETTING_FRONT)\n";
            size_t newliner = 0;
            for (; m_wettingFronts.end() != m_iterToNextWettingFront; ++m_iterToNextWettingFront)
            {
                if (m_outputStr.size() >= approximateSegmentSize) return false;

                (*m_iterToNextWettingFront)->appendTo(m_outputStr);
                wxDELETE(*m_iterToNextWettingFront);
                if (++newliner % 2 == 0) m_outputStr << "\n";
            }
            m_outputStr << "\n#end  // End of wetting fronts\n\n";
        }

        // If we are here, this is the last effective call to this function for this frame.

        // Erase all box, nitrate and wetting front pointers, which now point to freed memory.
        m_scoreboardBoxes.clear();
        m_nitratePackets.clear();
        m_wettingFronts.clear();

        // If nothing else is creating some geometry to be rendered,
        // create a placeholder object.
        if (!m_anyGeometryWritten)
        {
            // Placeholder object so that the frame is not empty. This should avoid the "Parse Warning: Degenerate CSG bounding box (not used!)." problem
            m_outputStr << "\nsphere { <-999,-999,-999> 1 }\n    texture {\n    pigment { color Yellow }\n}\n\n";
            placeholderAdded = true;
        }

        return true;
    }

    std::string RaytracerData::ToString(const DoubleCoordinate& coord)
    {
        std::string outstr = "{x=";
        outstr += Utility::ToString(coord.x).c_str();
        outstr += ", y=";
        outstr += Utility::ToString(coord.y).c_str();
        outstr += ", z=";
        outstr += Utility::ToString(coord.z).c_str();
        outstr += "}";
        return outstr;
    }

    /**
    * Private internal method for preparing the output string
    */
    void RaytracerData::WriteDataToString(const size_t frameNum)
    {
        if (!m_rootSegments.empty())
        {
            m_outputStr << "#if(DRAW_ROOT_SYSTEM)\n";
            size_t newliner = 0;
            for (std::list<root*>::iterator root_iter(m_rootSegments.begin());
                m_rootSegments.end() != root_iter;
                ++root_iter)
            {
                (*root_iter)->appendTo(m_outputStr);
                // Note: Do not delete root segments;
                // they are added as they grow and NOT
                // renewed every frame like the others.
                if (++newliner % 2 == 0) m_outputStr << "\n";
            }
            m_outputStr << "\n#end  // End of root cylinders\n\n";
        }

        if (m_useDF3 && !m_df3Volumes.empty())
        {
            m_outputStr << "#if(DRAW_SCOREBOARD_BOXES)\n\nRV(";
            m_outputStr << m_minpoint.x << "," << m_minpoint.y << "," << m_minpoint.z << "," << m_maxpoint.x << "," << m_maxpoint.y << "," << m_maxpoint.z << ",";
            wxString str = "concat(HOME_DIRECTORY,\"";
            str << wxString::Format(m_filenameNumberFormat, frameNum);
            wxChar sep = wxFileName::GetPathSeparator();
            str << sep;
            if (sep == 92) str << sep; // Need to use double path separator if it's backslash (ASCII 92). Yep, ugly kludge, this
            str << "ProcessCharacteristic_\",str(";
            m_outputStr << str << "CYAN_CHARACTERISTIC_INDEX,0,0),\".df3\"),\n\t\t";
            m_outputStr << str << "MAGENTA_CHARACTERISTIC_INDEX,0,0),\".df3\"),\n\t\t";
            m_outputStr << str << "YELLOW_CHARACTERISTIC_INDEX,0,0),\".df3\"))\n";
            m_outputStr << "#end  // End of Scoreboard data\n\n";
            m_baseCharacteristics.clear();
        }
        else if (!m_useDF3 && !m_scoreboardBoxes.empty())
        {
            std::list<CharacteristicValue>::iterator iter = m_baseCharacteristics.begin();
            m_outputStr << "#if(DRAW_SCOREBOARD_BOXES)\n\n#declare baseCA = array[" << m_baseCharacteristics.size() << "] {" << *iter;
            for (; m_baseCharacteristics.end() != iter; ++iter)
            {
                m_outputStr << ',' << *iter;
            }
            m_outputStr << "}\n\n";
            for (std::list<box*>::iterator sbox_iter(m_scoreboardBoxes.begin());
                m_scoreboardBoxes.end() != sbox_iter;
                ++sbox_iter)
            {
                (*sbox_iter)->appendTo(m_outputStr, m_baseCharacteristics);
                wxDELETE(*sbox_iter);
            }
            m_outputStr << "#end  // End of Scoreboard data\n\n";
            m_baseCharacteristics.clear();
        }

        if (!m_nitratePackets.empty())
        {
            m_outputStr << "#if(DRAW_NITRATE_PACKETS)\n";
            size_t newliner = 0;
            for (std::list<nitrate*>::iterator npkt_iter(m_nitratePackets.begin());
                m_nitratePackets.end() != npkt_iter;
                ++npkt_iter)
            {
                (*npkt_iter)->appendTo(m_outputStr);
                wxDELETE(*npkt_iter);
                if (++newliner % 3 == 0) m_outputStr << "\n";
            }
            m_outputStr << "\n#end  // End of nitrate packets\n\n";
        }

        if (!m_wettingFronts.empty())
        {
            m_outputStr << "#if(DRAW_WETTING_FRONT)\n";
            size_t newliner = 0;
            for (std::list<front*>::iterator wf_iter(m_wettingFronts.begin());
                m_wettingFronts.end() != wf_iter;
                ++wf_iter)
            {
                (*wf_iter)->appendTo(m_outputStr);
                wxDELETE(*wf_iter);
                if (++newliner % 2 == 0) m_outputStr << "\n";
            }
            m_outputStr << "\n#end  // End of wetting fronts\n\n";
        }

        // Erase all box, nitrate and wetting front pointers, which now point to freed memory
        m_scoreboardBoxes.clear();
        m_nitratePackets.clear();
        m_wettingFronts.clear();
    }

    void RaytracerData::WriteDF3DataToFile(const size_t frameNum)
    {
        // Create directory name
        wxString dir = wxString::Format(m_filenameNumberFormat, frameNum);
        if (!wxFileName::DirExists(dir)) wxFileName::Mkdir(dir);

        std::map<size_t, DF3Array*>::iterator iter = m_df3Volumes.begin();
        while (iter != m_df3Volumes.end())
        {
            wxString fname(dir);
            fname << wxFileName::GetPathSeparator() << "ProcessCharacteristic_" << iter->first << ".df3";
            wxFFileOutputStream outputStream(fname, "wb+");
            if (!outputStream.IsOk())
            {
                LOG_ERROR << "Failed to open a direct file writing stream at "
                    << fname << ". Could not write DF3 volume data.";
            }
            else
            {
                unsigned short x = iter->second->xmax;
                unsigned short y = iter->second->ymax;
                unsigned short z = iter->second->zmax;

                if (sizeof(short) > 2)
                {
                    if (x > 0xFFFF)
                    {
                        LOG_ERROR << "Scoreboard X-axis size exceeds maximum resolution of DF3 volume. Truncating.";
                        x = 0xFFFF;
                    }
                    if (y > 0xFFFF)
                    {
                        LOG_ERROR << "Scoreboard Y-axis size exceeds maximum resolution of DF3 volume. Truncating.";
                        y = 0xFFFF;
                    }
                    if (z > 0xFFFF)
                    {
                        LOG_ERROR << "Scoreboard Z-axis size exceeds maximum resolution of DF3 volume. Truncating.";
                        z = 0xFFFF;
                    }
                    x &= 0xFFFF; // Clamp x, y, z to 2 bytes each
                    y &= 0xFFFF;
                    z &= 0xFFFF;
                }
                outputStream.PutC(x >> 8);
                outputStream.PutC(x & 0xFF);
                outputStream.PutC(y >> 8);
                outputStream.PutC(y & 0xFF);
                outputStream.PutC(z >> 8);
                outputStream.PutC(z & 0xFF);

                // Recall that in DF3 volumes, X varies fastest, then Y, then Z.
                for (unsigned short k = 0; k < z; ++k)
                {
                    for (unsigned short j = 0; j < y; ++j)
                    {
                        for (unsigned short i = 0; i < x; ++i)
                        {
                            Utility::DF3Voxel val = Utility::FixEndianness(iter->second->values[i][j][k], m_isLittleEndian);
                            if (sizeof(Utility::DF3Voxel) != outputStream.Write(&val, sizeof(Utility::DF3Voxel)).LastWrite())
                            {
                                LOG_ERROR << "Failed to correctly write DF3 voxel " << val << " to " << fname
                                    << " at (x,y,z) = (" << i << "," << j << "," << k << "). ";
                            }
                        }
                    }
                }
            }
            wxDELETE(iter->second); // Is this necessary, given that map::clear() calls destructor for each value?
            ++iter;
        } // while(iter!=m_df3Volumes.end())
        m_df3Volumes.clear();
    }

    const wxString& RaytracerData::GetNextFrameStringSegment(const size_t& frameNumber, const wxString& filename, bool & placeholderAdded)
    {
        // RvH Raytrace Output Performance
        // reserve 2MB of string in one hit
        static const size_t FRAMESTRING_RESERVE_SIZE = 2000000;
        m_outputStr.assign("");
        m_outputStr.reserve(FRAMESTRING_RESERVE_SIZE);

        if (m_newFrameStarted)
        {
            m_iterToNextRoot = m_rootSegments.begin();
            m_iterToNextBox = m_scoreboardBoxes.begin();
            m_iterToNextNitrate = m_nitratePackets.begin();
            m_iterToNextWettingFront = m_wettingFronts.begin();
            m_rootsAllWritten = false;

            if (frameNumber < 1) // Do first-frame setup
            {
                // Set up the frame filename format
                size_t i = 0;
                wxString prefix = filename;
                // Strip .inc file extension
                prefix.Remove(filename.Len() - 4);
                // Count and remove all trailing (serial) digits
                while (prefix.Right(1).IsNumber())
                {
                    prefix.RemoveLast();
                    ++i;
                }
                m_filenameNumberFormat = prefix;
                m_filenameNumberFormat << "%0" << i << "d";

                WriteFirstFrameDataToString();
            }

            m_newFrameStarted = false;
        } // if(m_newFrameStarted)    // i.e. if first segment of this frame

        // Get this frame's data
        bool allDone = WriteDataSegmentToString(frameNumber, placeholderAdded);

        if (frameNumber < 1 && allDone && !m_thisFrameFinished)
        {
            m_outputStr << m_firstFrameLastSection;
            m_thisFrameFinished = true;
        }

        // WriteDF3DataToFile automatically erases its data when complete,
        // so this conditional will only evaluate to true once per frame.
        if (!m_df3Volumes.empty()) WriteDF3DataToFile(frameNumber);

        return m_outputStr;
    }

    const wxString& RaytracerData::GetFrameString(const size_t& frameNumber, const wxString& filename)
    {
        // RvH Raytrace Output Performance
        // reserve 2MB of string in one hit
        static const size_t FRAMESTRING_RESERVE_SIZE = 2000000;
        m_outputStr.assign("");
        m_outputStr.reserve(FRAMESTRING_RESERVE_SIZE);

        if (frameNumber < 1) // Do first-frame setup
        {
            // Set up the frame filename format
            size_t i = 0;
            wxString prefix = filename;
            // Strip .inc file extension
            prefix.Remove(filename.Len() - 4);
            // Count and remove all trailing (serial) digits
            while (prefix.Right(1).IsNumber())
            {
                prefix.RemoveLast();
                ++i;
            }
            m_filenameNumberFormat = prefix;
            m_filenameNumberFormat << "%0" << i << "d";
            WriteFirstFrameDataToString();
        } // if(frameNumber<1)    // i.e. if first frame

        // Get this frame's data
        WriteDataToString(frameNumber);

        if (frameNumber < 1)
        {
            m_outputStr << m_firstFrameLastSection;
            m_thisFrameFinished = true;
        }

        if (!m_df3Volumes.empty()) WriteDF3DataToFile(frameNumber);

        return m_outputStr;
    }
} /* namespace rootmap */
