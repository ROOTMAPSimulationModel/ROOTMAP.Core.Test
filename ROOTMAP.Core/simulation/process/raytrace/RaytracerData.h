#ifndef RaytracerData_H
#define RaytracerData_H

#include "core/common/DoubleCoordinates.h"
#include "core/scoreboard/ScoreboardBox.h"

namespace rootmap
{
    class RaytracerData
    {
    public:

        RaytracerData(const bool& useDF3);
        RaytracerData(const DoubleCoordinate& origin, const bool& useDF3);
        ~RaytracerData();

        /**
         * Gets the output string, for writing to file.
         * Note that frameNumber count begins from 1, not zero
         * (though, confusingly, the files themselves are named starting from zero)
         */
        const wxString& GetFrameString(const size_t& frameNumber, const wxString& filename);

        /**
         *    Segmented version of the above, used for optimisation (to avoid having very large strings to handle)
         *
         */
        const wxString& GetNextFrameStringSegment(const size_t& frameNumber, const wxString& filename, bool & placeholderAdded);

        void AddBoundary(const Dimension& dim, const double& value);

        /**
         *    Writes a SB (scoreboard box) macro call to the POV-ray data file,
         *  along with an array of Characteristic values pertaining to this box.
         */
        void AddScoreboardBox(const DoubleCoordinate& vec1, const DoubleCoordinate& vec2, const std::vector<CharacteristicValue>& chVec);

        void AddDensityField(const size_t& characteristicIndex, DF3Array* theArray);

        /**
         * Add root segment using raw root-order argument
         */
        void AddRootSegment(const long& plantNumber, const unsigned long& rootOrder, const double& radius, const DoubleCoordinate& vec1, const DoubleCoordinate& vec2);

        void AddNitrate(const DoubleCoordinate& pt, const double& s);

        /**
         *    Add a wetting front rectangle. The two coordinates have the same z-value, as a ScoreboardBox's wetting front
         *  is horizontally uniform. Z-values may vary between ScoreboardBoxes, of course.
         */
        void AddWettingFront(const DoubleCoordinate& leftFront, const DoubleCoordinate& rightBack, const wxColour& col);

        /**
         *    Add a rectangular prism. This method is intended to represent a BoundingRectangularPrism.
         *    As such, the resulting rectangular prism is aligned with the Z-axis.
         */
        void AddRectangularPrism(const DoubleBox& rectangularPrism);

        /**
         *    Add a cylinder. This method is intended to represent a BoundingCylinder.
         *    As such, the resulting cylinder is aligned with the Z-axis.
         */
        void AddCylinder(const DoubleCoordinate& topFaceCentre, const double& radius, const double& height);

        void AddCharacteristicSummary(const wxString& summaryString);

        void SetMinMaxPoints(const DoubleCoordinate& minpt, const DoubleCoordinate& maxpt);

        void SetFrameCount(const size_t& count);

        void StartNextFrame();

        const bool& IsUsingDF3() const;

        void SetUsingDF3(const bool& b);

    private:
        RootMapLoggerDeclaration();


        std::string ToString(const DoubleCoordinate& coord);

        void WriteDataToString(const size_t frameNum);
        // Important note about approximateSegmentSize: it will usually be exceeded, but not by much.
        // Boolean return value = has all data been written?
        bool WriteDataSegmentToString(const size_t frameNum, bool & placeholderAdded, const size_t approximateSegmentSize = 0xFFFFF);
        void WriteFirstFrameDataToString();
        void WriteDF3DataToFile(const size_t frameNum);

        struct root
        {
        private:
            const double x1, y1, z1, x2, y2, z2, radius;
            const unsigned long order;
            const long plantNum;
        public:
            root(const long& plantNumber, const unsigned long& o, const double& rad, const double& xx1, const double& yy1, const double& zz1, const double& xx2, const double& yy2, const double& zz2) :
                x1(xx1), y1(yy1), z1(zz1), x2(xx2), y2(yy2), z2(zz2), order(o), plantNum(plantNumber), radius(rad)
            {
            }

