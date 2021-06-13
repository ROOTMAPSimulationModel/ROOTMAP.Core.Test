#ifndef ITableDataMaintainer_H
#define ITableDataMaintainer_H

/////////////////////////////////////////////////////////////////////////////
// Name:        ITableDataMaintainer.h
// Purpose:     Declaration of the ITableDataMaintainer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// ITableDataMaintainer is the interface for a class which is able to store
// Table windows to the TableDPI
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class TableDPI;

    class ITableDataMaintainer
    {
    public:
        /**
         * Called by the DataAccessManager::visualiseTables for each table that
         * has configuration data
         *
         * @param
         */
        virtual void storeTable(TableDPI& data) = 0;

        virtual ~ITableDataMaintainer()
        {
        }

    protected:
        ITableDataMaintainer()
        {
        }
    }; // class ITableDataMaintainer
} /* namespace rootmap */

#endif // #ifndef ITableDataMaintainer_H
