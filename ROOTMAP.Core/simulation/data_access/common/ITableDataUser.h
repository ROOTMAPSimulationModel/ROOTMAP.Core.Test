#ifndef ITableDataUser_H
#define ITableDataUser_H

/////////////////////////////////////////////////////////////////////////////
// Name:        ITableDataUser.h
// Purpose:     Declaration of the ITableDataUser class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// ITableDataUser is the interface for a class which is able to create
// Table windows from the TableDAI
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class TableDAI;

    class ITableDataUser
    {
    public:
        /**
         * Called by the DataAccessManager::visualiseTables for each table that
         * has configuration data
         *
         * @param
         */
        virtual void createTable(TableDAI& data) = 0;

        virtual ~ITableDataUser()
        {
        }

    protected:
        ITableDataUser()
        {
        }
    }; // class ITableDataUser
} /* namespace rootmap */

#endif // #ifndef ITableDataUser_H
