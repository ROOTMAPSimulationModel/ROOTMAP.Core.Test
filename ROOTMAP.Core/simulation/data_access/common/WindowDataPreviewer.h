#ifndef WindowDataPreviewer_H
#define WindowDataPreviewer_H
/////////////////////////////////////////////////////////////////////////////
// Name:        WindowDataPreviewer.h
// Purpose:     Declaration of the WindowDataPreviewer class
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataPreviewer.h"
#include <vector>
#include "wx/string.h"

namespace rootmap
{
    class WindowDataPreviewer : public DataPreviewer
    {
        friend class RmCWWindowsPage;
    public:

        WindowDataPreviewer();
        virtual ~WindowDataPreviewer();

        void ResetWindowIndex();
        const size_t windowCount() const;
        void ChangesHaveBeenMade();
        const bool DataChanged() const;

        class Window
        {
        public:
            bool m_useThisWindow;
        private:
            virtual void dummy() = 0;
        };

        class Table : public Window
        {
        public:
            wxString m_processName;
            wxString m_characteristicName;
            wxString m_viewDirection;
            wxString m_stratum;
            int m_layerNumber;
        private:
            virtual void dummy();
        };

        class TwoDView : public Window
        {
        public:
            wxString m_viewDirection;
            double m_referenceIndexX;
            double m_referenceIndexY;
            double m_referenceIndexZ;
            double m_scale;
            double m_zoomRatio;
            bool m_repeat;
            bool m_wrap;
            bool m_drawBoundaries;
            bool m_drawBoxes;
            bool m_drawBoxColours;
            wxString m_scoreboards;
            wxString m_processes;
            wxString m_cyanProcessName;
            wxString m_cyanCharacteristicName;
            unsigned char m_cyanColourMin;
            unsigned char m_cyanColourMax;
            double m_cyanCharacteristicMin;
            double m_cyanCharacteristicMax;
            wxString m_magentaProcessName;
            wxString m_magentaCharacteristicName;
            unsigned char m_magentaColourMin;
            unsigned char m_magentaColourMax;
            double m_magentaCharacteristicMin;
            double m_magentaCharacteristicMax;
            wxString m_yellowProcessName;
            wxString m_yellowCharacteristicName;
            unsigned char m_yellowColourMin;
            unsigned char m_yellowColourMax;
            double m_yellowCharacteristicMin;
            double m_yellowCharacteristicMax;
        private:
            virtual void dummy();
        };

        class ThreeDView : public Window
        {
        public:
            double m_viewPositionX;
            double m_viewPositionY;
            double m_viewPositionZ;
            double m_scale;
            bool m_drawBoundaries;
            bool m_drawBoxes;
            bool m_drawBoxColours;
            double m_rootRadiusMultiplier;
            bool m_rootColourByBranchOrder;
            bool m_highContrastRootColour;
            double m_baseRootRed;
            double m_baseRootGreen;
            double m_baseRootBlue;
            wxString m_quality;
            wxString m_scoreboards;
            wxString m_processes;
        private:
            virtual void dummy();
        };

        void AddTable(Table* tbl);
        void Add2DView(TwoDView* twoD);
        void Add3DView(ThreeDView* threeD);
        void RemoveLastTable();
        void RemoveLast2DView();
        void RemoveLast3DView();
        const bool HasMoreWDPWindows() const;
        Window* GetNextWDPWindow();

    protected:

        std::vector<Table*> m_tables;
        std::vector<TwoDView*> m_2DViews;
        std::vector<ThreeDView*> m_3DViews;

    private:

        size_t m_nextTableIndex;
        size_t m_next2DViewIndex;
        size_t m_next3DViewIndex;
        bool m_dataChanged;
    };

    inline void WindowDataPreviewer::ResetWindowIndex()
    {
        m_nextTableIndex = 0;
        m_next2DViewIndex = 0;
        m_next3DViewIndex = 0;
    }

    inline const size_t WindowDataPreviewer::windowCount() const
    {
        return (m_tables.size() + m_2DViews.size() + m_3DViews.size());
    }

    inline const bool WindowDataPreviewer::HasMoreWDPWindows() const
    {
        return (m_nextTableIndex < m_tables.size())
            || (m_next2DViewIndex < m_2DViews.size())
            || (m_next3DViewIndex < m_3DViews.size());
    }

    inline void WindowDataPreviewer::RemoveLastTable()
    {
        std::vector<Table*>::const_iterator i = m_tables.end() - 1;
        delete (*i);
        m_tables.erase(i);
    }

    inline void WindowDataPreviewer::RemoveLast2DView()
    {
        std::vector<TwoDView*>::const_iterator i = m_2DViews.end() - 1;
        delete (*i);
        m_2DViews.erase(i);
    }

    inline void WindowDataPreviewer::RemoveLast3DView()
    {
        std::vector<ThreeDView*>::const_iterator i = m_3DViews.end() - 1;
        delete (*i);
        m_3DViews.erase(i);
    }

    inline void WindowDataPreviewer::ChangesHaveBeenMade()
    {
        m_dataChanged = true;
    }

    inline const bool WindowDataPreviewer::DataChanged() const
    {
        return m_dataChanged;
    }
} /* namespace rootmap */


#endif // #ifndef WindowDataPreviewer_H
