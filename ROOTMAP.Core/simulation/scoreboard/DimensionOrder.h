#ifndef DimensionOrder_H
#define DimensionOrder_H
/////////////////////////////////////////////////////////////////////////////
// Name:        DimensionOrder.h
// Purpose:     Declaration of the DimensionOrder class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
/////////////////////////////////////////////////////////////////////////////
#include "core/scoreboard/Dimension.h"

#include <string>

namespace rootmap
{
    /**
     * Encapsulates functionality for the order in which 3 dimensions are used,
     * and whether they are incrementing or decrementing
     */
    struct DimensionOrder
    {
    public:
        DimensionOrder();
        //DimensionOrder(Dimension dimensions[3], signed int increments[3]);

        virtual ~DimensionOrder();

        // MSA 09.12.10 This function is suspected to be buggy and might never even be used.
        /**
         * Sets the dimension order from 2 arrays
         *
         * @param dimensions an array of 3 dimensions
         * @param increments an array of 3 signed integers - valid values [-1,+1]
         */
         //void Set(Dimension dimensions[3], signed int increments[3]);

         /**
          *
          */
        void Set(const std::string& spec);

        /**
         * see ScoreboardDataOutputStrategy::OutputToFile for usage example
         */
        void MatchDimensions(long* x_ptr, long* y_ptr, long* z_ptr,
            long* * least_hndl, long* * middle_hndl, long* * most_hndl);

        void MatchIncrements(signed int& least_inc,
            signed int& middle_inc,
            signed int& most_inc);

    private:

        ///
        /// The three dimensions in order
        Dimension m_dimensions[3];

        ///
        /// The incremental/decremental nature of the dimensions. Only the sign
        /// of the integer is necessary, thus only +1 and -1 are valid values 
        signed int m_increments[3];

        friend struct DimensionIterator;
    }; // class DimensionOrder
} /* namespace rootmap */

#endif // #ifndef DimensionOrder_H
