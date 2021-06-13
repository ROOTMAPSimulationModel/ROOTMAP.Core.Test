// MSA 11.05.26 Just a wrapper on the Packetizer implementation
// Needless to say... don't use this for any cryptographic purposes.

/*
 *  sha1.h
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved.
 *
 *****************************************************************************
 *  $Id: sha1.h 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This class implements the Secure Hashing Standard as defined
 *      in FIPS PUB 180-1 published April 17, 1995.
 *
 *      Many of the variable names in this class, especially the single
 *      character names, were used because those were the names used
 *      in the publication.
 *
 *      Please read the file sha1.cpp for more information.
 *
 */

#ifndef _SHA1_H_
#define _SHA1_H_

namespace rootmap
{
    // IMPORTANT NOTE: A SHA-1 fingerprint is 160 bits
    // MSA TODO Handle for non-4-byte ints
    struct SHA1Fingerprint
    {
        unsigned int zero, one, two, three, four;

        SHA1Fingerprint()
            : zero(0)
            , one(0)
            , two(0)
            , three(0)
            , four(0)
        {
        }

        SHA1Fingerprint(const unsigned int& theZeroth, const unsigned int& theFirst, const unsigned int& theSecond, const unsigned int& theThird, const unsigned int& theFourth)
            : zero(theZeroth)
            , one(theFirst)
            , two(theSecond)
            , three(theThird)
            , four(theFourth)
        {
        }

        SHA1Fingerprint(const unsigned int theArray[5])
            : zero(theArray[0])
            , one(theArray[1])
            , two(theArray[2])
            , three(theArray[3])
            , four(theArray[4])
        {
        }

        bool operator==(const SHA1Fingerprint& rhs)
        {
            return zero == rhs.zero
                && one == rhs.one
                && two == rhs.two
                && three == rhs.three
                && four == rhs.four;
        }

        bool operator!=(const SHA1Fingerprint& rhs)
        {
            return !(this->operator==(rhs));
        }

        bool operator<(const SHA1Fingerprint& rhs)
        {
            if (zero == rhs.zero)
            {
                if (one == rhs.one)
                {
                    if (two == rhs.two)
                    {
                        if (three == rhs.three)
                        {
                            if (four == rhs.four)
                            {
                                return false;
                            }
                            return four < rhs.four;
                        }
                        return three < rhs.three;
                    }
                    return two < rhs.two;
                }
                return one < rhs.one;
            }
            return zero < rhs.zero;
        }

        bool operator>(const SHA1Fingerprint& rhs)
        {
            if (zero == rhs.zero)
            {
                if (one == rhs.one)
                {
                    if (two == rhs.two)
                    {
                        if (three == rhs.three)
                        {
                            if (four == rhs.four)
                            {
                                return false;
                            }
                            return four > rhs.four;
                        }
                        return three > rhs.three;
                    }
                    return two > rhs.two;
                }
                return one > rhs.one;
            }
            return zero > rhs.zero;
        }
    };

    class RmSHA1
    {
    public:

        RmSHA1();
        virtual ~RmSHA1();

        /*
         *  Re-initialize the class
         */
        void Reset();

        /*
         *  Returns the message digest
         */
        bool Result(unsigned* message_digest_array);

        /*
         *  Provide input to RmSHA1
         */
        void Input(const unsigned char* message_array,
            unsigned length);
        void Input(const char* message_array,
            unsigned length);
        void Input(unsigned char message_element);
        void Input(char message_element);
        RmSHA1& operator<<(const char* message_array);
        RmSHA1& operator<<(const unsigned char* message_array);
        RmSHA1& operator<<(const char message_element);
        RmSHA1& operator<<(const unsigned char message_element);

    private:

        /*
         *  Process the next 512 bits of the message
         */
        void ProcessMessageBlock();

        /*
         *  Pads the current message block to 512 bits
         */
        void PadMessage();

        /*
         *  Performs a circular left shift operation
         */
        inline unsigned CircularShift(int bits, unsigned word);

        unsigned H[5]; // Message digest buffers

        unsigned Length_Low; // Message length in bits
        unsigned Length_High; // Message length in bits

        unsigned char Message_Block[64]; // 512-bit message blocks
        int Message_Block_Index; // Index into message block array

        bool Computed; // Is the digest computed?
        bool Corrupted; // Is the message digest corruped?
    };
}

#endif
