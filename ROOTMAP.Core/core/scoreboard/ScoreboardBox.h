#ifndef ScoreboardBox_H
#define ScoreboardBox_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ScoreboardBox.h
// Purpose:     Declaration of various Scoreboard Box types
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////

//#include "CS_DEBUG_DEFINES.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "core/utility/Utility.h"
#include <map>

namespace rootmap
{
    class VolumeObject;

    //
    // Scoreboard definitions
    //

    //
    // Scoreboard Box Index: the 0-based index of the box within the scoreboard.
    // Derived from the BoxCoordinate or the x,y,z using
    //  Scoreboard::GetIndexFromXYZ()
    typedef long int BoxIndex;

    struct TransferRestrictionData
    {
        double unrestrictedProportionFrom;
        double restrictedProportionFrom;
        double unrestrictedProportionTo;
        double restrictedProportionTo;
        double voFromPermeability;
        double voToPermeability;
        VolumeObject* volumeObjectFrom;
        VolumeObject* volumeObjectTo;
        // MSA 11.06.23 This is usually NULL, that is,
        // plain Scoreboard without any VolumeObject.
        // Sometimes it points to another VolumeObject, 
        // larger than volumeObjectFrom or volumeObjectTo,
        // which surrounds this TRD's box(es) completely.
        // CRITICAL:
        // Its surfaces MUST NOT occupy this TRD's box(es) at all.
        // MSA 11.06.29 Allowed each box to have a different background VO.
        VolumeObject* backgroundVolumeObjectFrom;
        VolumeObject* backgroundVolumeObjectTo;
        bool unrestrictedTransferWithinVO;
    };

    const BoxIndex InvalidBoxIndex = -1;

    /// not to be confused with BoxIndex, this represents the 1-based indices
    /// of a BoxCoordinate
    typedef long int BoxCoordinateIndex;

    //
    // BoxCoordinate: the 
    struct BoxCoordinate
    {
        BoxCoordinateIndex x;
        BoxCoordinateIndex y;
        BoxCoordinateIndex z;

        BoxCoordinate();
        explicit BoxCoordinate(BoxCoordinateIndex x_, BoxCoordinateIndex y_, BoxCoordinateIndex z_);

        void set(BoxCoordinateIndex x_, BoxCoordinateIndex y_, BoxCoordinateIndex z_);
        bool isValid() const;

        bool operator==(const BoxCoordinate& rhs) const;
        bool operator!=(const BoxCoordinate& rhs) const;
        // copy constructor and equality operator ok as default c++ bitwise
        // implementations
    };

    // WAS:
    // struct BoxCoordinate { short int x,y,z; };
    // ALSO:
    // typedef struct BoxCoordinate BoxCoordinate;
    // which is deprecated for c++

    //
    // CharacteristicValueArray Index: the index of a characteristic
    // (ie. ValueArray) in the vector of CharacteristicValueArrays
    typedef long int CharacteristicIndex;

    typedef std::vector<CharacteristicIndex> CharacteristicIndices;

    const CharacteristicIndex InvalidCharacteristicIndex = -1;


    //
    // Storing stuff in scoreboards
    //
    // CharacteristicValueArray : the 
    typedef double CharacteristicValue;

    struct DF3Array
    {
        Utility::DF3Voxel*** values;
        const unsigned short xmax, ymax, zmax; // Only 6 bytes allowed for DF3 header. Short>2b case checked for and handled in output code.
    public:
        explicit DF3Array(const unsigned short x, const unsigned short y, const unsigned short z)
            : xmax(x), ymax(y), zmax(z)
        {
            // We assume here that if malloc fails, the whole thing is toast
            values = (Utility::DF3Voxel***)malloc(xmax * sizeof(Utility::DF3Voxel **));
            unsigned short i, j;
            for (i = 0; i < xmax; ++i)
            {
                values[i] = (Utility::DF3Voxel**)malloc(ymax * sizeof(Utility::DF3Voxel *));
            }
            for (i = 0; i < xmax; ++i)
            {
                for (j = 0; j < ymax; ++j)
                {
                    values[i][j] = (Utility::DF3Voxel*)malloc(zmax * sizeof(Utility::DF3Voxel));
                }
            }
        }

        ~DF3Array()
        {
            // MSA 09.10.02 Must use C-style memory management for deletion also
            unsigned short i, j;
            for (i = 0; i < xmax; ++i)
            {
                for (j = 0; j < ymax; ++j)
                {
                    free(values[i][j]);
                    values[i][j] = __nullptr; // Just in case
                }
            }
            for (i = 0; i < xmax; ++i)
            {
                free(values[i]);
                values[i] = __nullptr;
            }
            free(values);
            values = __nullptr;
        }

        // Function to set the value of any given DF3Voxel. 
        // Returns false if an out-of-range index is specfied.
        const bool set(const unsigned short atx, const unsigned short aty, const unsigned short atz, const Utility::DF3Voxel val)
        {
            if (atx >= xmax || aty >= ymax || atz >= zmax) return false;
            values[atx][aty][atz] = val;
            return true;
        }
    };

    typedef CharacteristicValue* CharacteristicValueArray;


    inline BoxCoordinate::BoxCoordinate()
        : x(0), y(0), z(0)
    {
    }

    inline BoxCoordinate::BoxCoordinate(long int x_, long int y_, long int z_)
        : x(x_), y(y_), z(z_)
    {
    }

    inline void BoxCoordinate::set(BoxIndex x_, BoxIndex y_, BoxIndex z_)
    {
        x = x_;
        y = y_;
        z = z_;
    }

    inline bool BoxCoordinate::isValid() const
    {
        return ((x > InvalidBoxIndex) && (y > InvalidBoxIndex) && (z > InvalidBoxIndex));
    }

    inline bool BoxCoordinate::operator==(const BoxCoordinate& rhs) const
    {
        return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
    }

    inline bool BoxCoordinate::operator!=(const BoxCoordinate& rhs) const
    {
        return ((x != rhs.x) || (y != rhs.y) || (z != rhs.z));
    }

    inline std::ostream& operator<<(std::ostream& ostr, const BoxCoordinate& boxcoord)
    {
        ostr << "(" << boxcoord.x << "," << boxcoord.y << "," << boxcoord.z << ")";
        return ostr;
    }

    /**
     * Nothing special about the class - forms the abstract base class for "special"
     * data that will be stored in the scoreboard
     *
     * TODO: make this a wx dynamic class ?
     */
    class SpecialScoreboardData
    {
    public:
        virtual ~SpecialScoreboardData()
        {
        }

    protected:
        SpecialScoreboardData()
        {
        }

    private:
        SpecialScoreboardData(const SpecialScoreboardData&);
        SpecialScoreboardData& operator=(const SpecialScoreboardData&);
    }; // class SpecialScoreboardData

    typedef SpecialScoreboardData* SpecialScoreboardDataPtr;
    typedef SpecialScoreboardDataPtr* SpecialScoreboardDataArray;
} /* namespace rootmap */
#endif // #ifndef ScoreboardBox_H