            void appendTo(wxString& str)
            {
                char buf[256];
                sprintf_s(buf, 256, "RS(%d,%u,%f,%f,%f,%f,%f,%f,%f)\t", plantNum, order, radius, x1, y1, z1, x2, y2, z2);
                str << buf;
            }
        };

        struct Cylinder
        {
            const double topCentreX, topCentreY, topCentreZ, bottomCentreX, bottomCentreY, bottomCentreZ, radius;

            Cylinder(const DoubleCoordinate& tfc, const double& r, const double& height) :
                topCentreX(tfc.x), topCentreY(tfc.y), topCentreZ(tfc.z), bottomCentreX(tfc.x), bottomCentreY(tfc.y), bottomCentreZ(tfc.z + height), radius(r)
            {
            }
        };

        // MSA Modifying the POV-ray ScoreboardBox Characteristic handling.
        // OLD: Array for each SB with a majority of redundant elements
        // NEW: Base reference array for each include file
        // and only the variations for each SB recorded
        struct box
        {
            std::list<CharacteristicValue> chVals;
            const double x1, y1, z1, x2, y2, z2;
            unsigned long arrayID;

            struct pair
            {
                double val;
                int idx;

                pair(const int i, const double d) : idx(i), val(d)
                {
                }
            };

        public:
            box(const double& xx1, const double& yy1, const double& zz1, const double& xx2, const double& yy2, const double& zz2, const std::vector<CharacteristicValue>& chV) :
                x1(xx1), y1(yy1), z1(zz1), x2(xx2), y2(yy2), z2(zz2), arrayID(0)
            {
                static unsigned long globalCount = 0; // Shared across all RaytracerDataOutputRules. Shouldn't matter (actual number unimportant, just needs to be unique).
                arrayID = globalCount++;
                chVals.assign(chV.begin(), chV.end());
            }

            void appendTo(wxString& str, const std::list<CharacteristicValue>& comparisonList)
            {
                RmAssert((chVals.size() == comparisonList.size()), "Number of unique Process/Characteristic pairs has somehow changed during simulation");
                const size_t sz = chVals.size();
                std::list<pair> pairs;
                std::list<CharacteristicValue>::iterator iter = chVals.begin();
                std::list<CharacteristicValue>::const_iterator iter2 = comparisonList.begin();
                int index = 0;
                for (; chVals.end() != iter; ++iter, ++iter2)
                {
                    // OK to use inequality operator on doubles in this instance
                    // because unchanged values will come from exactly the same sequence of operations
                    if (*iter != *iter2)
                    {
                        pairs.push_back(pair(index, *iter));
                    }
                    ++index;
                }
                // Heuristic: mean approx. 25 characters per variant Characteristic,
                // mean approx. 3 characters per Characteristic if whole array is specified.
                // Therefore, if more than a few percent of this SB's array
                // vary from the base array, it's more efficient to just write the whole array again.
                if ((25 * pairs.size()) > (3 * comparisonList.size()))
                {
                    iter = chVals.begin();
                    str << "#local ca" << arrayID << " = array[" << sz << "] {" << *iter;
                    for (; chVals.end() != iter; ++iter)
                    {
                        str << ',' << *iter;
                    }
                    str << "}\n";
                }
                else
                {
                    str << "#local ca" << arrayID << "=baseCA;\n";
                    for (std::list<pair>::iterator pairIter = pairs.begin();
                        pairs.end() != pairIter; ++pairIter)
                    {
                        str << "#local ca" << arrayID << '[' << pairIter->idx << "]=" << pairIter->val << ";\n";
                    }
                }
                char buf[128];
                sprintf_s(buf, 128, "SB(%f,%f,%f,%f,%f,%f,ca%u)\n", x1, y1, z1, x2, y2, z2, arrayID);
                str << buf;
            }
        };

        struct nitrate
        {
            const double x1, y1, z1, s;
        public:
            nitrate(const double& x, const double& y, const double& z, const double& ss) :
                x1(x), y1(y), z1(z), s(ss)
            {
            }

            void appendTo(wxString& str)
            {
                char buf[64];
                sprintf_s(buf, 64, "NP(%f,%f,%f,%f)\t", x1, y1, z1, s);
                str << buf;
            }
        };

