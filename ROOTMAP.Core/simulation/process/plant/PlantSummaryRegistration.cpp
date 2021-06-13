#include "core/common/Constants.h"
#include "simulation/process/plant/PlantSummaryRegistration.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantSummary.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "core/utility/Utility.h"
#include "simulation/common/SprintUtility.h"
#include "simulation/process/modules/VolumeObjectCoordinator.h"


namespace rootmap
{
    RootMapLoggerDefinition(PlantSummaryRegistration);

    //
    // The one and only constructor
    PlantSummaryRegistration::PlantSummaryRegistration()
        : psr_per_box(true)
        , psr_all_plants_individually(false)
        , psr_all_branch_order_combinations(false)
        , psr_all_volume_object_combinations(false)
        , psr_wrap(wraporderNone)
        , psr_summary_type(srNone)
        , psr_requesting_module(0)
        , psr_start_index(-1)
        , psr_number_of_plants(-1)
        , psr_number_of_spatial_subsections(1)
        , psr_number_of_summaries(-1)
    {
        RootMapLoggerInitialisation("rootmap.PlantSummaryRegistration");
    }

    void PlantSummaryRegistration::RegisterByPerBox(Boolean per_box)
    {
        psr_per_box = per_box;
    }

    void PlantSummaryRegistration::RegisterAllPlants()
    {
        psr_all_plants_individually = false;
    }

    void PlantSummaryRegistration::RegisterAllPlantCombos()
    {
        psr_all_plants_individually = true;
    }

    void PlantSummaryRegistration::RegisterAllBranchOrders()
    {
        psr_all_branch_order_combinations = false;
    }

    void PlantSummaryRegistration::RegisterAllBranchOrderCombos()
    {
        psr_all_branch_order_combinations = true;
    }

    void PlantSummaryRegistration::RegisterAllVolumeObjects()
    {
        psr_all_volume_object_combinations = false;
    }

    void PlantSummaryRegistration::RegisterAllVolumeObjectCombos()
    {
        psr_all_volume_object_combinations = true;
    }

    void PlantSummaryRegistration::RegisterWrapping(Boolean wrap_x, Boolean wrap_y)
    {
        psr_wrap = wraporderNone;
        if (wrap_x) psr_wrap = wraporderX;
        if (wrap_y) psr_wrap |= wraporderY;
    }

    void PlantSummaryRegistration::RegisterSummaryType(SummaryRoutine summary_type)
    {
        psr_summary_type = summary_type;
    }

    void PlantSummaryRegistration::RegisterModule(Process* module)
    {
        psr_requesting_module = module;
    }

    Boolean PlantSummaryRegistration::IsPerBox()
    {
        return (psr_per_box);
    }


    long PlantSummaryRegistration::GetStartIndex()
    {
        return (psr_start_index);
    }


    long PlantSummaryRegistration::GetNumberOfPlants()
    {
        return (psr_number_of_plants);
    }

    size_t PlantSummaryRegistration::GetNumberOfSpatialSubsections()
    {
        return (psr_number_of_spatial_subsections);
    }

    long PlantSummaryRegistration::GetNumberOfBranchOrders()
    {
        return (psr_number_of_branch_orders);
    }

    ScoreboardStratum PlantSummaryRegistration::GetSummaryRoutineStratum(SummaryRoutine sr)
    {
        ScoreboardStratum stratum(ScoreboardStratum::NONE);

        switch (sr)
        {
        case srRootLength:
        case srRootLengthDensity:
        case srRootTipCount:
        case srRootTipCountDensity:
        case srRootBranchCount:
        case srRootBranchCountDensity:
        case srRootVolume:
        case srRootSurfaceArea:
            stratum = ScoreboardStratum::Soil;
            break;

        default:
            break;
        }

        return stratum;
    }

    PlantSummaryRegistration* nonexistentCurrentPSR = 0;

    long int PlantSummaryRegistration::MakeScoreboardSummaries
    (ProcessActionDescriptor* action,
        PlantCoordinator* plantcoordinator
    )
    {
        RootMapLogTrace("PlantSummaryRegistration::MakeScoreboardSummaries");

        Use_ReturnValue;
        Use_SharedAttributeManager;
        Use_VolumeObjectCoordinator;
        CharacteristicDescriptor* cd = new CharacteristicDescriptor;

        //
        // Fill in most of the CharacteristicDescriptor
        ScoreboardStratum stratum = GetSummaryRoutineStratum(psr_summary_type);
        if (0 != action->GetScoreboard())
        {
            stratum = action->GetScoreboard()->GetScoreboardStratum();
        }
        cd->id = plantcoordinator->GetNewSummaryID(Utility::DetermineSummaryTypeArrayIndex(psr_summary_type), stratum);
        cd->ScoreboardIndex = -1; // added during ProcessModuleBase::AddCharacteristic()
        cd->Units = Utility::NewUnitsString(psr_summary_type);
        cd->Minimum = 0.0;
        cd->Maximum = Utility::infinity();
        cd->Default = 0.0;
        cd->Visible = true;
        cd->Edittable = false;
        cd->Savable = false;
        cd->SpecialPerBoxInfo = false;
        cd->m_ScoreboardStratum = stratum;

        //
        // Construct the name.  Only the SummaryRoutine and WrapOrder are significant
        // here.  If the Plant, BranchOrder and VolumeObject are also, they will be taken care of
        // by the SharedAttributeManager::MakeAttributes
        cd->Name = SprintUtility::StringOf(psr_summary_type) + " "
            + SprintUtility::StringOf(psr_wrap);

        LOG_DEBUG << "Created empty CharacteristicDescriptor {Name:" << cd->Name.c_str()
            << "} {Units:" << cd->Units.c_str()
            << "} {Stratum:" << stratum.value()
            << "} {ID:" << cd->id
            << "}";

        myPlant = 0;
        //
        // set up SharedAttributeVariations if required
        std::vector<std::string> variation_names;
        if (psr_all_plants_individually)
        {
            variation_names.push_back(std::string("Plant"));

            myPlantIterator = (plantcoordinator->GetPlants()).begin();
            if (myPlantIterator != (plantcoordinator->GetPlants()).end())
            {
                myPlant = *myPlantIterator;
            }
        }
        else
        {
            myPlantIterator = (plantcoordinator->GetPlants()).end();
        }

        if (psr_all_branch_order_combinations)
        {
            variation_names.push_back(std::string("RootOrder"));
        }

        if (psr_all_volume_object_combinations)
        {
            variation_names.push_back(std::string("VolumeObject"));
        }

        // fill in various PSR fields
        psr_start_index = -1;
        psr_number_of_plants = plantcoordinator->GetPlants().size();
        psr_number_of_branch_orders = 4;
        psr_number_of_spatial_subsections = VO_COUNT + 1;

        //
        // The push for attributes to be made
        nonexistentCurrentPSR = this;
        myRootOrderIterator = 0;
        myVOIndexIterator = 0;
        myPlantcoordinator = plantcoordinator;
        std::vector<double> nulldefaultsvector; // PlantSummaries always have 0.0 as defaults
        sharedattributemanager->MakeAttributes(cd,
            std::string("PlantCoordinator"),
            std::string("PlantSummary"),
            variation_names,
            "",
            nulldefaultsvector
        );
        nonexistentCurrentPSR = 0;


        // send the PSR back to the requesting module, as an answer
        Use_PostOffice;
        if (psr_requesting_module != 0)
        {
            postoffice->sendMessage(plantcoordinator, psr_requesting_module, kScoreboardPlantSummaryRegisteredMessage, this);
        }

        return (return_value);
    }

