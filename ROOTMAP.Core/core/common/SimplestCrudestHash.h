/*
    SimplestCrudestHash.h

    Author:            Robert van Hugten
    Description:    <describe the SimplestCrudestHash class here>
*/

#ifndef SimplestCrudestHash_H
#define SimplestCrudestHash_H


namespace rootmap
{
    long int crudeHash(const char* s);


    const long int crudeHashTable = 'Tabl'; // = 0x5461626C
    const long int crudeHashEdit = 'Edit'; // = 0x45646974
    const long int crudeHashLayers = 'Laye'; // = 0x4C617965
    const long int crudeHashPlant = 'Plan'; // = 0x506C616E
    const long int crudeHashNewTable = 'New '; // = 0x4E657720
    const long int crudeHashScoreboard = 'Scor'; // = 0x53636F72
    const long int crudeHashView = 'View'; // = 0x56696577
    const long int crudeHashZero = 0x00000000;

    class SimplestCrudestHash
    {
    public:
        SimplestCrudestHash();
        virtual ~SimplestCrudestHash();
    };
} /* namespace rootmap */

#endif    // SimplestCrudestHash_H
