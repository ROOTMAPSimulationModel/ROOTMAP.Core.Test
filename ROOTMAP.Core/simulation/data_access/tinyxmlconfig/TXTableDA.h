#ifndef TXTableDA_H
#define TXTableDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXTableDA.h
// Purpose:     Declaration of the TXTableDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/TableDAI.h"

namespace rootmap
{
    class TXTableDA
        : public TableDAI
    {
    public:
        TXTableDA(const char* processName,
            const char* characteristicName,
            ViewDirection viewDir,
            long layer,
            const ScoreboardStratum& stratum);
        ~TXTableDA();

        std::string getProcessName() const;
        std::string getCharacteristicName() const;
        ViewDirection getViewDirection() const;
        long getLayer() const;
        ScoreboardStratum getScoreboardStratum() const;

    private:
        std::string m_processName;
        std::string m_characteristicName;
        ViewDirection m_viewDirection;
        long m_layer;
        ScoreboardStratum m_stratum;
    }; // class TXTableDA
} /* namespace rootmap */

#endif // #ifndef TXTableDA_H
