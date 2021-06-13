/////////////////////////////////////////////////////////////////////////////
// Name:        DirectoryData.h
// Purpose:     Declaration of the DirectoryData class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#ifndef DirectoryData_H
#define DirectoryData_H

#include <list>
#include <map>


namespace rootmap
{
    class DirectoryData
    {
    public:
        DirectoryData(const char* name, const char* location);
        DirectoryData(const std::string& name, const std::string& location);
        virtual ~DirectoryData();

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
    }; // class DirectoryData

    inline const std::string& DirectoryData::getName() const
    {
        return myName;
    }

    inline const std::string& DirectoryData::getLocation() const
    {
        return myLocation;
    }

    typedef std::list<DirectoryData> DirectoryDataList;
} /* namespace rootmap */

#endif // #ifndef DirectoryData_H