    long int PlantSummaryRegistration::AlreadyMadeScoreboardSummaries
    (ProcessActionDescriptor* action,
        PlantCoordinator* plantcoordinator
    )
    {
        Use_ReturnValue;
        // Just send this back to the requesting module, as an answer
        Use_PostOffice;
        if (psr_requesting_module != 0)
        {
            postoffice->sendMessage(plantcoordinator, action->GetSource(), kScoreboardPlantSummaryRegisteredMessage, this);
        }
        return return_value;
    }

    bool PlantSummaryRegistration::GetNewPlantSummaryInformation
    (long int& rootOrder,
        Plant*& plant,
        PlantSet& plants,
        size_t& voIndex
    )
    {
        bool increment_plant = false;
        bool doContinue = true;
        bool rootOrdersWrappedAround = false;

        if (psr_all_branch_order_combinations)
        {
            rootOrder = myRootOrderIterator;

            // MSA 11.01.18 Note that we only want to go to the next RootOrder if we've covered all VolumeObjects (if applicable).

            if (!psr_all_volume_object_combinations || (psr_all_volume_object_combinations && myVOIndexIterator == static_cast<size_t>(psr_number_of_spatial_subsections - 1)))
            {
                ++myRootOrderIterator; // redundant but avoids those nasty side-effects
                if (myRootOrderIterator > 3)
                {
                    myRootOrderIterator = 0;
                    increment_plant = true;
                    rootOrdersWrappedAround = true;
                }
                else
                {
                }
            }
            doContinue = true;
        }
        else
        {
            rootOrder = -1;
            doContinue = false;
        }

        if (psr_all_volume_object_combinations)
        {
            voIndex = myVOIndexIterator;
            ++myVOIndexIterator;
            if (myVOIndexIterator >= static_cast<size_t>(psr_number_of_spatial_subsections))
            {
                myVOIndexIterator = 0;
                // Must ensure that all root orders have been covered, too, if applicable
                if (!psr_all_branch_order_combinations || (psr_all_branch_order_combinations && rootOrdersWrappedAround))
                {
                    increment_plant = true;
                }
            }
            else
            {
            }
            doContinue = true;
        }
        else
        {
            voIndex = 0;
            doContinue = false;
        }

        if (!psr_all_branch_order_combinations && !psr_all_volume_object_combinations)
        {
            increment_plant = true;
        }

        if (psr_all_plants_individually)
        {
            plants.clear();
            plant = myPlant;

            if (increment_plant)
            {
                ++myPlantIterator;
                if (myPlantIterator != (myPlantcoordinator->GetPlants()).end())
                {
                    myPlant = *myPlantIterator;
                }
                else
                {
                    myPlant = 0;
                    doContinue = false;
                }
            }
            else
            {
            }
            //
            // update myPlant *after* the
        }
        else
        {
            plant = 0;
            // copies the plants from whichever collection type the plantcoordinator
            // returns as, to whichever we're expecting - in this case from vector to set
            plants.insert(myPlantcoordinator->GetPlants().begin(), myPlantcoordinator->GetPlants().end());
        }


        if (!doContinue)
        {
            myRootOrderIterator = 0;
            myVOIndexIterator = 0;
        }

        return doContinue;
    }


    bool operator==(const PlantSummaryRegistration& lhs,
        const PlantSummaryRegistration& rhs
        )
    {
        return (
            (lhs.psr_per_box == rhs.psr_per_box)
            && (lhs.psr_all_plants_individually == rhs.psr_all_plants_individually)
            && (lhs.psr_all_volume_object_combinations == rhs.psr_all_volume_object_combinations)
            && (lhs.psr_all_branch_order_combinations == rhs.psr_all_branch_order_combinations)
            && (lhs.psr_summary_type == rhs.psr_summary_type)
            && (lhs.psr_wrap == rhs.psr_wrap)
            );
    }
} /* namespace rootmap */


