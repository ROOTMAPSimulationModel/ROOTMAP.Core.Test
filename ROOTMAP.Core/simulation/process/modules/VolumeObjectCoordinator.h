#ifndef VolumeObjectCoordinator_H
#define VolumeObjectCoordinator_H

#include "simulation/process/common/Process.h"
#include "simulation/process/common/ProcessGroupLeader.h"

#include "core/common/Structures.h"
#include "core/log/Logger.h"

#include "simulation/process/modules/VolumeObject.h"

#include "simulation/scoreboard/IScoreboardIterationUser.h"

#include <list>

namespace rootmap
{
    // "Forward Declarations"
    class Plant;
    class PlantSummary;
    class Scoreboard;
    class ProcessActionDescriptor;
    class ScoreboardCoordinator;
    class ScoreboardStratum;
    class SimulationEngine;
    class ProcessSharedAttributeOwner;

    struct ScoreboardBoxIndex;

    typedef std::list<VolumeObject*> VolumeObjectList;

    // MSA TODO Think about implementing these macros as real iterators.
    // Low priority; do more important stuff first.

#define VOLUME_OBJECT_ITERATION_BEGIN for (VolumeObjectList::iterator iter = m_volumeObjectList.begin() ; iter != m_volumeObjectList.end() ; ++iter) { VolumeObject * vo = (*iter);
#define VOLUME_OBJECT_ITERATION_END }

#define VOLUME_OBJECT_CONST_ITERATION_BEGIN for (VolumeObjectList::const_iterator iter = m_volumeObjectList.begin() ; iter != m_volumeObjectList.end() ; ++iter) { const VolumeObject * vo = (*iter);
#define VOLUME_OBJECT_CONST_ITERATION_END }

#define VOLUME_OBJECT_ITERATION_USEPTR_BEGIN for (VolumeObjectList::iterator iter = m_volumeObjectListPtr->begin() ; iter != m_volumeObjectListPtr->end() ; ++iter) { VolumeObject * vo = (*iter);
#define VOLUME_OBJECT_ITERATION_USEPTR_END }

#define VOLUME_OBJECT_CONST_ITERATION_USEPTR_BEGIN for (VolumeObjectList::const_iterator iter = m_volumeObjectListPtr->begin() ; iter != m_volumeObjectListPtr->end() ; ++iter) { const VolumeObject * vo = (*iter);
#define VOLUME_OBJECT_CONST_ITERATION_USEPTR_END }

    // Important note about these macros: it is SAFE to use continue; to skip out
    // of a subsection iteration, because the END macros are simply closing braces.

#define SUBSECTION_ITER_BEGIN    VolumeObjectList::const_iterator iter = volumeobjectcoordinator->GetVolumeObjectList().begin();    for(size_t j=0; j<=VO_COUNT; ++j)    {    /*Get pointer to the VolumeObject, or a NULL pointer if it's the last iteration of the loop.*/    VolumeObject* vo = __nullptr;    if(j<VO_COUNT)    {    vo = *iter;    ++iter;    }    const size_t idx = vo==__nullptr ? 0 : vo->GetIndex();
#define SUBSECTION_ITER_END    }

