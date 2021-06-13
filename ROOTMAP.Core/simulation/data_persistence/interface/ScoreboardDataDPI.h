#ifndef ScoreboardDataDPI_H
#define ScoreboardDataDPI_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardDataDPI.h
// Purpose:     Declaration of the ScoreboardDataDPI and support
//              classes for 3, 2, 1 and 0-dimensional data.
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/scoreboard/ScoreboardBox.h"
//#include "core/scoreboard/ScoreboardStratum.h"


#include <map>
#include <list>

namespace rootmap
{
    class ScoreboardDataDPI
    {
    public:
        virtual ~ScoreboardDataDPI()
        {
        }

        virtual void storeProcessName(const std::string& pname) const = 0;
        virtual void storeCharacteristicName(const std::string& cname) const = 0;

        virtual void storeData(CharacteristicValueArray values, size_t numberOfValues) = 0;

    protected:
        ScoreboardDataDPI()
        {
        }

    private:

        ///
        /// member declaration
    }; // class ScoreboardDataDPI
} /* namespace rootmap */

#endif // #ifndef ScoreboardDataDPI_H
