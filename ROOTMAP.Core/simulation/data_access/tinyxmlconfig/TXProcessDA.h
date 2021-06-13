#ifndef TXProcessDA_H
#define TXProcessDA_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXProcessDA.h
// Purpose:     Declaration of the TXProcessDA class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/interface/ProcessDAI.h"

namespace rootmap
{
    class TXCharacteristicDA;

    class TXProcessDA : public virtual ProcessDAI
    {
    public:
        virtual ProcessIdentifier getIdentifier() const;
        virtual std::string getName() const;
        virtual ScoreboardStratum getStratum() const;
        virtual ProcessActivity getActivity() const;
        virtual bool doesOverride() const;

        virtual const CharacteristicDAICollection& getCharacteristics() const;

        virtual ~TXProcessDA();

        /**
         * Takes ownership of the CharacteristicDA
         */
        void addCharacteristicDA(TXCharacteristicDA* characteristicDA);

        TXProcessDA(const std::string& name,
            bool doesOverride,
            ProcessIdentifier identifier,
            ScoreboardStratum stratum,
            ProcessActivity activity);
        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        bool isValid() const;

    private:

        ///
        /// member declaration
        std::string m_name;
        ProcessIdentifier m_identifier;
        ScoreboardStratum m_stratum;
        ProcessActivity m_activity;
        bool m_doesOverride;

        CharacteristicDAICollection m_characteristics;
    }; // class TXProcessDA
} /* namespace rootmap */

#endif // #ifndef TXProcessDA_H