        struct front
        {
            const double x1, y1, x2, y2, z;
            unsigned char colour[4];
        public:
            front(const double& xx1, const double& yy1, const double& xx2, const double& yy2, const double& zz, const wxColour& col) :
                x1(xx1), y1(yy1), x2(xx2), y2(yy2), z(zz)
            {
                colour[0] = col.Red();
                colour[1] = col.Green();
                colour[2] = col.Blue();
                colour[3] = col.Alpha();
            }

            void appendTo(wxString& str)
            {
                char buf[192];
                sprintf_s(buf, 192, "WF(%f,%f,%f,%f,%f,%f,%f,%f,%f)\t", x1, y1, x2, y2, z, colour[0] / 255.0, colour[1] / 255.0, colour[2] / 255.0, colour[3] / 255.0);
                str << buf;
            }
        };

        std::list<CharacteristicValue> m_baseCharacteristics;

        DoubleCoordinate m_minpoint, m_centrepoint, m_maxpoint;

        std::vector<double> m_xBoundaries;
        std::vector<double> m_yBoundaries;
        std::vector<double> m_zBoundaries;
        std::list<root*> m_rootSegments;
        std::list<box*> m_scoreboardBoxes;
        std::list<nitrate*> m_nitratePackets;
        std::list<front*> m_wettingFronts;
        std::list<DoubleBox*> m_rectangularPrisms;
        std::list<Cylinder*> m_cylinders;
        wxString** m_characteristicSummaries;
        size_t m_characteristicSummaryCount;
        std::map<size_t, DF3Array*> m_df3Volumes;

        std::list<root*>::iterator m_iterToNextRoot;
        std::list<box*>::iterator m_iterToNextBox;
        std::list<nitrate*>::iterator m_iterToNextNitrate;
        std::list<front*>::iterator m_iterToNextWettingFront;

        // Filename helper string
        wxString m_filenameNumberFormat;

        // The data string itself
        wxString m_outputStr;

        // Another helper string
        wxString m_firstFrameLastSection;

        // Private animation timing variables
        double m_clock;
        double m_clockInterval;
        size_t m_frameCount;

        bool m_useDF3;
        bool m_isLittleEndian;
        bool m_newFrameStarted;
        bool m_thisFrameFinished;
        bool m_df3IndexingWritten;
        // Need a flag for this, because roots aren't erased each step.
        bool m_rootsAllWritten;
        // And this flag is facilitate a POV-Ray optimisation.
        bool m_anyGeometryWritten;
    };

    inline void RaytracerData::SetMinMaxPoints(const DoubleCoordinate& minpt, const DoubleCoordinate& maxpt)
    {
        m_minpoint = minpt;
        m_maxpoint = maxpt;
        DoubleCoordinate dc = DoubleCoordinate();
        if (m_centrepoint == dc)
        {
            m_centrepoint = DoubleCoordinate(maxpt.x - minpt.x, maxpt.y - minpt.y, maxpt.z - minpt.z);
        }
        else
        {
            m_centrepoint.z = (maxpt.z - minpt.z) / 2;
        }
    }

    inline void RaytracerData::SetFrameCount(const size_t& count)
    {
        m_frameCount = count;
        // POV-ray uses a weird clock interval - begins at 0.0 for the first frame and goes
        // to 1.0 on the last frame. E.g. a 3-frame sequence will have clock values of 0.0, 0.5 and 1.0,
        // not the 0.333, 0.666, 0.999 you might expect.        
        m_clockInterval = 1.0 / (static_cast<double>(m_frameCount) - 1);
    }

    inline void RaytracerData::StartNextFrame()
    {
        // Clock is initialised to -1 as a first-hit flag
        if (m_clock < 0)
        {
            m_clock = 0.0;
        }
        else
        {
            m_clock += m_clockInterval;
        }
        m_newFrameStarted = true;
        m_thisFrameFinished = false;
        m_df3IndexingWritten = false;
    }

    inline const bool& RaytracerData::IsUsingDF3() const
    {
        return m_useDF3;
    }
} /* namespace rootmap */
#endif // #ifndef RaytracerData_H
