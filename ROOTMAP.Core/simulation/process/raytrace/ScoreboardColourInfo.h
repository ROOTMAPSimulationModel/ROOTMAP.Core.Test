#ifndef ScoreboardColourInfo_H
#define ScoreboardColourInfo_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardColourInfo.h
// Purpose:     Declaration of the ScoreboardColourInfo class
// Created:     29-06-2009 15:46:41
// Author:      RvH
// $Date$
// $Revision$
// Copyright:   ©2002-2009 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/process/interprocess_communication/Message.h"
#include "simulation/common/Types.h"

namespace rootmap
{
    class ScoreboardColourInfo : public SpecialProcessData
    {
    public:
        // Default constructor
        ScoreboardColourInfo();

        /**
         * "Copy"-ish constructor
         */
        ScoreboardColourInfo(const CharacteristicColours_t& rhs);

        ~ScoreboardColourInfo();

        void push_back(const CharacteristicColourInfo& cci);

        CharacteristicColourInfo& at(const size_t& index);

        const size_t size() const;

    private:
        std::vector<CharacteristicColourInfo> theData;
    }; // class ScoreboardColourInfo
} /* namespace rootmap */

#endif // #ifndef ScoreboardColourInfo_H
