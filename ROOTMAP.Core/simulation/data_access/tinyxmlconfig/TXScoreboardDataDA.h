#ifndef TXScoreboardDataDA_H
#define TXScoreboardDataDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXScoreboardDataDA.h
// Purpose:     Declaration of the TXScoreboardDataDA and support
//              classes for 3, 2, 1 and 0-dimensional data.
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ScoreboardDataDAI.h"

#include <map>
#include <list>

namespace rootmap
{
    class TXScoreboardDataDA : public ScoreboardDataDAI
    {
    public:
        virtual ~TXScoreboardDataDA();

        const std::string& getProcessName() const;

        const std::string& getCharacteristicName() const;

        /**
         *
         * @param
         * @return
         */
        SchemeContentElementList& getElementList() const;


        TXScoreboardDataDA(const std::string& process_name,
            const std::string& characteristic_name,
            SchemeContentElementList& element_list);


    private:
        std::string m_processName;
        std::string m_characteristicName;
        SchemeContentElementList& m_elementList;

        // MSA 09.12.09 Private copy semantics to prevent copying. These methods are not properly implemented.
        TXScoreboardDataDA(const TXScoreboardDataDA& /*rhs*/);

        TXScoreboardDataDA& operator=(const TXScoreboardDataDA& /* rhs */)
        {
        };

        ///
        /// member declaration
    }; // class TXScoreboardDataDA
} /* namespace rootmap */

#endif // #ifndef TXScoreboardDataDA_H
