/*
    SimplestCrudestHash.cpp

    Author:            Robert van Hugten
    Description:    <describe the SimplestCrudestHash class here>
*/

#include "SimplestCrudestHash.h"


namespace rootmap
{
    SimplestCrudestHash::SimplestCrudestHash()
    {
    }


    SimplestCrudestHash::~SimplestCrudestHash()
    {
    }


    //
    // Should really be implemented as a map, in the fullness of time.
    // Because at the moment we don't have that many SVS-dependent menus,
    // its ok to do it the crude and un-beautiful way.
    //
    // The alternative to the crude hash is strcmp. Not so much more difficult to
    // use, more accurate than the crudeHash, and probably as fast as a map.
    //
    typedef union
    {
        long int thelongint;
        char thechararray[4];
    } crudeHashUnion;

    long int crudeHash(const char* s)
    {
        // if the string is non-null, copy the first 4 bytes to the crude hash
        if ((s != 0) && (s[0] != 0))
        {
            crudeHashUnion chu;
            for (int i = 0; i < 4; ++i)
            {
                chu.thechararray[i] = s[i];
            }
            return chu.thelongint;
        }
        // otherwise, the "hash" is zero
        return 0;
    }
} /* namespace rootmap */