    // MSA 11.06.23 Made this macro multiline for clarity, as an example.
#define SUBSECTION_ITER_WITH_PROPORTIONS_BEGIN    VolumeObjectList::const_iterator iter = volumeobjectcoordinator->GetVolumeObjectList().begin();\
                                                double noncoincidentProportion = 1;\
                                                for(size_t k=0; k<=VO_COUNT; ++k)\
                                                {\
                                                    double thisProportion;\
                                                    /*Get pointer to the VolumeObject, or a NULL pointer if it's the last iteration of the loop.*/    VolumeObject* vo = __nullptr;\
                                                    if(k<VO_COUNT)\
                                                    {\
                                                        vo = *iter;    ++iter;\
                                                        thisProportion = scoreboard->GetCoincidentProportion(box_index, vo->GetIndex());\
                                                        noncoincidentProportion -= thisProportion;\
                                                    }\
                                                    else\
                                                    {\
                                                        /* MSA 11.06.23 Updated to correctly handle for VOs located inside one another.  */\
                                                        thisProportion = Utility::CSMax(0.0, noncoincidentProportion);\
                                                    }\
                                                    const size_t idx = vo==__nullptr ? 0 : vo->GetIndex();
#define SUBSECTION_ITER_WITH_PROPORTIONS_END    }

#define SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_BEGIN    VolumeObjectList::const_iterator iter = volumeobjectcoordinator->GetVolumeObjectList().begin();    double thisNoncoincidentProportion = 1;    double nextNoncoincidentProportion = 1;    for(size_t ii=0; ii<=VO_COUNT; ++ii)    {    double thisProportion, nextProportion;    VolumeObject* vo = __nullptr;    if(ii<VO_COUNT)    {    vo = *iter;    ++iter;    thisProportion = scoreboard->GetCoincidentProportion(thisBox, vo->GetIndex());    nextProportion = scoreboard->GetCoincidentProportion(nextBox, vo->GetIndex());    thisNoncoincidentProportion -= thisProportion;    nextNoncoincidentProportion -= nextProportion;    }    else    {    /* MSA 11.06.23 Updated to correctly handle for VOs located inside one another.*/ thisProportion = Utility::CSMax(0.0, thisNoncoincidentProportion);    nextProportion = Utility::CSMax(0.0, nextNoncoincidentProportion);    }    const size_t idx = vo==__nullptr ? 0 : vo->GetIndex();
#define SUBSECTION_ITER_WITH_TO_AND_FROM_BOXES_AND_PROPORTIONS_END }


    struct ScoreboardBoxSubsection
    {
        BoxIndex boxIndex;
        VolumeObject* volumeObject;
        double coincidentProportion;
    };

    typedef std::list<ScoreboardBoxSubsection>::iterator ScoreboardBoxSubsectionIterator;
    typedef std::list<ScoreboardBoxSubsection>::const_iterator ScoreboardBoxSubsectionConstIterator;

    class VolumeObjectCoordinator
        : public ProcessGroupLeader, public IScoreboardIterationUser
    {
    public:
        DECLARE_DYNAMIC_CLASS(VolumeObjectCoordinator)

        VolumeObjectCoordinator();
        void Initialise(SimulationEngine& engine);

        ~VolumeObjectCoordinator();

        // Mandatory overrides (abstract in ProcessGroupLeader)
        virtual Process* FindProcessByProcessID(ProcessIdentifier process_id);
        virtual Process* FindProcessByProcessName(const std::string& pname);

        virtual void UseScoreboard(Scoreboard* scoreboard);

        ScoreboardBoxSubsectionIterator begin(const BoxIndex& b);
        ScoreboardBoxSubsectionIterator end(const BoxIndex& b);

        /**
         *    MSA N.B. The VolumeObjectCoordinator itself does not (currently)
         *    draw anything to the RaytracerData object.
         *    However, it may be the <source> class for the
         *    raytracer Output Rule, and as such needs access
         *    to the object pointer for outputting to file.
         */
        virtual bool DoesRaytracerOutput() const;
        virtual bool DoesDrawing() const;
        virtual long int DoRaytracerOutput(RaytracerData* raytracerData);
        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);
        virtual DoubleCoordinate GetOrigin() const;
        const VolumeObjectList& GetVolumeObjectList() const;

        const VolumeObject* GetContainingVolumeObject(const DoubleCoordinate& dc) const;
        const size_t GetContainingVolumeObjectIndex(const DoubleCoordinate& dc) const;

        // This function returns a pointer to heap memory that must be disposed of by the caller.
        ProcessDAI* AccomodateVolumeObjects(const ProcessDAI& processDAI, std::vector<std::string>& invariantCharacteristicNames) const;
    public:
        ////////////////////////////////////////
        // VO addition, removal and iteration //
        ////////////////////////////////////////
        void AddVolumeObject(VolumeObject* newVO);

        void RemoveVolumeObject(VolumeObject* VOToRemove);

        const size_t GetNextVOIndex();

        // todo iter

    private:
        RootMapLoggerDeclaration();

        std::map<BoxIndex, std::list<ScoreboardBoxSubsection>> m_subsections;

        int m_validScoreboardsFound;

        ScoreboardCoordinator* m_scoreboardCoordinator;
        //
        // Linked list of VolumeObjects; VOs are owned by this class
        VolumeObjectList m_volumeObjectList;

        size_t m_nextVOIndex;

        bool m_hasBeenDrawn;
    };


    inline const size_t VolumeObjectCoordinator::GetNextVOIndex()
    {
        // Remember that 0 represents the nonexistent VolumeObject
        return ++m_nextVOIndex;
    }

    inline const VolumeObjectList& VolumeObjectCoordinator::GetVolumeObjectList() const
    {
        return m_volumeObjectList;
    }
} /* namespace rootmap */

#endif // #ifndef VolumeObjectCoordinator_H
