#ifndef IViewDataMaintainer_H
#define IViewDataMaintainer_H

/////////////////////////////////////////////////////////////////////////////
// Name:        IViewDataMaintainer.h
// Purpose:     Declaration of the IViewDataMaintainer class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// IViewDataMaintainer is the interface for a class which is able to store
// View windows to the ViewDPI
/////////////////////////////////////////////////////////////////////////////

namespace rootmap
{
    class ViewDPI;

    class IViewDataMaintainer
    {
    public:
        /**
         *
         */
        virtual void storeView(ViewDPI& data) = 0;

        virtual ~IViewDataMaintainer()
        {
        }

    protected:
        IViewDataMaintainer()
        {
        }
    }; // class IViewDataMaintainer
} /* namespace rootmap */

#endif // #ifndef IViewDataMaintainer_H
