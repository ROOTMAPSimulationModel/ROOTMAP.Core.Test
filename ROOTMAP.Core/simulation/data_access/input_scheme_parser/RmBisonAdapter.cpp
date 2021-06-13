/////////////////////////////////////////////////////////////////////////////
// Name:        RmBisonAdapter.cpp
// Purpose:     Implementation of the RmBisonAdapter class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "RmBisonAdapter.h"
#include "InputSchemeParser.h"
#include "simulation/data_access/interface/ScoreboardDataDAI.h"


void onScoreboardDeclaration()
{
    rootmap::InputSchemeParser::instance().setElementType(rootmap::sceScoreboard);
}

void onPlaneDeclaration()
{
    rootmap::InputSchemeParser::instance().setElementType(rootmap::scePlane);
}

void onRowColumnDeclaration()
{
    rootmap::InputSchemeParser::instance().setElementType(rootmap::sceRowColumn);
}

void onBoxDeclaration()
{
    rootmap::InputSchemeParser::instance().setElementType(rootmap::sceBox);
}

void onBoxesDeclaration()
{
    rootmap::InputSchemeParser::instance().setElementType(rootmap::sceBoxes);
}

void onDimensionSpec(char c, long index)
{
    rootmap::InputSchemeParser::instance().accumulateDimension(c, index);
}

void onValueSpec(double value)
{
    rootmap::InputSchemeParser::instance().accumulateValue(value);
}

void onArraySpec(long x, long y, long z)
{
    rootmap::InputSchemeParser::instance().accumulateArray(x, y, z);
}

void onStatementEnd()
{
    rootmap::InputSchemeParser::instance().endStatement();
}

void onParseStart()
{
    rootmap::InputSchemeParser::instance().startParse();
}

void onParseEnd()
{
    rootmap::InputSchemeParser::instance().endParse();
}


// RmBisonAdapter::RmBisonAdapter()
// {
// }
// 
// RmBisonAdapter::~RmBisonAdapter()
// {
// }


