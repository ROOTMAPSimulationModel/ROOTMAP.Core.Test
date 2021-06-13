#ifndef RmBisonAdapter_H
#define RmBisonAdapter_H
/////////////////////////////////////////////////////////////////////////////
// Name:        RmBisonAdapter.h
// Purpose:     Declaration of the RmBisonAdapter class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

#if defined __cplusplus
extern "C"
{
#endif
    void onScoreboardDeclaration();
    void onPlaneDeclaration();
    void onRowColumnDeclaration();
    void onBoxesDeclaration();
    void onBoxDeclaration();

    void onDimensionSpec(char c, long index);
    void onValueSpec(double value);
    void onArraySpec(long x, long y, long z);

    void onStatementEnd();
    void onParseStart();
    void onParseEnd();
#if defined __cplusplus
}
#endif

// class RmBisonAdapter
// {
// public:
//     RmBisonAdapter();
//     virtual ~RmBisonAdapter();
// 
//     /**
//      * ThisFunctionName
//      *
//      * @param
//      * @return
//      */
// 
// private:
// 
//     ///
//     /// member declaration
// 
// 
// }; // class RmBisonAdapter

#endif // #ifndef RmBisonAdapter_H
