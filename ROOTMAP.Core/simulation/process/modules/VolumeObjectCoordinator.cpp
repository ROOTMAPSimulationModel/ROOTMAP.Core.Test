/*
VolumeObjectCoordinator
*/

#include "simulation/common/SimulationEngine.h"
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/modules/VolumeObject.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"

namespace rootmap
{
    IMPLEMENT_DYNAMIC_CLASS(VolumeObjectCoordinator, ProcessGroupLeader);
    RootMapLoggerDefinition(VolumeObjectCoordinator);

    VolumeObjectCoordinator::VolumeObjectCoordinator()
        : ProcessGroupLeader()
        , m_scoreboardCoordinator(__nullptr)
        , m_nextVOIndex(0)
        , m_hasBeenDrawn(false)
        , m_validScoreboardsFound(0)
    {
        RootMapLoggerInitialisation("rootmap.VolumeObjectCoordinator");
    }

    /*
        Our implementation of this method registers all the VolumeObjects with the ScoreboardCoordinator.
        This involves calculation of how they interact with each Scoreboardbox and thus is quite computationally intensive.
    */
    void VolumeObjectCoordinator::Initialise(SimulationEngine& engine)
    {
        DataAccessManager& dam = engine.GetDataAccessManager();
        dam.constructVolumeObjects(this);
        m_scoreboardCoordinator = &(engine.GetScoreboardCoordinator());
        VOLUME_OBJECT_ITERATION_BEGIN
            m_scoreboardCoordinator->RegisterVolumeObject(vo);
        VOLUME_OBJECT_ITERATION_END

            m_scoreboardCoordinator->IterateOverScoreboards(this);
    }

    VolumeObjectCoordinator::~VolumeObjectCoordinator()
    {
        VOLUME_OBJECT_ITERATION_BEGIN
            delete vo;
        VOLUME_OBJECT_ITERATION_END
    }

    void VolumeObjectCoordinator::UseScoreboard(Scoreboard* scoreboard)
    {
        if (m_validScoreboardsFound < 1)
        {
            for (BoxIndex b = scoreboard->begin(); b != scoreboard->end(); ++b)
            {
                ScoreboardBoxSubsection sbs;
                sbs.boxIndex = b;
                sbs.volumeObject = __nullptr;
                sbs.coincidentProportion = scoreboard->GetCoincidentProportion(b, 0);
                m_subsections[b].push_back(sbs);

                VOLUME_OBJECT_ITERATION_BEGIN
                    ScoreboardBoxSubsection innerSbs;
                innerSbs.boxIndex = b;
                innerSbs.volumeObject = vo;
                innerSbs.coincidentProportion = scoreboard->GetCoincidentProportion(b, vo->GetIndex());
                m_subsections[b].push_back(innerSbs);
                VOLUME_OBJECT_ITERATION_END
            }
        }
        else
        {
            throw RmException("This code does not expect a multiple-Scoreboard simulation");
        }
        m_validScoreboardsFound++;
    }

    ScoreboardBoxSubsectionIterator VolumeObjectCoordinator::begin(const BoxIndex& b)
    {
        return m_subsections[b].begin();
    }

    ScoreboardBoxSubsectionIterator VolumeObjectCoordinator::end(const BoxIndex& b)
    {
        return m_subsections[b].end();
    }

    DoubleCoordinate VolumeObjectCoordinator::GetOrigin() const
    {
        return DoubleCoordinate(0, 0, 0);
    }

    const VolumeObject* VolumeObjectCoordinator::GetContainingVolumeObject(const DoubleCoordinate& dc) const
    {
        const VolumeObject* containingVO = __nullptr;
        VOLUME_OBJECT_CONST_ITERATION_BEGIN
            if (vo->Contains(dc) && (containingVO == __nullptr || vo->Within(containingVO)))
            {
                containingVO = vo;
            }
        VOLUME_OBJECT_CONST_ITERATION_END

            return containingVO;
    }

    const size_t VolumeObjectCoordinator::GetContainingVolumeObjectIndex(const DoubleCoordinate& dc) const
    {
        const VolumeObject* containingVO = GetContainingVolumeObject(dc);
        return containingVO == __nullptr ? 0 : containingVO->GetIndex();
    }

