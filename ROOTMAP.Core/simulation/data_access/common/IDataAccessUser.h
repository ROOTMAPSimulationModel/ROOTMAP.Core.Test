#ifndef IDataAccessUser_H
#define IDataAccessUser_H

/////////////////////////////////////////////////////////////////////////////
// Name:        IDataAccessUser.h
// Purpose:     Declaration of the IDataAccessUser class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////


namespace rootmap
{
    class IDataAccessUser
    {
    public:
        /**
         * Abstract method implemented by classes that use the DataAccessManager
         *
         * @return the name of this class insofar as it owns data
         */
        std::string getDataOwnerName() = 0;

    protected:

        IDataAccessUser();
        virtual ~IDataAccessUser();
    }; // class IDataAccessUser
} /* namespace rootmap */

#endif // #ifndef IDataAccessUser_H
