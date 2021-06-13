/////////////////////////////////////////////////////////////////////////////
// Name:        NameLocationData.h
// Purpose:     Declaration of the NameLocationData class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef NameLocationData_H
#define NameLocationData_H

#include <list>
#include <map>


namespace rootmap
{
    class NameLocationData
    {
    public:
        NameLocationData(const char* name, const char* location);
        NameLocationData(const std::string& name, const std::string& location);
        virtual ~NameLocationData();

        /**
         * ThisFunctionName
         *
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
        const std::string& getName() const;


        const std::string& getLocation() const;

    private:

        ///
        /// member declaration
        std::string myName;

        ///
        /// member declaration
        std::string myLocation;
    }; // class NameLocationData

    inline const std::string& NameLocationData::getName() const
    {
        return myName;
    }

    inline const std::string& NameLocationData::getLocation() const
    {
        return myLocation;
    }

    typedef std::list<NameLocationData> NameLocationDataList;
} /* namespace rootmap */

#endif // #ifndef NameLocationData_H