    void VolumeObjectCoordinator::DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard)
    {
        VOLUME_OBJECT_ITERATION_BEGIN
            if (!m_hasBeenDrawn)
            {
                // All classes derived from VolumeObject do their own drawing.
                // Give them all the pointer to the one and only ProcessDrawing,
                // so all their visualisations can be drawn to the same ProcessDrawing.
                // By placing all VOs in one (eventual) VertexBuffer, we can handle the translucency better.
                vo->SetDrawing(&(getDrawing()));
            }
        vo->DrawScoreboard(area, scoreboard);
        VOLUME_OBJECT_ITERATION_END

            m_hasBeenDrawn = true;
    }

    long int VolumeObjectCoordinator::DoRaytracerOutput(RaytracerData* raytracerData)
    {
        VOLUME_OBJECT_ITERATION_BEGIN
            if (vo->DoesRaytracerOutput()) vo->DoRaytracerOutput(raytracerData);
        VOLUME_OBJECT_ITERATION_END

            return kNoError;
    }

    bool VolumeObjectCoordinator::DoesRaytracerOutput() const
    {
        return (true);
    }

    bool VolumeObjectCoordinator::DoesDrawing() const
    {
        return (true);
    }

    Process* VolumeObjectCoordinator::FindProcessByProcessID(ProcessIdentifier /* process_id */)
    {
        return __nullptr;
    }

    Process* VolumeObjectCoordinator::FindProcessByProcessName(const std::string& /* pname */)
    {
        return __nullptr;
    }

    void VolumeObjectCoordinator::AddVolumeObject(VolumeObject* newVO)
    {
        m_volumeObjectList.push_back(newVO);
    }

    void VolumeObjectCoordinator::RemoveVolumeObject(VolumeObject* VOToRemove)
    {
        m_volumeObjectList.remove(VOToRemove);
        delete VOToRemove;
    }

    ProcessDAI* VolumeObjectCoordinator::AccomodateVolumeObjects(const ProcessDAI& processDAI, std::vector<std::string>& invariantCharacteristicNames) const
    {
        // MSA 11.03.17 We now always make variants, regardless of number of VolumeObjects.
        // This is so config files referring to "$CHARACTERISTIC_NAME VolumeObject [none]" can be used unmodified
        // in simulations with no VolumeObjects.

        const CharacteristicDAICollection characteristic_data = processDAI.getCharacteristics();
        CharacteristicDAICollection new_characteristic_data;
        for (CharacteristicDAICollection::const_iterator iter = characteristic_data.begin();
            iter != characteristic_data.end();
            ++iter)
        {
            CharacteristicDAI* cdai = *iter;
            std::string name = cdai->getName();
            std::string units = cdai->getUnits();
            TXCharacteristicDA* newcdai = new TXCharacteristicDA(cdai->getIdentifier(), name, units,
                cdai->getStratum(), cdai->getMinimum(), cdai->getMaximum(),
                cdai->getDefault(), cdai->isVisible(), cdai->isEdittable(),
                cdai->isSavable(), cdai->hasSpecialPerBoxInfo());
            new_characteristic_data.push_back(newcdai);

            if (find(invariantCharacteristicNames.begin(), invariantCharacteristicNames.end(), cdai->getName()) == invariantCharacteristicNames.end())
            {
                for (VolumeObjectList::const_iterator voliter = GetVolumeObjectList().begin();
                    voliter != GetVolumeObjectList().end();
                    ++voliter)
                {
                    std::string newname = cdai->getName() + " VolumeObject " + std::to_string((*voliter)->GetIndex());
                    units = cdai->getUnits();
                    newcdai = new TXCharacteristicDA(cdai->getIdentifier(), newname, units,
                        cdai->getStratum(), cdai->getMinimum(), cdai->getMaximum(),
                        cdai->getDefault(), cdai->isVisible(), cdai->isEdittable(),
                        cdai->isSavable(), cdai->hasSpecialPerBoxInfo());
                    new_characteristic_data.push_back(newcdai);
                }
            }
        }

        TXProcessDA* updatedData = new TXProcessDA(processDAI.getName(),
            processDAI.doesOverride(),
            processDAI.getIdentifier(),
            processDAI.getStratum(),
            processDAI.getActivity());

        for (CharacteristicDAICollection::iterator newiter = new_characteristic_data.begin();
            newiter != new_characteristic_data.end();
            ++newiter)
        {
            updatedData->addCharacteristicDA(dynamic_cast<TXCharacteristicDA*>(*newiter));
        }
        return updatedData;
    }
} /* namespace rootmap */
