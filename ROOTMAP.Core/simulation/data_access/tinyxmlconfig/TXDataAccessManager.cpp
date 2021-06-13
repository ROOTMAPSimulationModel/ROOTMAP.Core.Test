/////////////////////////////////////////////////////////////////////////////
// Name:        TXDataAccessManager.cpp
// Purpose:     Implementation of the TXDataAccessManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2009-07-19 19:34:59 +0800 (Sun, 19 Jul 2009) $
// $Revision: 72 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/tinyxmlconfig/TXDataAccessManager.h"
#include "simulation/data_access/tinyxmlconfig/TXConfigurationDataStore.h"
#include "simulation/data_access/tinyxmlconfig/TXScoreboardDA.h"
#include "simulation/data_access/tinyxmlconfig/TXScoreboardDataDA.h"
#include "simulation/data_access/tinyxmlconfig/TXPostOfficeDA.h"
#include "simulation/data_access/tinyxmlconfig/TXProcessDA.h"
#include "simulation/data_access/tinyxmlconfig/TXProcessSpecialDA.h"
#include "simulation/data_access/tinyxmlconfig/TXCharacteristicDA.h"
#include "simulation/data_access/tinyxmlconfig/TXPlantDA.h"
#include "simulation/data_access/tinyxmlconfig/TXUtility.h"
#include "simulation/data_access/tinyxmlconfig/TXTableDA.h"
#include "simulation/data_access/tinyxmlconfig/TXViewDA.h"
#include "simulation/data_access/tinyxmlconfig/TXOutputRuleDA.h"

#include "simulation/data_access/common/ITableDataUser.h"
#include "simulation/data_access/common/IViewDataUser.h"
#include "simulation/data_access/common/IView3DDataUser.h"
#include "simulation/data_access/input_scheme_parser/InputSchemeParser.h"
#include "simulation/data_access/common/ScoreboardDataPreviewer.h"
#include "simulation/data_access/common/PlantDataPreviewer.h"
#include "simulation/data_access/common/VolumeObjectDataPreviewer.h"
#include "simulation/data_access/common/ProcessDataPreviewer.h"
#include "simulation/data_access/common/WindowDataPreviewer.h"
#include "simulation/data_access/common/OutputDataPreviewer.h"

#include "simulation/common/IdentifierUtility.h"
#include "simulation/common/SimulationEngine.h"
#include "simulation/file/output/DataOutputCoordinator.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/process/common/ProcessCoordinator.h"
#include "simulation/process/interprocess_communication/PostOffice.h"
#include "simulation/process/plant/PlantCoordinator.h"
#include "simulation/process/plant/Plant.h"
#include "simulation/process/plant/PlantType.h"
#include "simulation/process/plant/PlantSummary.h"
#include "simulation/process/plant/PossibleAxis.h"
#include "simulation/process/shared_attributes/SharedAttributeManager.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"
#include "simulation/process/raytrace/RenderCoordinator.h"

#include "simulation/process/modules/VolumeObjectCoordinator.h"
#include "simulation/process/modules/BoundingCylinder.h"
#include "simulation/process/modules/BoundingRectangularPrism.h"

#include "core/common/RmAssert.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/utility/StringParserUtility.h"
#include "core/utility/Utility.h"

#include "tinyxml.h"
#include "boost/algorithm/string.hpp"


namespace rootmap
{
    RootMapLoggerDefinition(TXDataAccessManager);

    using StringParserUtility::ParseCommaSeparatedDoubles;

    TXDataAccessManager::TXDataAccessManager(std::string absoluteConfigDir, std::string absoluteOutputDir, std::string configFile, std::string randomSeed)
        : m_txDataStore(0)
        , m_outputDir(absoluteOutputDir)
        , m_randomSeed(randomSeed)
    {
        RootMapLoggerInitialisation("rootmap.TXDataAccessManager");
        m_txDataStore = new TXConfigurationDataStore(absoluteConfigDir, configFile);
        setDataStore(m_txDataStore);

        m_txDataStore->initialise();
    }

    TXDataAccessManager::~TXDataAccessManager()
    {
        wxDELETE(m_txDataStore);
    }

    // //////////////////////////////////////////////////////////////////////
    // 
    // Generic Template Functions
    // 
    // //////////////////////////////////////////////////////////////////////


    // //////////////////////////////////////////////////////////////////////
    // This is the generic template version of the GenericFunction
    template <class T>
    void TXDataAccessManager::GenericFunction(T* target, const char* ownerName, const char* childType, const char* itemName, HasOwnerDataFunction hasDataFunction, GetOwnerDataFunction getDataFunction)
    {
        if ((getConfigurationDataStore().*hasDataFunction)(ownerName))
        {
            NameLocationDataList generic_data = (getConfigurationDataStore().*getDataFunction)(ownerName);
            for (NameLocationDataList::iterator iter = generic_data.begin();
                iter != generic_data.end(); ++iter)
            {
                std::string location = (*iter).getLocation();
                if ((location == "local") || (location == "this"))
                {
                    // retrieve from the same xml doc as the configuration document
                    TiXmlElement* rootElement = m_txDataStore->getTxDocument()->RootElement();
                    RmAssert(strcmp(rootElement->Value(), "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

                    GenericSubFunction(target, ownerName, childType, itemName, rootElement, location.c_str());
                }
                else
                {
                    std::string configPath = m_txDataStore->getConfigDir() + "/" + location;

                    TiXmlDocument genericDoc(configPath.c_str());
                    bool loadOk = genericDoc.LoadFile();

                    RmAssert(loadOk, "The TinyXml document did not load ok");
                    if (loadOk)
                    {
                        RootMapLogInfo("Successfully loaded " << childType << " file \"" << configPath << "\".");
                    }
                    else
                    {
                        RootMapLogInfo("Could not load " << childType << " file \"" << configPath << "\", error=\"" << genericDoc.ErrorDesc() << "\"");
                        //TODO: throw a ConfigurationDataException with same string
                        return;
                    }

                    //fetch from file

                    TiXmlElement* rootElement = genericDoc.RootElement();
                    const char* rootValue = rootElement->Value();
                    RmAssert(0 == strcmp(rootValue, "rootmap"), "Root node of any rootmap xml file must be \"rootmap\"");

                    GenericSubFunction(target, ownerName, childType, itemName, rootElement, location.c_str());

                    // i *thought* tinyxml would do this on destruction, however
                    // the TiXmlDocument destructor is empty ("{}").
                    genericDoc.Clear();
                }
            }
        }
    }


    // //////////////////////////////////////////////////////////////////////
    // This is the generic template version of the GenericSubFunction - applies to all types
    // MSA 09.10.05 This function intentionally assigns to a pointer within a conditional statement.
    // Suppressing the MSVC++ warning that this causes:
#pragma warning( push )
#pragma warning( disable: 4706 )
    template <class T>
    void TXDataAccessManager::GenericSubFunction(T* target, const char* ownerName, const char* childType, const char* itemName, TiXmlElement* rootElement, const char* location)
    {
        TiXmlNode* genericNode = 0;
        int num_generic_nodes = 0;

        // for each scoreboard construct-able item
        while (genericNode = rootElement->IterateChildren(childType, genericNode))
        {
            ++num_generic_nodes;
            // The configuration elements are name, type, owner, location
            //
            TiXmlElement* genericElem = genericNode->ToElement();
            if (genericElem != 0)
            {
                std::string owner_str = TXUtility::getChildElementText(genericElem, "owner");
                if (owner_str == ownerName)
                {
                    // MSA 09.10.05    Commenting out this unused variable to silence compiler warning
                    //int num_sub_nodes = 0;
                    TiXmlNode* sub_node = 0;
                    // delve further into this element
                    while (sub_node = genericElem->IterateChildren(itemName, sub_node))
                    {
                        TiXmlElement* sub_element = sub_node->ToElement();

                        if (0 != sub_element)
                        {
                            GenericElementFunction(target, sub_element, location);
                        }
                        else
                        {
                            RootMapLogError("Could not convert " << childType << " Node " << itemName << " to Element, File \"" << location << "\" [Line " << sub_node->Row() << ",Col " << sub_node->Column() << "]");
                        }
                    }
                } // if (owner_str == ownerName)
                else
                {
                    // move right along to the next generic element
                    continue;
                } // if (owner_str != ownerName)
            } // if (genericElem != 0)
            else
            {
                RootMapLogError("Could not convert " << childType << " Node to Element, File \"" << location << "\" [Line " << genericNode->Row() << ",Col " << genericNode->Column() << "]");
            } // else (genericElem == 0)
        } // while (genericNode
    }
#pragma warning( pop )


    // //////////////////////////////////////////////////////////////////////
    // This is the generic template version of the ConstructElementFunction
    //
    // The only reason at all for a template function of this type is to allow
    // it to be called from the generic ConstructSubFunction implementation.
    //
    // However since there is no real way to implement this *generically*, the
    // generic implementation merely asserts to let the developer know that an
    // explicit specialization is required to be implemented.
    //
    template <class T>
    void TXDataAccessManager::GenericElementFunction(T* /*target*/, TiXmlElement* /*element*/, const char* /*location*/)
    {
        RmAssert(false, "No specialisation of GenericElementFunction template was provided for accessed type");
    }


    // //////////////////////////////////////////////////////////////////////
    //
    // Specialized Template Functions
    //
    // //////////////////////////////////////////////////////////////////////


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for ScoreboardDataPreviewer
    template <>
    void TXDataAccessManager::GenericElementFunction(ScoreboardDataPreviewer* sdp, TiXmlElement* element, const char* location)
    {
        TXScoreboardDA scoreboard_data;

        // Attempt to create Scoreboards
        try
        {
            std::string name = TXUtility::getChildElementText(element, "name");
            std::string stratum = TXUtility::getChildElementText(element, "stratum");

            scoreboard_data.setName(name);
            scoreboard_data.setStratum(stratum);

            TiXmlNode* boundary_node = 0;
            // delve further into this element

            while (boundary_node = element->IterateChildren("boundaryarray", boundary_node))
            {
                TiXmlElement* boundary_elem = boundary_node->ToElement();

                if (0 != boundary_elem)
                {
                    std::string dimension_str = TXUtility::getChildElementText(boundary_elem, "dimension");
                    const char* position_array = TXUtility::getChildElementText(boundary_elem, "positionarray");

                    Dimension dimension = Utility::StringToDimension(dimension_str[0]);
                    scoreboard_data.setBoundaryValueVector(position_array, dimension);
                }
                else
                {
                    RootMapLogError("Could not convert boundaryarray Node to Element, File \"" << location << "\" [Line " << boundary_node->Row() << ",Col " << boundary_node->Column() << "]");
                } // else (boundary_elem == 0)
            } // while (boundary_node)

            if (scoreboard_data.isValid())
            {
                // bells, whistles, applause
                sdp->Populate(scoreboard_data);
            }
            else
            {
                RootMapLogError("Scoreboard construction data in file \"" << location << "\" was not valid");
            }
        }

        catch (...)
        {
            RootMapLogError("XML data in file \"" << location << "\" was not valid");
        }
    }

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for PlantDataPreviewer
    template <>
    void TXDataAccessManager::GenericElementFunction(PlantDataPreviewer* pdp, TiXmlElement* element, const char* location)
    {
        if (0 == strcmp(element->Value(), "planttype"))
        {
            PreviewPlantType(pdp, element, location);
        }
        else if (0 == strcmp(element->Value(), "plant"))
        {
            PreviewPlant(pdp, element, location);
        }
        else if (0 == strcmp(element->Value(), "plantsummary"))
        {
            PreviewPlantSummary(pdp, element, location);
        }
        else
        {
            RootMapLogError("PlantCoordinator construction data in file \"" << location << "\" was not valid");
        }
    }

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for ProcessDataPreviewer
    // MSA 09.10.05 This function intentionally assigns to a pointer within a conditional statement.
    // Suppressing the MSVC++ warning that this causes:
#pragma warning( push )
#pragma warning( disable: 4706 )
    template <>
    void TXDataAccessManager::GenericElementFunction(ProcessDataPreviewer* pdp, TiXmlElement* element, const char* location)
    {
        std::string name = TXUtility::getChildElementText(element, "name");
        if (name.compare(pdp->GetProcessName()) == 0)
        {
            // MSA 11.06.06 These Process properties aren't [currently] editable by the config wizard that this function feeds.
            /*
            bool doesOverride = TXUtility::getChildElementBool(element, "override");
            const char * stratum_str = TXUtility::getChildElementText(element, "scoreboard");
            ScoreboardStratum stratum = ScoreboardStratum::StringToStratum(stratum_str);
            ProcessActivity activity_type = Utility::StringToOSType(TXUtility::getChildElementText(element, "activitytype"));
            ProcessIdentifier pid = IdentifierUtility::instance().useNextProcessIdentifier(stratum);

            RootMapLogDebug("Process Data found, name=\"" << name << "\" stratum=" << stratum_str << "(" << stratum.value() << ") pid=" << pid);

            TXProcessDA process_data(name,doesOverride,pid,stratum,activity_type);
            */
            TiXmlNode* characteristic_node = 0;
            // delve further into this element
            while (characteristic_node = element->IterateChildren("characteristic", characteristic_node))
            {
                TXCharacteristicDA* cda = CreateCharacteristicDA(characteristic_node, StratumALL, location);
                if ((0 != cda) && (cda->isValid()))
                {
                    pdp->AddCharacteristicSummary(cda->getName() + " (" + cda->getUnits() + ")", cda->getMinimum(), cda->getDefault(), cda->getMaximum());
                }
                delete cda;
            }
        }
    }
#pragma warning( pop )

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for scoreboard coordinator
    // MSA 09.10.05 This function intentionally assigns to a pointer within a conditional statement.
    // Suppressing the MSVC++ warning that this causes:
#pragma warning( push )
#pragma warning( disable: 4706 )
    template <>
    void TXDataAccessManager::GenericElementFunction(ScoreboardCoordinator* scoord, TiXmlElement* element, const char* location)
    {
        TXScoreboardDA scoreboard_data;

        // Attempt to create Scoreboards
        try
        {
            std::string name = TXUtility::getChildElementText(element, "name");
            std::string stratum = TXUtility::getChildElementText(element, "stratum");

            scoreboard_data.setName(name);
            scoreboard_data.setStratum(stratum);

            TiXmlNode* boundary_node = 0;
            // delve further into this element

            while (boundary_node = element->IterateChildren("boundaryarray", boundary_node))
            {
                TiXmlElement* boundary_elem = boundary_node->ToElement();

                if (0 != boundary_elem)
                {
                    std::string dimension_str = TXUtility::getChildElementText(boundary_elem, "dimension");
                    const char* position_array = TXUtility::getChildElementText(boundary_elem, "positionarray");

                    Dimension dimension = Utility::StringToDimension(dimension_str[0]);
                    scoreboard_data.setBoundaryValueVector(position_array, dimension);
                }
                else
                {
                    RootMapLogError("Could not convert boundaryarray Node to Element, File \"" << location << "\" [Line " << boundary_node->Row() << ",Col " << boundary_node->Column() << "]");
                } // else (boundary_elem == 0)
            } // while (boundary_node)

            if (scoreboard_data.isValid())
            {
                // bells, whistles, applause
                scoord->createScoreboard(scoreboard_data);
            }
            else
            {
                RootMapLogError("Scoreboard construction data in file \"" << location << "\" was not valid");
            }
        }
        catch (...)
        {
            RootMapLogError("Scoreboard construction data in file \"" << location << "\" was not valid");
        }
    }
#pragma warning( pop )


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for process coordinator
    // MSA 09.10.05 This function intentionally assigns to a pointer within a conditional statement.
    // Suppressing the MSVC++ warning that this causes:
#pragma warning( push )
#pragma warning( disable: 4706 )
    template <>
    void TXDataAccessManager::GenericElementFunction(ProcessCoordinator* pcoord, TiXmlElement* element, const char* location)
    {
        std::string name = TXUtility::getChildElementText(element, "name");
        // OMG OMG OMG !!! 
        // "override" is a microsoft C++/CLI (extended c++) keyword !!!
        // Apparently it is still contextually safe to use as an identifier
        // http://blogs.msdn.com/hsutter/archive/2003/11/23/53519.aspx
        // but i can't see the point in risking that supposed safety, given
        // option of using a more meaningful identifier anyway.
        bool doesOverride = TXUtility::getChildElementBool(element, "override");
        const char* stratum_str = TXUtility::getChildElementText(element, "scoreboard");
        ScoreboardStratum stratum = ScoreboardStratum::StringToStratum(stratum_str);
        ProcessActivity activity_type = Utility::StringToOSType(TXUtility::getChildElementText(element, "activitytype"));
        ProcessIdentifier pid = IdentifierUtility::instance().useNextProcessIdentifier(stratum);

        RootMapLogDebug("Process Data found, name=\"" << name << "\" stratum=" << stratum_str << "(" << stratum.value() << ") pid=" << pid);

        TXProcessDA process_data(name, doesOverride, pid, stratum, activity_type);

        TiXmlNode* characteristic_node = 0;
        // delve further into this element
        while (characteristic_node = element->IterateChildren("characteristic", characteristic_node))
        {
            // MSA These pointers are to free store objects - delete them in ~TXProcessDA?
            TXCharacteristicDA* characteristic_data = CreateCharacteristicDA(characteristic_node, stratum, location);
            wxString characteristicName = characteristic_data->getName();
            if (characteristicName.Replace(" VolumeObject [none]", "") > 0)
            {
                RootMapLogAlert("Warning: obsolete characteristic name '" << characteristicName << " VolumeObject [none]' found. Replacing with '" << characteristicName << "'.");
                characteristic_data->setName(std::string(characteristicName));
            }
            if ((0 != characteristic_data) && (characteristic_data->isValid()))
            {
                //RootMapLogInfo("Adding characteristic \"%s\", min %f, max %f, default %f"), name, minimum, maximum, default_);
                process_data.addCharacteristicDA(characteristic_data);
            }
            else
            {
                delete characteristic_data;
            }
        } // while (boundary_node = 

        if (process_data.isValid())
        {
            // bells, whistles, applause
            pcoord->createProcess(process_data);
        }
        else
        {
            RootMapLogError("Process construction data in file \"" << location << "\" was not valid");
        }
    }
#pragma warning( pop )


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for plant coordinator
    template <>
    void TXDataAccessManager::GenericElementFunction(PlantCoordinator* plantcoord, TiXmlElement* element, const char* location)
    {
        if (0 == strcmp(element->Value(), "planttype"))
        {
            CreatePlantType(plantcoord, element, location);
        }
        else if (0 == strcmp(element->Value(), "plant"))
        {
            CreatePlant(plantcoord, element, location);
        }
        else if (0 == strcmp(element->Value(), "plantsummary"))
        {
            CreatePlantSummary(plantcoord, element, location);
        }
        else
        {
            RootMapLogError("PlantCoordinator construction data in file \"" << location << "\" was not valid");
        }
    }

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for VolumeObjectDataPreviewer
    template <>
    void TXDataAccessManager::GenericElementFunction(VolumeObjectDataPreviewer* vodp, TiXmlElement* element, const char* location)
    {
        if (0 == strcmp(element->Value(), "volumeobject"))
        {
            PreviewVolumeObject(vodp, element, location);
        }
        else
        {
            RootMapLogError("VolumeObjectCoordinator construction data in file \"" << location << "\" was not valid");
        }
    }

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for VolumeObject coordinator
    template <>
    void TXDataAccessManager::GenericElementFunction(VolumeObjectCoordinator* vocoord, TiXmlElement* element, const char* location)
    {
        if (0 == strcmp(element->Value(), "volumeobject"))
        {
            CreateVolumeObject(vocoord, element, location);
        }
        else
        {
            RootMapLogError("VolumeObjectCoordinator construction data in file \"" << location << "\" was not valid");
        }
    }


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for Render Coordinator
    template <>
    void TXDataAccessManager::GenericElementFunction(RenderCoordinator* rcoord, TiXmlElement* element, const char* /* location */)
    {
        TiXmlNode* characteristic_node = element;

        ViewDAI::CharacteristicColourData ccd;
        GetCharacteristicColourData(characteristic_node, ccd);
        rcoord->AddCharacteristicColourData(ccd);
    }

    void TXDataAccessManager::PreviewPlantType(PlantDataPreviewer* pdp, TiXmlElement* element, const char* /* location */)
    {
        /*
        <name>RootMap Plant Type</name>
        <roots_to_foliage_ratio>1.0</roots_to_foliage_ratio>
        <vegetate_to_reproduce_ratio>1.0</vegetate_to_reproduce_ratio>
        <structure_to_photosynthesize_ratio>1.0</structure_to_photosynthesize_ratio>
        <initial_seminal_deflection>1.0</initial_seminal_deflection>
        <germination_lag>1.0</germination_lag>
        <first_seminal_probability>1.0</first_seminal_probability>
        <temperature_of_zero_growth>7.0</temperature_of_zero_growth>
        */
        try
        {
            std::string name = TXUtility::getChildElementText(element, "name");
            // MSA 09.10.05 Commenting out these currently-unused variable initialisations to silence compiler warnings
            /*
            double roots_to_foliage_ratio = TXUtility::getChildElementDouble(element, "roots_to_foliage_ratio");
            double vegetate_to_reproduce_ratio = TXUtility::getChildElementDouble(element, "vegetate_to_reproduce_ratio");
            double structure_to_photosynthesize_ratio = TXUtility::getChildElementDouble(element, "structure_to_photosynthesize_ratio");
            */
            double initial_seminal_deflection = TXUtility::getChildElementDouble(element, "initial_seminal_deflection");
            double germination_lag = TXUtility::getChildElementDouble(element, "germination_lag");
            double first_seminal_probability = TXUtility::getChildElementDouble(element, "first_seminal_probability");
            double temperature_of_zero_growth = TXUtility::getChildElementDouble(element, "temperature_of_zero_growth");

            PlantTypeIdentifier new_id = IdentifierUtility::instance().useNextPlantTypeIdentifier();

            PlantType* ptype = new PlantType(name, new_id, initial_seminal_deflection, germination_lag, first_seminal_probability, temperature_of_zero_growth);

            pdp->AddPlantType(ptype);
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
            return;
        }
    }

    void TXDataAccessManager::PreviewPlant(PlantDataPreviewer* pdp, TiXmlElement* element, const char* location)
    {
        try
        {
            std::string name = TXUtility::getChildElementText(element, "name");
            std::string type_name = TXUtility::getChildElementText(element, "type");

            // <origin>
            std::vector<double> origin_vector;
            TXUtility::getChildElementDoubleArray(element, "origin", origin_vector);
            if (3 != origin_vector.size())
            {
                RootMapLogError("Invalid Plant origin element. Expected 3 comma-separated doubles, got: " << origin_vector.size());
                return;
            }
            DoubleCoordinate origin(origin_vector[0], origin_vector[1], origin_vector[2]);

            // seed_location
            std::vector<double> seedlocation_vector;
            TXUtility::getChildElementDoubleArray(element, "seed_location", seedlocation_vector);
            if (3 != seedlocation_vector.size())
            {
                RootMapLogError("Invalid Plant seed_location element. Expected 3 comma-separated doubles, got: " << seedlocation_vector.size());
                return;
            }
            DoubleCoordinate seed_location(seedlocation_vector[0], seedlocation_vector[1], seedlocation_vector[2]);

            double seeding_time = TXUtility::getChildElementDouble(element, "seeding_time");

            PossibleAxis* seminal_axes = CreatePossibleAxes(element, location, "seminal_axis");
            PossibleAxis* nodal_axes = CreatePossibleAxes(element, location, "nodal_axis");

            // Important note: PlantDataPreviewer does not use PlantIdentifier.

            TXPlantDA* plant_da = new TXPlantDA(name, type_name, /*PlantIdentifier==*/-1, StratumALL /*ScoreboardStratum::ALL*/, paGrowing,
                origin, seed_location, seeding_time, seminal_axes, nodal_axes);
            TogglablePlantDAI* togglableDai = new TogglablePlantDAI();
            togglableDai->plantDAI = plant_da;
            pdp->AddPlant(togglableDai);
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
            return;
        }
    }

    void TXDataAccessManager::PreviewPlantSummary(PlantDataPreviewer* /* pdp */, TiXmlElement* /* element */, const char* /* location */)
    {
        RootMapLogWarn("PreviewPlantSummary not implemented");
    }

    void TXDataAccessManager::CreatePlantType(PlantCoordinator* pcoord, TiXmlElement* element, const char* /* location */)
    {
        /*
        <name>RootMap Plant Type</name>
        <roots_to_foliage_ratio>1.0</roots_to_foliage_ratio>
        <vegetate_to_reproduce_ratio>1.0</vegetate_to_reproduce_ratio>
        <structure_to_photosynthesize_ratio>1.0</structure_to_photosynthesize_ratio>
        <initial_seminal_deflection>1.0</initial_seminal_deflection>
        <germination_lag>1.0</germination_lag>
        <first_seminal_probability>1.0</first_seminal_probability>
        <temperature_of_zero_growth>7.0</temperature_of_zero_growth>
        */
        try
        {
            std::string name = TXUtility::getChildElementText(element, "name");
            // MSA 09.10.05 Commenting out these currently-unused variable initialisations to silence compiler warnings
            /*
            double roots_to_foliage_ratio = TXUtility::getChildElementDouble(element, "roots_to_foliage_ratio");
            double vegetate_to_reproduce_ratio = TXUtility::getChildElementDouble(element, "vegetate_to_reproduce_ratio");
            double structure_to_photosynthesize_ratio = TXUtility::getChildElementDouble(element, "structure_to_photosynthesize_ratio");
            */
            double initial_seminal_deflection = TXUtility::getChildElementDouble(element, "initial_seminal_deflection");
            double germination_lag = TXUtility::getChildElementDouble(element, "germination_lag");
            double first_seminal_probability = TXUtility::getChildElementDouble(element, "first_seminal_probability");
            double temperature_of_zero_growth = TXUtility::getChildElementDouble(element, "temperature_of_zero_growth");

            PlantTypeIdentifier new_id = IdentifierUtility::instance().useNextPlantTypeIdentifier();

            PlantType* ptype = new PlantType(name, new_id, initial_seminal_deflection, germination_lag, first_seminal_probability, temperature_of_zero_growth);

            pcoord->AddPlantType(ptype);
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
            return;
        }
    }

    void TXDataAccessManager::CreatePlant(PlantCoordinator* pcoord, TiXmlElement* element, const char* location)
    {
        try
        {
            std::string name = TXUtility::getChildElementText(element, "name");
            std::string type_name = TXUtility::getChildElementText(element, "type");

            // <origin>
            std::vector<double> origin_vector;
            TXUtility::getChildElementDoubleArray(element, "origin", origin_vector);
            if (3 != origin_vector.size())
            {
                RootMapLogError("Invalid Plant origin element. Expected 3 comma-separated doubles, got: " << origin_vector.size());
                return;
            }
            DoubleCoordinate origin(origin_vector[0], origin_vector[1], origin_vector[2]);

            // seed_location
            std::vector<double> seedlocation_vector;
            TXUtility::getChildElementDoubleArray(element, "seed_location", seedlocation_vector);
            if (3 != seedlocation_vector.size())
            {
                RootMapLogError("Invalid Plant seed_location element. Expected 3 comma-separated doubles, got: " << seedlocation_vector.size());
                return;
            }
            DoubleCoordinate seed_location(seedlocation_vector[0], seedlocation_vector[1], seedlocation_vector[2]);

            double seeding_time = TXUtility::getChildElementDouble(element, "seeding_time");

            PossibleAxis* seminal_axes = CreatePossibleAxes(element, location, "seminal_axis");
            PossibleAxis* nodal_axes = CreatePossibleAxes(element, location, "nodal_axis");

            // can't use useNextPlantIdentifier(): this confuses the FindProcessByProcessID routines
            PlantIdentifier new_id = IdentifierUtility::instance().useNextProcessIdentifier(StratumALL /*ScoreboardStratum::ALL*/);

            TXPlantDA plant_da(name, type_name, new_id, StratumALL /*ScoreboardStratum::ALL*/, paGrowing,
                origin, seed_location, seeding_time, seminal_axes, nodal_axes);

            Plant* plant = new Plant(*pcoord);

            plant->Initialise(plant_da);

            pcoord->AddPlant(plant);
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
            return;
        }
    }

    // MSA 09.10.05 This function intentionally assigns to a pointer within a conditional statement.
    // Suppressing the MSVC++ warning that this causes:
#pragma warning( push )
#pragma warning( disable: 4706 )
    PossibleAxis* TXDataAccessManager::CreatePossibleAxes(TiXmlElement* element, const char* /* location */, const char* axistype)
    {
        PossibleAxis* p_axis = 0;
        TiXmlNode* axis_node = 0;

        while (axis_node = element->IterateChildren(axistype, axis_node))
        {
            double start_lag = TXUtility::getChildElementDouble(axis_node, "start_lag");
            double probability = TXUtility::getChildElementDouble(axis_node, "probability");

            PossibleAxis* new_axis = new PossibleAxis(start_lag, probability);

            if (0 == p_axis)
            {
                p_axis = new_axis;
            }
            else
            {
                p_axis->Push(new_axis);
            }
        }

        return p_axis;
    }
#pragma warning( pop )

    void TXDataAccessManager::CreatePlantSummary(PlantCoordinator* /* pcoord */, TiXmlElement* /* element */, const char* /* location */)
    {
        RootMapLogWarn("CreatePlantSummary not implemented");
    }

    void TXDataAccessManager::PreviewVolumeObject(VolumeObjectDataPreviewer* vodp, TiXmlElement* element, const char* /* location */)
    {
        static const std::string bcname = "BoundingCylinder";
        static const std::string brpname = "BoundingRectangularPrism";
        const std::string class_name = TXUtility::getChildElementText(element, "class_name");

        // Root Penetration Probability defaults to 0.0, if not found
        double topRPP = 0;
        double bottomRPP = 0;
        double sideRPP = 0;
        // Default Deflection Probability Calculation Algorithm is CosinProportional
        std::string dpca = "cosin proportional";

        TiXmlElement* rppElem = element->FirstChildElement("root_penetration_probability");
        if (rppElem != __nullptr)
        {
            // Do not throw an exception on __nullptr; default to 0.0.
            topRPP = TXUtility::getChildElementDouble(rppElem, "top", false, 0.0);
            bottomRPP = TXUtility::getChildElementDouble(rppElem, "bottom", false, 0.0);
            sideRPP = TXUtility::getChildElementDouble(rppElem, "sides", false, 0.0);
            const char* temp = TXUtility::getChildElementText(rppElem, "probability_calculation_algorithm", false);
            if (temp) dpca = std::string(temp);
        }

        // Ditto Permeability
        double topP = 0;
        double bottomP = 0;
        double sideP = 0;

        TiXmlElement* pElem = element->FirstChildElement("permeability");
        if (pElem != __nullptr)
        {
            // Do not throw an exception on __nullptr; default to 0.0.
            topP = TXUtility::getChildElementDouble(pElem, "top", false, 0.0);
            bottomP = TXUtility::getChildElementDouble(pElem, "bottom", false, 0.0);
            sideP = TXUtility::getChildElementDouble(pElem, "sides", false, 0.0);
        }

        if (class_name == bcname)
        {
            const char* origin_string = TXUtility::getChildElementText(element, "origin");
            std::vector<double> coordinates;
            ParseCommaSeparatedDoubles(origin_string, coordinates);
            DoubleCoordinate origin(coordinates[0], coordinates[1], coordinates[2]);
            const double depth = TXUtility::getChildElementDouble(element, "depth");
            const double radius = TXUtility::getChildElementDouble(element, "radius");
            BoundingCylinder* new_bounding_cylinder = new BoundingCylinder(0, origin, depth, radius, dpca, topRPP, bottomRPP, sideRPP, topP, bottomP, sideP);
            vodp->AddBoundingCylinder(new_bounding_cylinder);
        }
        else if (class_name == brpname)
        {
            const char* lft = TXUtility::getChildElementText(element, "leftfronttop");
            std::vector<double> lft_coordinates;
            ParseCommaSeparatedDoubles(lft, lft_coordinates);
            DoubleCoordinate lftDC(lft_coordinates[0], lft_coordinates[1], lft_coordinates[2]);
            const char* rbb = TXUtility::getChildElementText(element, "rightbackbottom");
            std::vector<double> rbb_coordinates;
            ParseCommaSeparatedDoubles(rbb, rbb_coordinates);
            DoubleCoordinate rbbDC(rbb_coordinates[0], rbb_coordinates[1], rbb_coordinates[2]);
            BoundingRectangularPrism* new_bounding_rectangular_prism = new BoundingRectangularPrism(0, lftDC, rbbDC, dpca, topRPP, bottomRPP, sideRPP, topP, bottomP, sideP);
            vodp->AddBoundingRectangularPrism(new_bounding_rectangular_prism);
        }
    }


    void TXDataAccessManager::CreateVolumeObject(VolumeObjectCoordinator* vocoord, TiXmlElement* element, const char* /* location */)
    {
        RootMapLogDebug("Creating a VolumeObject.");
        static const std::string bcname = "BoundingCylinder";
        static const std::string brpname = "BoundingRectangularPrism";
        const std::string class_name = TXUtility::getChildElementText(element, "class_name");

        RootMapLogDebug("class name:" + class_name);
        // Root Penetration Probability defaults to 0.0, if not found
        double topRPP = 0;
        double bottomRPP = 0;
        double sideRPP = 0;
        // Default Deflection Probability Calculation Algorithm is CosinProportional
        std::string dpca = "cosin proportional";

        TiXmlElement* rppElem = element->FirstChildElement("root_penetration_probability");
        if (rppElem != __nullptr)
        {
            // Do not throw an exception on __nullptr; default to 0.0.
            topRPP = TXUtility::getChildElementDouble(rppElem, "top", false, 0.0);
            bottomRPP = TXUtility::getChildElementDouble(rppElem, "bottom", false, 0.0);
            sideRPP = TXUtility::getChildElementDouble(rppElem, "sides", false, 0.0);
            const char* temp = TXUtility::getChildElementText(rppElem, "probability_calculation_algorithm", false);
            if (temp) dpca = std::string(temp);
        }

        // Ditto Permeability
        double topP = 0;
        double bottomP = 0;
        double sideP = 0;

        TiXmlElement* pElem = element->FirstChildElement("permeability");
        if (pElem != __nullptr)
        {
            // Do not throw an exception on __nullptr; default to 0.0.
            topP = TXUtility::getChildElementDouble(pElem, "top", false, 0.0);
            bottomP = TXUtility::getChildElementDouble(pElem, "bottom", false, 0.0);
            sideP = TXUtility::getChildElementDouble(pElem, "sides", false, 0.0);
        }


        RootMapLogDebug("Have set the RPP and permeability. Next gonna instantiate the appropriate class. Uh oh, there's some boost here!");

        if (class_name == bcname)
        {

            RootMapLogDebug("It's a boundingcylinder.");
            const char* origin_string = TXUtility::getChildElementText(element, "origin");
            std::vector<double> coordinates;
            ParseCommaSeparatedDoubles(origin_string, coordinates);
            DoubleCoordinate origin(coordinates[0], coordinates[1], coordinates[2]);
            const double depth = TXUtility::getChildElementDouble(element, "depth");
            const double radius = TXUtility::getChildElementDouble(element, "radius");

            RootMapLogDebug("gonna new that BC up...");
            BoundingCylinder* new_bounding_cylinder = new BoundingCylinder(vocoord->GetNextVOIndex(), origin, depth, radius, dpca, topRPP, bottomRPP, sideRPP, topP, bottomP, sideP);

            RootMapLogDebug("Done. Adding it.");
            vocoord->AddVolumeObject(new_bounding_cylinder);

            RootMapLogDebug("Done adding it to the VO Coordinator.");
        }
        else if (class_name == brpname)
        {
            RootMapLogDebug("It's a boundingrectangularprism.");
            const char* lft = TXUtility::getChildElementText(element, "leftfronttop");
            std::vector<double> lft_coordinates;
            ParseCommaSeparatedDoubles(lft, lft_coordinates);
            DoubleCoordinate lftDC(lft_coordinates[0], lft_coordinates[1], lft_coordinates[2]);
            const char* rbb = TXUtility::getChildElementText(element, "rightbackbottom");
            std::vector<double> rbb_coordinates;
            ParseCommaSeparatedDoubles(rbb, rbb_coordinates);
            DoubleCoordinate rbbDC(rbb_coordinates[0], rbb_coordinates[1], rbb_coordinates[2]);
            RootMapLogDebug("gonna new that BRP up...");

            BoundingRectangularPrism* new_bounding_rectangular_prism = new BoundingRectangularPrism(vocoord->GetNextVOIndex(), lftDC, rbbDC, dpca, topRPP, bottomRPP, sideRPP, topP, bottomP, sideP);
            RootMapLogDebug("Done. Adding it.");

            vocoord->AddVolumeObject(new_bounding_rectangular_prism);
            RootMapLogDebug("Done adding it to the VO Coordinator.");

        }
    }

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for OutputDataPreviewer
    template <>
    void TXDataAccessManager::GenericElementFunction(OutputDataPreviewer* odp, TiXmlElement* element, const char* location)
    {
        // Mandatory elements (exceptions will be thrown if not found)
        std::string name = TXUtility::getChildElementText(element, "source");
        std::string type = TXUtility::getChildElementText(element, "type");
        ScoreboardStratum stratum = ScoreboardStratum::StringToStratum(TXUtility::getChildElementText(element, "stratum"));
        std::string reopen_str = TXUtility::getChildElementText(element, "reopen");
        std::string filename = TXUtility::getChildElementText(element, "filename");

        // Elements that may or may not be required
        std::string characteristic = "";
        const char* characteristic_raw = TXUtility::getChildElementText(element, "characteristic", false);
        if (__nullptr != characteristic_raw)
        {
            characteristic = characteristic_raw;
        }
        // If characteristic tag not found, RaytracerData and ViewCapture are the only valid types
        else
        {
            RmAssert("RaytracerData" == type || "ViewCapture" == type, "Characteristic tag not found for Output Rule");
        }

        std::string specification1 = "";
        const char* specification1_raw = TXUtility::getChildElementText(element, "specification1", false);
        if (__nullptr != specification1_raw)
        {
            specification1 = specification1_raw;
        }

        std::string specification2 = "";
        const char* specification2_raw = TXUtility::getChildElementText(element, "specification2", false);
        if (__nullptr != specification2_raw)
        {
            specification2 = specification2_raw;
        }

        //Finished getting mandatory & possibly-optional elements------------------------------------------

        TiXmlNode* when_node = 0;
        // delve further into this element
        if (0 != (when_node = element->FirstChild("when")))
        {
            TiXmlElement* when_elem = when_node->ToElement();
            try
            {
                if (0 != when_elem)
                {
                    int count = TXUtility::getChildElementInt(when_elem, "count", false, 0);

                    const char* interval = TXUtility::getChildElementText(when_elem, "interval", false);
                    const char* initial_time = TXUtility::getChildElementText(when_elem, "initialtime", false);
                    const char* at_time = TXUtility::getChildElementText(when_elem, "attime", false);
                    const char* regularity = TXUtility::getChildElementText(when_elem, "regularity", false);

                    // either interval+initialtime OR attime+regularity are required.

                    // MSA 11.08.12 TODO Implement FixedPerRun.

                    // At_time:
                    if ((0 != at_time) && (0 != regularity))
                    {
                        std::vector<long> at_time_v;
                        StringParserUtility::ParseCommaSeparatedLongs(at_time, at_time_v);
                        if (at_time_v.size() != 6)
                        {
                            RootMapLogError("OutputRule \"attime\" element, File \"" << location << "\" encountered " << at_time << ", must contain 6 valid integer values {y,m,d,H,M,S}");
                            return;
                        }

                        OutputRuleDAI::RegularWhen::ERegularity regularity_val = OutputRuleDAI::RegularWhen::unset;
                        switch (*regularity)
                        {
                        case 'y':
                            regularity_val = OutputRuleDAI::RegularWhen::r_year;
                            break;
                        case 'm':
                            regularity_val = OutputRuleDAI::RegularWhen::r_month;
                            break;
                        case 'd':
                            regularity_val = OutputRuleDAI::RegularWhen::r_day;
                            break;
                        case 'H':
                            regularity_val = OutputRuleDAI::RegularWhen::r_hour;
                            break;
                        case 'M':
                            regularity_val = OutputRuleDAI::RegularWhen::r_minute;
                            break;
                        case 'S':
                            regularity_val = OutputRuleDAI::RegularWhen::r_second;
                            break;
                        default:
                            RootMapLogError("OutputRule \"when:regularity\" element, File \"" << location << "\" encountered " << regularity << ", must contain one of {y,m,d,H,M,S}");
                            break;
                        }

                        TXOutputRuleDA* data = new TXOutputRuleDA(name,
                            type,
                            characteristic,
                            stratum,
                            filename,
                            m_outputDir,
                            specification1,
                            specification2,
                            reopen_str);
                        data->SetRegularWhen(count,
                            at_time_v[0], at_time_v[1], at_time_v[2],
                            at_time_v[3], at_time_v[4], at_time_v[5],
                            regularity_val);

                        odp->AddOutputRule(data); // Passes ownership of the TXOutputRuleDA pointer to OutputDataPreviewer.
                    }
                    // Not At_time (may yet be invalid):
                    else
                    {
                        ProcessTime_t initial_time_val = 0;
                        ProcessTime_t interval_val = 0;

                        // Initial_time & Interval:
                        if ((0 != interval) && (0 != initial_time))
                        {
                            std::vector<long> interval_v;
                            StringParserUtility::ParseCommaSeparatedLongs(interval, interval_v);
                            if (interval_v.size() != 6)
                            {
                                RootMapLogError("OutputRule \"interval\" element, File \"" << location << "\" encountered " << interval << ", must contain 6 valid integer values {y,m,d,H,M,S}");
                                return;
                            }
                            interval_val = PROCESS_YEARS(interval_v[0])
                                + PROCESS_MONTHS(interval_v[1])
                                + PROCESS_DAYS(interval_v[2])
                                + PROCESS_HOURS(interval_v[3])
                                + PROCESS_MINUTES(interval_v[4])
                                + interval_v[5];

                            std::vector<long> initial_time_v;
                            StringParserUtility::ParseCommaSeparatedLongs(initial_time, initial_time_v);
                            if (interval_v.size() != 6)
                            {
                                RootMapLogError("OutputRule \"initialtime\" element, File \"" << location << "\" encountered " << initial_time << ", must contain 6 valid integer values {y,m,d,H,M,S}");
                                return;
                            }
                            initial_time_val = PROCESS_YEARS(initial_time_v[0])
                                + PROCESS_MONTHS(initial_time_v[1])
                                + PROCESS_DAYS(initial_time_v[2])
                                + PROCESS_HOURS(initial_time_v[3])
                                + PROCESS_MINUTES(initial_time_v[4])
                                + initial_time_v[5];

                            // MSA 10.09.23 UPDATE: Initial time should be defined as an offset from the start time!
                            initial_time_val += odp->GetPostOffice()->GetStart();
                        }
                        // MSA If no <when> is specified, and <count> is a positive integer,
                        // treat it as a total (simulation-length) frame count setting.
                        // Specify Frame Count:
                        else if ((count > 0) &&
                            ((0 == interval) && (0 == initial_time))
                            )
                        {
                            interval_val = odp->GetPostOffice()->GetDefaultRunTime() / count;
                            initial_time_val = odp->GetPostOffice()->GetStart() + interval_val;
                        }
                        else
                        {
                            RootMapLogError("OutputRule \"when\" element, File \"" << location << "\" must contain either count+interval+initialtime, count+attime+regularity, or just count");
                            return;
                        }

                        TXOutputRuleDA* data = new TXOutputRuleDA(name,
                            type,
                            characteristic,
                            stratum,
                            filename,
                            m_outputDir,
                            specification1,
                            specification2,
                            reopen_str);

                        data->SetIntervalWhen(count, interval_val, initial_time_val);

                        odp->AddOutputRule(data); // Passes ownership of the TXOutputRuleDA pointer to OutputDataPreviewer.
                    }
                }
                else
                {
                    RootMapLogError("Could not convert outputrule \"when\" Node to Element, File \"" << location << "\" [Line " << when_node->Row() << ",Col " << when_node->Column() << "]");
                    return;
                } // else (boundary_elem == 0)
            }
            catch (const TXUtilityException& tue)
            {
                RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag() << "");
                return;
            }
        } // end else [i.e. end(is calendar-based output rule)]
    } // end if(0!=when_node)


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for data output coordinator
    template <>
    void TXDataAccessManager::GenericElementFunction(DataOutputCoordinator* dcoord, TiXmlElement* element, const char* location)
    {
        // Mandatory elements (exceptions will be thrown if not found)
        std::string name = TXUtility::getChildElementText(element, "source");
        std::string type = TXUtility::getChildElementText(element, "type");
        ScoreboardStratum stratum = ScoreboardStratum::StringToStratum(TXUtility::getChildElementText(element, "stratum"));
        std::string reopen_str = TXUtility::getChildElementText(element, "reopen");
        std::string filename = TXUtility::getChildElementText(element, "filename");

        // Elements that may or may not be required
        std::string characteristic = "";
        const char* characteristic_raw = TXUtility::getChildElementText(element, "characteristic", false);
        if (__nullptr != characteristic_raw)
        {
            characteristic = characteristic_raw;
        }
        // If characteristic tag not found, RaytracerData and ViewCapture are the only valid types
        else
        {
            RmAssert("RaytracerData" == type || "ViewCapture" == type, "Characteristic tag not found for Output Rule");
        }

        std::string specification1 = "";
        const char* specification1_raw = TXUtility::getChildElementText(element, "specification1", false);
        if (__nullptr != specification1_raw)
        {
            specification1 = specification1_raw;
        }

        std::string specification2 = "";
        const char* specification2_raw = TXUtility::getChildElementText(element, "specification2", false);
        if (__nullptr != specification2_raw)
        {
            specification2 = specification2_raw;
        }

        //Finished getting mandatory & possibly-optional elements------------------------------------------

        TiXmlNode* when_node = 0;
        // delve further into this element
        if (0 != (when_node = element->FirstChild("when")))
        {
            TiXmlElement* when_elem = when_node->ToElement();
            try
            {
                if (0 != when_elem)
                {
                    int count = TXUtility::getChildElementInt(when_elem, "count", false, 0);

                    const char* interval = TXUtility::getChildElementText(when_elem, "interval", false);
                    const char* initial_time = TXUtility::getChildElementText(when_elem, "initialtime", false);
                    const char* at_time = TXUtility::getChildElementText(when_elem, "attime", false);
                    const char* regularity = TXUtility::getChildElementText(when_elem, "regularity", false);

                    // either interval+initialtime OR attime+regularity are required.

                    // At_time:
                    if ((0 != at_time) && (0 != regularity))
                    {
                        std::vector<long> at_time_v;
                        StringParserUtility::ParseCommaSeparatedLongs(at_time, at_time_v);
                        if (at_time_v.size() != 6)
                        {
                            RootMapLogError("OutputRule \"attime\" element, File \"" << location << "\" encountered " << at_time << ", must contain 6 valid integer values {y,m,d,H,M,S}");
                            return;
                        }

                        OutputRuleDAI::RegularWhen::ERegularity regularity_val = OutputRuleDAI::RegularWhen::unset;
                        switch (*regularity)
                        {
                        case 'y':
                            regularity_val = OutputRuleDAI::RegularWhen::r_year;
                            break;
                        case 'm':
                            regularity_val = OutputRuleDAI::RegularWhen::r_month;
                            break;
                        case 'd':
                            regularity_val = OutputRuleDAI::RegularWhen::r_day;
                            break;
                        case 'H':
                            regularity_val = OutputRuleDAI::RegularWhen::r_hour;
                            break;
                        case 'M':
                            regularity_val = OutputRuleDAI::RegularWhen::r_minute;
                            break;
                        case 'S':
                            regularity_val = OutputRuleDAI::RegularWhen::r_second;
                            break;
                        default:
                            RootMapLogError("OutputRule \"when:regularity\" element, File \"" << location << "\" encountered " << regularity << ", must contain one of {y,m,d,H,M,S}");
                            break;
                        }

                        TXOutputRuleDA* data = new TXOutputRuleDA(name,
                            type,
                            characteristic,
                            stratum,
                            filename,
                            m_outputDir,
                            specification1,
                            specification2,
                            reopen_str);
                        data->SetRegularWhen(count,
                            at_time_v[0], at_time_v[1], at_time_v[2],
                            at_time_v[3], at_time_v[4], at_time_v[5],
                            regularity_val);

                        dcoord->InitialiseOutput(*data);

                        delete data;
                        data = 0;
                    }
                    // Not At_time (may yet be invalid):
                    else
                    {
                        ProcessTime_t initial_time_val = 0;
                        ProcessTime_t interval_val = 0;

                        // Initial_time & Interval:
                        if ((0 != interval) && (0 != initial_time))
                        {
                            std::vector<long> interval_v;
                            StringParserUtility::ParseCommaSeparatedLongs(interval, interval_v);
                            if (interval_v.size() != 6)
                            {
                                RootMapLogError("OutputRule \"interval\" element, File \"" << location << "\" encountered " << interval << ", must contain 6 valid integer values {y,m,d,H,M,S}");
                                return;
                            }
                            interval_val = PROCESS_YEARS(interval_v[0])
                                + PROCESS_MONTHS(interval_v[1])
                                + PROCESS_DAYS(interval_v[2])
                                + PROCESS_HOURS(interval_v[3])
                                + PROCESS_MINUTES(interval_v[4])
                                + interval_v[5];

                            std::vector<long> initial_time_v;
                            StringParserUtility::ParseCommaSeparatedLongs(initial_time, initial_time_v);
                            if (interval_v.size() != 6)
                            {
                                RootMapLogError("OutputRule \"initialtime\" element, File \"" << location << "\" encountered " << initial_time << ", must contain 6 valid integer values {y,m,d,H,M,S}");
                                return;
                            }
                            initial_time_val = PROCESS_YEARS(initial_time_v[0])
                                + PROCESS_MONTHS(initial_time_v[1])
                                + PROCESS_DAYS(initial_time_v[2])
                                + PROCESS_HOURS(initial_time_v[3])
                                + PROCESS_MINUTES(initial_time_v[4])
                                + initial_time_v[5];

                            // MSA 10.09.23 UPDATE: Initial time should be defined as an offset from the start time!
                            // MSA 2012.07.05 Why? When is it used as such? :/
                            //initial_time_val += dcoord->GetSimulationEngine().GetPostOffice().GetStart();
                        }
                        // MSA If no <when> is specified, and <count> is a positive integer,
                        // treat it as a total (simulation-length) frame count setting.
                        // Specify Frame Count:
                        else if ((count > 0) &&
                            ((0 == interval) && (0 == initial_time))
                            )
                        {
                            interval_val = dcoord->GetSimulationEngine().GetPostOffice().GetDefaultRunTime() / count;
                            initial_time_val = dcoord->GetSimulationEngine().GetPostOffice().GetStart() + interval_val;
                        }
                        else
                        {
                            RootMapLogError("OutputRule \"when\" element, File \"" << location << "\" must contain either count+interval+initialtime, count+attime+regularity, or just count");
                            return;
                        }

                        TXOutputRuleDA* data = new TXOutputRuleDA(name,
                            type,
                            characteristic,
                            stratum,
                            filename,
                            m_outputDir,
                            specification1,
                            specification2,
                            reopen_str);

                        data->SetIntervalWhen(count, interval_val, initial_time_val);

                        dcoord->InitialiseOutput(*data);

                        delete data;
                        data = 0;
                    }
                }
                else
                {
                    RootMapLogError("Could not convert outputrule \"when\" Node to Element, File \"" << location << "\" [Line " << when_node->Row() << ",Col " << when_node->Column() << "]");
                    return;
                } // else (boundary_elem == 0)
            }
            catch (const TXUtilityException& tue)
            {
                RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag() << "");
                return;
            }
        } // end else [i.e. end(is calendar-based output rule)]
    } // end if(0!=when_node)


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for Shared Attribute Manager
    template <>
    void TXDataAccessManager::GenericElementFunction(SharedAttributeManager* sam, TiXmlElement* element, const char* location)
    {
        std::string owner_name = TXUtility::getChildElementText(element, "owner");
        std::string supplier_name = TXUtility::getChildElementText(element, "storage");
        const char* variations = TXUtility::getChildElementText(element, "variations", false);
        std::string variationstr = ((variations != __nullptr) ? variations : "");

        std::vector<std::string> variations_vec;
        StringParserUtility::ParseCommaSeparatedStrings(variations, variations_vec);

        TiXmlNode* characteristic_node = 0;
        // delve further into this element
        if (0 != (characteristic_node = element->FirstChild("characteristic_descriptor")))
        {
            TXCharacteristicDA* characteristic_data = CreateCharacteristicDA(characteristic_node, StratumNONE /*ScoreboardStratum::NONE*/, location);

            if ((0 == characteristic_data) || (!characteristic_data->isValid()))
            {
                LOG_ALERT << LOG_LINE << "SharedAttribute construction data contains an invalid characteristic_descriptor {location:" << location << "}";
            }

            std::string defaults_name("");
            std::vector<double> defaults_array;

            TiXmlNode* defaults_node = 0;
            if (0 != (defaults_node = element->FirstChild("defaults")))
            {
                try
                {
                    defaults_name = TXUtility::getChildElementText(defaults_node, "variation_name");
                    TXUtility::getChildElementDoubleArray(defaults_node, "values", defaults_array);
                }
                catch (TXUtilityException& txue)
                {
                    LOG_ALERT << LOG_LINE << "Caught TXUtilityException {What:" << txue.what() << ", WhatString:" << txue.whatString() << ", WhatTag:" << txue.whatTag() << "}";
                }
            }

            LOG_DEBUG << "Adding shared attribute {Name:" << characteristic_data->getName() << ", Variations:" << variationstr << ", Owner:" << owner_name << "}";
            sam->MakeAttributes(*characteristic_data, owner_name, supplier_name, variations_vec, defaults_name, defaults_array);
            delete characteristic_data;
        } // if characteristic_node ok
        else
        {
            LOG_ALERT << LOG_LINE << "SharedAttribute construction data is invalid {file:" << location << "}";
        }
    }


    TXCharacteristicDA* TXDataAccessManager::CreateCharacteristicDA(TiXmlNode* characteristic_node, const ScoreboardStratum& stratum, const char* location)
    {
        TiXmlElement* characteristic_elem = characteristic_node->ToElement();
        TXCharacteristicDA* characteristic_data = __nullptr;

        try
        {
            if (0 != characteristic_elem)
            {
                double minimum = TXUtility::getChildElementDouble(characteristic_elem, "minimum");
                double maximum = TXUtility::getChildElementDouble(characteristic_elem, "maximum");
                double default_ = TXUtility::getChildElementDouble(characteristic_elem, "default");

                std::string name = TXUtility::getChildElementText(characteristic_elem, "name");
                std::string units = TXUtility::getChildElementText(characteristic_elem, "units");

                bool isVisible = TXUtility::getChildElementBool(characteristic_elem, "visible");
                bool isEdittable = TXUtility::getChildElementBool(characteristic_elem, "editable");
                bool isToBeSaved = TXUtility::getChildElementBool(characteristic_elem, "tobesaved");
                bool isSpecial = TXUtility::getChildElementBool(characteristic_elem, "specialperboxinfo", false, false);

                // if specified stratum exists, it overrides passed in value
                ScoreboardStratum stratum_local = stratum;
                const char* stratum_str = TXUtility::getChildElementText(characteristic_elem, "stratum", false);
                if ((0 != stratum_str) && (0 < strlen(stratum_str)))
                {
                    stratum_local = ScoreboardStratum::StringToStratum(stratum_str);
                }

                // Error if no stratum here or passed in (stratum_local is initialised to stratum, above)
                if (stratum_local == ScoreboardStratum::NONE)
                {
                    RootMapLogError("No stratum defined for characteristic \"" << name << "\", cannot determine ID (file \"" << location << "\" [Line " << characteristic_node->Row() << ",Col " << characteristic_node->Column() << "])");
                    return __nullptr;
                }

                CharacteristicIdentifier cid = IdentifierUtility::instance().useNextCharacteristicIdentifier(stratum_local);

                LOG_DEBUG << "CharacteristidData: cid=" << cid << ", name=" << name << ", units=" << units << ", min=" << minimum << ", max=" << maximum << ", def=" << default_ << ", isVisible=" << Utility::ToString(isVisible);
                characteristic_data = new TXCharacteristicDA
                (cid, name, units, stratum_local, minimum, maximum, default_, isVisible, isEdittable, isToBeSaved, isSpecial);
            }
            else
            {
                RootMapLogError("Could not convert characteristic Node to Element, File \"" << location << "\" [Line " << characteristic_node->Row() << ",Col " << characteristic_node->Column() << "]");
            } // else (boundary_elem == 0)
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
        }

        return characteristic_data;
    }


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for PostOffice
    template <>
    void TXDataAccessManager::GenericElementFunction(PostOffice* poffice, TiXmlElement* element, const char* /* location */)
    {
        try
        {
            bool configFileHasSeed = TXUtility::hasChildElement(element, "random_seed");
            unsigned int seed = configFileHasSeed
                ? TXUtility::getChildElementUnsignedInt(element, "random_seed")
                : std::time(__nullptr);
            if (!m_randomSeed.empty())
            {
                unsigned int newSeed = (unsigned int)strtoul(m_randomSeed.c_str(), __nullptr, 10);
                if (configFileHasSeed)
                {
                    RootMapLogInfo("Overriding seed " << seed << " from configuration file with command line argument seed " << newSeed << ".");
                }
                seed = newSeed;
            }
            ProcessTime_t previous_time = TXUtility::getChildElementDateTime(element, "previous");
            ProcessTime_t now_time = TXUtility::getChildElementDateTime(element, "now");
            ProcessTime_t start_time = TXUtility::getChildElementDateTime(element, "next_start");
            ProcessTime_t end_time = TXUtility::getChildElementDateTime(element, "next_end");
            ProcessTime_t runtime_time = TXUtility::getChildElementTimeSpan(element, "defaultruntime");

            TXPostOfficeDA postoffice_data(seed, previous_time, now_time, start_time, end_time, runtime_time);

            poffice->constructPostOffice(postoffice_data);
        }
        catch (TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException while constructing PostOffice. Reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
        }
    }


    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for scoreboard data initialisation
    template <>
    void TXDataAccessManager::GenericElementFunction(SimulationEngine* engine, TiXmlElement* element, const char* /* location */)
    {
        std::string process_name = TXUtility::getChildElementText(element, "process");
        std::string characteristic = TXUtility::getChildElementText(element, "characteristic");
        std::string scheme_str = TXUtility::getChildElementText(element, "scheme");

        InputSchemeParser& isp = InputSchemeParser::instance();
        isp.ParseScheme(scheme_str.c_str());

        TXScoreboardDataDA scoreboard_data(process_name, characteristic, isp.getElementList());
        engine->InitialiseScoreboardDataValues(scoreboard_data);
    }

    // //////////////////////////////////////////////////////////////////////
    // Specialised Template function for Process Special Data
    template <>
    void TXDataAccessManager::GenericElementFunction(Process* process, TiXmlElement* element, const char* location)
    {
        try
        {
            TXProcessSpecialDA process_special_data;

            TiXmlElement* child_elem = element->FirstChildElement();

            while (__nullptr != child_elem)
            {
                const char* name_attribute = child_elem->Attribute("name");

                // only using elements with name="" attributes
                if (__nullptr != name_attribute)
                {
                    std::string elem_name = child_elem->Value();

                    LOG_INFO << "Encountered element " << elem_name << " in location " << location << " for initialising process data";

                    if (elem_name == "namefloatpairs")
                    {
                        const char* elem_text = child_elem->GetText();
                        std::vector<std::string> string_vec;
                        StringParserUtility::ParseCommaSeparatedStrings(elem_text, string_vec);
                        if ((string_vec.size() % 2) != 0)
                        {
                            // Not an even number of values - log error and continue with next element
                            LOG_ERROR << "Element " << elem_name << " in location " << location << " does not contain an even number of values";
                        }

                        for (std::vector<std::string>::iterator nfp_iter(string_vec.begin());
                            nfp_iter != string_vec.end(); ++nfp_iter)
                        {
                            // first string is the name
                            std::string name_str = *nfp_iter;

                            ++nfp_iter;
                            std::string value_str = *nfp_iter;

                            double value = Utility::StringToDouble(value_str.c_str());

                            process_special_data.addNameFloatPair(name_str, value);
                        }
                    }
                    else if (elem_name == "nameintegerpairs")
                    {
                        const char* elem_text = child_elem->GetText();
                        std::vector<std::string> string_vec;
                        StringParserUtility::ParseCommaSeparatedStrings(elem_text, string_vec);
                        if ((string_vec.size() % 2) != 0)
                        {
                            // Not an even number of values - log error and continue with next element
                            LOG_ERROR << "Element " << elem_name << " in location " << location << " does not contain an even number of values";
                        }
                    }
                    else if (elem_name == "namestringpairs")
                    {
                        const char* elem_text = child_elem->GetText();
                        std::vector<std::string> string_vec;
                        StringParserUtility::ParseCommaSeparatedStrings(elem_text, string_vec);
                        if ((string_vec.size() % 2) != 0)
                        {
                            // Not an even number of values - log error and continue with next element
                            LOG_ERROR << "Element " << elem_name << " in location " << location << " does not contain an even number of values";
                        }
                    }
                    else if (elem_name == "floatarray")
                    {
                        const char* elem_text = child_elem->GetText();
                        std::vector<double> float_vec;
                        StringParserUtility::ParseCommaSeparatedDoubles(elem_text, float_vec);

                        process_special_data.addNamedFloatArray(float_vec, name_attribute);
                    }
                    else if (elem_name == "integerarray")
                    {
                        const char* elem_text = child_elem->GetText();
                        std::vector<long> long_vec;
                        StringParserUtility::ParseCommaSeparatedLongs(elem_text, long_vec);

                        process_special_data.addNamedIntegerArray(long_vec, name_attribute);
                    }
                    else if (elem_name == "stringarray")
                    {
                        const char* elem_text = child_elem->GetText();
                        std::vector<std::string> string_vec;
                        StringParserUtility::ParseCommaSeparatedStrings(elem_text, string_vec);

                        process_special_data.addNamedStringArray(string_vec, name_attribute);
                    }
                    else if ((elem_name == "bytearray") || (elem_name == "blob"))
                    {
                    }
                    else
                    {
                        LOG_ERROR << "Element " << elem_name << " in location " << location << " not supported for initialising process data";
                    }
                }

                child_elem = element->NextSiblingElement();
            }

            process->InitialiseSpecial(process_special_data);
        }
        catch (TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException while initialising Process " << process->GetProcessName() << " special data. Reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
        }
    }

    void TXDataAccessManager::previewScoreboards(ScoreboardDataPreviewer* sdp)
    {
        GenericFunction(sdp, "ScoreboardCoordinator", "construction", "scoreboard", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
        // MSA 11.05.18 How to read/edit Scoreboard data?
        //GenericFunction(sdp, "Scoreboard", "initialisation", "scoreboarddata", &ConfigurationDataStore::hasInitialisationData, &ConfigurationDataStore::getInitialisationData);
    }

    void TXDataAccessManager::previewPlants(PlantDataPreviewer* pdp)
    {
        GenericFunction(pdp, "PlantCoordinator", "construction", "planttype", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
        GenericFunction(pdp, "PlantCoordinator", "construction", "plant", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
        GenericFunction(pdp, "PlantCoordinator", "construction", "plantsummary", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::previewPostOffice(PostOfficeDataPreviewer* podp)
    {
        GenericFunction(podp, "PostOffice", "construction", "postoffice", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::previewProcesses(ProcessDataPreviewer* pdp)
    {
        GenericFunction(pdp, "ProcessCoordinator", "construction", "process", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::previewSharedAttributes(SharedAttributeDataPreviewer* sadp)
    {
        GenericFunction(sadp, "SharedAttributeManager", "construction", "attribute", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::previewVolumeObjects(VolumeObjectDataPreviewer* vodp)
    {
        GenericFunction(vodp, "VolumeObjectCoordinator", "construction", "volumeobject", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::previewOutputs(OutputDataPreviewer* odp)
    {
        GenericFunction(odp, "DataOutputCoordinator", "initialisation", "outputrule", &ConfigurationDataStore::hasInitialisationData, &ConfigurationDataStore::getInitialisationData);
    }

    void TXDataAccessManager::previewWindows(WindowDataPreviewer* wdp)
    {
        GenericFunction(wdp, "TableCoordinator", "visualisation", "table", &ConfigurationDataStore::hasVisualisationData, &ConfigurationDataStore::getVisualisationData);
        GenericFunction(wdp, "ViewCoordinator", "visualisation", "view", &ConfigurationDataStore::hasVisualisationData, &ConfigurationDataStore::getVisualisationData);
        GenericFunction(wdp, "View3DCoordinator", "visualisation", "view3d", &ConfigurationDataStore::hasVisualisationData, &ConfigurationDataStore::getVisualisationData);
    }

    // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


    void TXDataAccessManager::constructScoreboards(ScoreboardCoordinator* scoord)
    {
        GenericFunction(scoord, "ScoreboardCoordinator", "construction", "scoreboard", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    };

    void TXDataAccessManager::constructVolumeObjects(VolumeObjectCoordinator* vocoord)
    {
        GenericFunction(vocoord, "VolumeObjectCoordinator", "construction", "volumeobject", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    };

    void TXDataAccessManager::constructProcesses(ProcessCoordinator* pcoord)
    {
        GenericFunction(pcoord, "ProcessCoordinator", "construction", "process", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    };

    void TXDataAccessManager::constructPlants(PlantCoordinator* pcoord)
    {
        GenericFunction(pcoord, "PlantCoordinator", "construction", "planttype", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
        GenericFunction(pcoord, "PlantCoordinator", "construction", "plant", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
        GenericFunction(pcoord, "PlantCoordinator", "construction", "plantsummary", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::constructOutputs(DataOutputCoordinator* /* docoord */)
    {
        // currently not useful. Intentions for future use include associating
        // data output types with class names that will be the output files
    }

    void TXDataAccessManager::constructPostOffice(PostOffice* sengine)
    {
        GenericFunction(sengine, "PostOffice", "construction", "postoffice", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }

    void TXDataAccessManager::constructSharedAttributes(SharedAttributeManager* sam)
    {
        GenericFunction(sam, "SharedAttributeManager", "construction", "attribute", &ConfigurationDataStore::hasConstructionData, &ConfigurationDataStore::getConstructionData);
    }


    void TXDataAccessManager::initialiseScoreboardData(SimulationEngine* sengine)
    {
        GenericFunction(sengine, "Scoreboard", "initialisation", "scoreboarddata", &ConfigurationDataStore::hasInitialisationData, &ConfigurationDataStore::getInitialisationData);
    }

    void TXDataAccessManager::initialiseOutputs(DataOutputCoordinator* dcoord)
    {
        GenericFunction(dcoord, "DataOutputCoordinator", "initialisation", "outputrule", &ConfigurationDataStore::hasInitialisationData, &ConfigurationDataStore::getInitialisationData);
    }

    void TXDataAccessManager::initialiseProcessData(Process* process)
    {
        const char* process_name = process->GetProcessName().c_str();
        GenericFunction(process, process_name, "initialisation", "data", &ConfigurationDataStore::hasInitialisationData, &ConfigurationDataStore::getInitialisationData);
    }

    void TXDataAccessManager::initialiseRenderer(RenderCoordinator* rcoord)
    {
        GenericFunction(rcoord, "RenderCoordinator", "initialisation", "characteristic_colour_info", &ConfigurationDataStore::hasInitialisationData, &ConfigurationDataStore::getInitialisationData);
    }


    void TXDataAccessManager::visualiseTables(ITableDataUser* tdatauser)
    {
        GenericFunction(tdatauser, "TableCoordinator", "visualisation", "table", &ConfigurationDataStore::hasVisualisationData, &ConfigurationDataStore::getVisualisationData);
    }

    void TXDataAccessManager::visualiseViews(IViewDataUser* vdatauser)
    {
        // Old skool 2D Views
        GenericFunction(vdatauser, "ViewCoordinator", "visualisation", "view", &ConfigurationDataStore::hasVisualisationData, &ConfigurationDataStore::getVisualisationData);
    }

    void TXDataAccessManager::visualiseView3D(IView3DDataUser* v3datauser)
    {
        // Gnu skool 3D OpenGL Views
        GenericFunction(v3datauser, "View3DCoordinator", "visualisation", "view3d", &ConfigurationDataStore::hasVisualisationData, &ConfigurationDataStore::getVisualisationData);
    }


    template <>
    void TXDataAccessManager::GenericElementFunction(ITableDataUser* tdatauser, TiXmlElement* element, const char* location)
    {
        const char* p_name = TXUtility::getChildElementText(element, "process_name");
        const char* c_name = TXUtility::getChildElementText(element, "characteristic_name");
        long layer = atol(TXUtility::getChildElementText(element, "layer_number"));
        ViewDirection viewdir = Utility::StringToViewDirection(TXUtility::getChildElementText(element, "view_direction"));
        ScoreboardStratum stratum = ScoreboardStratum::StringToStratum(TXUtility::getChildElementText(element, "stratum"));

        TXTableDA table_data(p_name, c_name, viewdir, layer, stratum);

        tdatauser->createTable(table_data);

        if (false)
        {
            RootMapLogError("Scoreboard construction data in file \"" << location << "\" was not valid");
        }
    }


    template <>
    void TXDataAccessManager::GenericElementFunction(IViewDataUser* vdatauser, TiXmlElement* element, const char* location)
    {
        ViewDirection viewdir = Utility::StringToViewDirection(TXUtility::getChildElementText(element, "view_direction"));

        std::vector<double> refindex_array;
        bool refindex_ok = TXUtility::getChildElementDoubleArray(element, "reference_index", refindex_array);
        if ((!refindex_ok) || (refindex_array.size() != 3))
        {
            RootMapLogError("View window dai contains incorrect reference_index (" << location << " [Line " << element->Row() << ",Col " << element->Column() << "])");
            return;
        }
        DoubleCoordinate refindex(refindex_array[0], refindex_array[1], refindex_array[2]);

        double scale = TXUtility::getChildElementDouble(element, "scale");
        bool repeat = TXUtility::getChildElementBool(element, "repeat");
        bool wrap = TXUtility::getChildElementBool(element, "wrap");
        bool boundaries = TXUtility::getChildElementBool(element, "boundaries");
        bool boxes = TXUtility::getChildElementBool(element, "boxes");
        bool boxcolours = TXUtility::getChildElementBool(element, "box_colours");
        double multiplier = 1.0; // MSA 10.05.27 Currently not using this because all roots are 1px wide in 2D View.
        int root_colour_mode = 0; // Static colouring
        if (TXUtility::getChildElementBool(element, "root_colour_by_branch_order", false, false))
        {
            if (TXUtility::getChildElementBool(element, "high_contrast_root_colour", false, false))
            {
                root_colour_mode = 2;
            } // High-contrast colouring
            else { root_colour_mode = 1; } // 'Realistic' colouring (shades of base_root colours)
        }
        float rootred = (float)TXUtility::getChildElementDouble(element, "base_root_red", false, 0.1);
        float rootgreen = (float)TXUtility::getChildElementDouble(element, "base_root_green", false, 0.1);
        float rootblue = (float)TXUtility::getChildElementDouble(element, "base_root_blue", false, 0.1);

        ScoreboardFlags sbFlags; // bitset default initialises to zero
        std::vector<std::string> scoreboard_strs;
        TXUtility::getChildElementTextArray(element, "scoreboards", scoreboard_strs);
        for (std::vector<std::string>::iterator sb_iter = scoreboard_strs.begin();
            sb_iter != scoreboard_strs.end(); ++sb_iter)
        {
            ScoreboardStratum stratum = ScoreboardStratum::StringToStratum((*sb_iter).c_str());
            sbFlags.set(stratum.value());
        }

        std::vector<std::string> process_strs;
        TXUtility::getChildElementTextArray(element, "processes", process_strs, false);

        ViewDAI::CharacteristicColourDataArray ccda;

        TiXmlNode* characteristic_node = 0;
        // delve further into this element
        if (0 != (characteristic_node = element->FirstChild("characteristics")))
        {
            ViewDAI::CharacteristicColourData cyan_data;
            ViewDAI::CharacteristicColourData magenta_data;
            ViewDAI::CharacteristicColourData yellow_data;

            GetCharacteristicColourData(characteristic_node, "cyan", cyan_data, location);
            GetCharacteristicColourData(characteristic_node, "magenta", magenta_data, location);
            GetCharacteristicColourData(characteristic_node, "yellow", yellow_data, location);

            ccda.push_back(cyan_data);
            ccda.push_back(magenta_data);
            ccda.push_back(yellow_data);
        }
        else
        {
            RootMapLogError("Could not find View DAI characteristics Node, File \"" << location << "\" [Line " << characteristic_node->Row() << ",Col " << characteristic_node->Column() << "]");
            return;
        } // else (boundary_elem == 0)

        TXViewDA* view_data = new TXViewDA(viewdir, refindex, scale,
            repeat, wrap, boxes,
            boundaries, boxcolours,
            multiplier,
            /* The following 4 arguments -
            Cylindrical Roots, Interstitial Spheres, Conical Tips and Stacks&Slices
            - have no meaning for 2D view  */
            false, false, false, 0,
            root_colour_mode, rootred,
            rootgreen, rootblue,
            sbFlags, process_strs, ccda);
        vdatauser->createView(*view_data);
        delete view_data;
        view_data = 0;
    }


    void TXDataAccessManager::GetCharacteristicColourData(TiXmlNode* characteristic_node, const char* colourTag, ViewDAI::CharacteristicColourData& ccd, const char* location)
    {
        TiXmlNode* colour_node = __nullptr;

        if (__nullptr != (colour_node = characteristic_node->FirstChild(colourTag)))
        {
            GetCharacteristicColourData(colour_node, ccd);
        }
        else
        {
            RootMapLogError("Could not find \"" << colourTag << "\" child node of view characteristics, File \"" << location << "\" [Line " << characteristic_node->Row() << ",Col " << characteristic_node->Column() << "]");
        }
    }

    // MSA 10.02.02 Utility function for populating a CCD with the data from a child-of-characteristic_colour_data XML node (i.e. one level below the CYAN/MAGENTA/YELLOW/ETC element)
    void TXDataAccessManager::GetCharacteristicColourData(TiXmlNode* theChildNode, ViewDAI::CharacteristicColourData& ccd)
    {
        RmAssert(theChildNode, "__nullptr XML child node encountered");
        try
        {
            int colourMin_int = TXUtility::getChildElementInt(theChildNode, "colour_min");
            ccd.colourRangeMin = static_cast<ColourElementValue_t>(Utility::CSMin(colourMin_int, static_cast<int>(std::numeric_limits<ColourElementValue_t>::max())));
            int colourMax_int = TXUtility::getChildElementInt(theChildNode, "colour_max");
            ccd.colourRangeMax = static_cast<ColourElementValue_t>(Utility::CSMax(colourMax_int, static_cast<int>(std::numeric_limits<ColourElementValue_t>::max())));

            ccd.characteristicRangeMin = TXUtility::getChildElementDouble(theChildNode, "characteristic_min");
            ccd.characteristicRangeMax = TXUtility::getChildElementDouble(theChildNode, "characteristic_max");

            ccd.processName = TXUtility::getChildElementText(theChildNode, "process_name");
            ccd.characteristicName = TXUtility::getChildElementText(theChildNode, "characteristic_name");
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
        }
    }

    template <>
    void TXDataAccessManager::GenericElementFunction(IView3DDataUser* v3datauser, TiXmlElement* element, const char* location)
    {
        std::vector<double> viewpos_array;
        const bool viewpos_ok = TXUtility::getChildElementDoubleArray(element, "view_position", viewpos_array);
        if ((!viewpos_ok) || (viewpos_array.size() != 3))
        {
            RootMapLogError("View window dai contains incorrect view_position (" << location << " [Line " << element->Row() << ",Col " << element->Column() << "])");
            return;
        }
        DoubleCoordinate viewpos(viewpos_array[0], viewpos_array[1], viewpos_array[2]);

        const double scale = TXUtility::getChildElementDouble(element, "scale");
        const bool boundaries = TXUtility::getChildElementBool(element, "boundaries");
        const bool boxes = TXUtility::getChildElementBool(element, "boxes");
        const bool inner_boxes = TXUtility::getChildElementBool(element, "box_colours");

        const double multiplier = TXUtility::getChildElementDouble(element, "root_radius_multiplier"); // Multiplier to apply to root radii at the visualisation stage
        /* MSA new quality spec stuff */
        const wxString quality = TXUtility::getChildElementText(element, "quality");
        // Initialise quality variables to CASE MEDIUM values
        bool cylinders = true;
        bool cones = false;
        bool spheres = false;
        //size_t antialias = 0;    MSA not done yet, TODO
        size_t stacksAndSlices = 4;

        // CASE LOW: Just lines drawn
        if (quality.IsSameAs("low", false)) // i.e. case_sensitive==false, see wxString doc
        {
            stacksAndSlices = 2;
            cylinders = false;
        }

        // CASE MEDIUM (default): Cylinders drawn in low resolution
        // with no wireframe overdraw
        else if (quality.IsSameAs("medium", false))
        {
            stacksAndSlices = 8;
        }

        // CASE HIGH: Cylinders and interstitial spheres drawn 
        // with wireframe overdraw to improve contrast and appearance
        else if (quality.IsSameAs("high", false))
        {
            spheres = true;
            stacksAndSlices = 16;
        }

        // CASE VERY HIGH: Cylinders, interstitial spheres and conical root tips
        // drawn with wireframe overdraw and nice antialiasing (TODO)
        else if (quality.Contains("very high"))
        {
            spheres = true;
            cones = true;
            stacksAndSlices = 32;
        }

        int root_colour_mode = 0; // Static colouring
        if (TXUtility::getChildElementBool(element, "root_colour_by_branch_order"))
        {
            if (TXUtility::getChildElementBool(element, "high_contrast_root_colour"))
            {
                root_colour_mode = 2;
            } // High-contrast colouring
            else { root_colour_mode = 1; } // 'Realistic' colouring (shades of base_root colours)
        }
        const float rootred = (float)TXUtility::getChildElementDouble(element, "base_root_red");
        const float rootgreen = (float)TXUtility::getChildElementDouble(element, "base_root_green");
        const float rootblue = (float)TXUtility::getChildElementDouble(element, "base_root_blue");

        ScoreboardFlags sbFlags; // bitset default initialises to zero
        std::vector<std::string> scoreboard_strs;
        TXUtility::getChildElementTextArray(element, "scoreboards", scoreboard_strs);
        for (std::vector<std::string>::iterator sb_iter = scoreboard_strs.begin();
            sb_iter != scoreboard_strs.end(); ++sb_iter)
        {
            ScoreboardStratum stratum = ScoreboardStratum::StringToStratum((*sb_iter).c_str());
            sbFlags.set(static_cast<size_t>(stratum.value()));
        }

        std::vector<std::string> process_strs;
        TXUtility::getChildElementTextArray(element, "processes", process_strs, false);

        // MSA 10.09.06 Pre-sorting the process names so that Water and VolumeObjectCoordinator are drawn last (in that order).
        // This is a way to ensure that all opaque objects are drawn before the translucent objects.
        std::vector<std::string>::iterator finder = find(process_strs.begin(), process_strs.end(), "Water");
        if (finder != process_strs.end())
        {
            process_strs.erase(finder);
            process_strs.push_back("Water");
        }
        finder = find(process_strs.begin(), process_strs.end(), "VolumeObjectCoordinator");
        if (finder != process_strs.end())
        {
            process_strs.erase(finder);
            process_strs.push_back("VolumeObjectCoordinator");
        }

        // Dummy characteristic data
        ViewDAI::CharacteristicColourDataArray ccda;
        ViewDAI::CharacteristicColourData cyan_data;
        ViewDAI::CharacteristicColourData magenta_data;
        ViewDAI::CharacteristicColourData yellow_data;
        ccda.push_back(cyan_data);
        ccda.push_back(magenta_data);
        ccda.push_back(yellow_data);

        TXViewDA* view_data = new TXViewDA(vFront, viewpos, scale,
            false, false, boxes,
            boundaries, inner_boxes,
            multiplier,
            cylinders, spheres,
            cones, stacksAndSlices,
            root_colour_mode, rootred,
            rootgreen, rootblue, sbFlags,
            process_strs, ccda);
        v3datauser->createView3D(*view_data);
        // MSA GuiSimulationEngine doesn't take ownership of the view_data pointer.
        // delete?
        delete view_data;
        view_data = 0;
    }

    template <>
    void TXDataAccessManager::GenericElementFunction(WindowDataPreviewer* wdp, TiXmlElement* element, const char* /*location*/)
    {
        try
        {
            static const wxString table = "table";
            static const wxString view = "view";
            static const wxString view3d = "view3d";
            const wxString name = element->Value();
            if (table.CmpNoCase(name) == 0)
            {
                WindowDataPreviewer::Table* t = new WindowDataPreviewer::Table;
                // Set struct member values
                t->m_processName = TXUtility::getChildElementText(element, "process_name");
                t->m_characteristicName = TXUtility::getChildElementText(element, "characteristic_name");
                t->m_viewDirection = TXUtility::getChildElementText(element, "view_direction");
                t->m_stratum = TXUtility::getChildElementText(element, "stratum");
                t->m_layerNumber = TXUtility::getChildElementInt(element, "layer_number");

                wdp->AddTable(t);
            }
            else if (view.CmpNoCase(name) == 0)
            {
                WindowDataPreviewer::TwoDView* t = new WindowDataPreviewer::TwoDView;
                // Set struct member values
                t->m_viewDirection = TXUtility::getChildElementText(element, "view_direction");
                std::vector<double> ref;
                TXUtility::getChildElementDoubleArray(element, "reference_index", ref);
                t->m_referenceIndexX = ref[0];
                t->m_referenceIndexY = ref[1];
                t->m_referenceIndexZ = ref[2];
                t->m_scale = TXUtility::getChildElementDouble(element, "scale");
                t->m_zoomRatio = TXUtility::getChildElementDouble(element, "zoom_ratio");
                t->m_repeat = TXUtility::getChildElementBool(element, "repeat");
                t->m_wrap = TXUtility::getChildElementBool(element, "wrap");
                t->m_drawBoundaries = TXUtility::getChildElementBool(element, "boundaries");
                t->m_drawBoxes = TXUtility::getChildElementBool(element, "boxes");
                t->m_drawBoxColours = TXUtility::getChildElementBool(element, "box_colours");
                t->m_scoreboards = TXUtility::getChildElementText(element, "scoreboards");
                t->m_processes = TXUtility::getChildElementText(element, "processes");

                TiXmlElement* cyanElem = element->FirstChildElement("characteristics")->FirstChildElement("cyan");
                t->m_cyanProcessName = TXUtility::getChildElementText(cyanElem, "process_name");
                t->m_cyanCharacteristicName = TXUtility::getChildElementText(cyanElem, "characteristic_name");
                t->m_cyanColourMin = TXUtility::getChildElementInt(cyanElem, "colour_min");
                t->m_cyanColourMax = TXUtility::getChildElementInt(cyanElem, "colour_max");
                t->m_cyanCharacteristicMin = TXUtility::getChildElementDouble(cyanElem, "characteristic_min");
                t->m_cyanCharacteristicMax = TXUtility::getChildElementDouble(cyanElem, "characteristic_max");

                TiXmlElement* magentaElem = element->FirstChildElement("characteristics")->FirstChildElement("magenta");
                t->m_magentaProcessName = TXUtility::getChildElementText(magentaElem, "process_name");
                t->m_magentaCharacteristicName = TXUtility::getChildElementText(magentaElem, "characteristic_name");
                t->m_magentaColourMin = TXUtility::getChildElementInt(magentaElem, "colour_min");
                t->m_magentaColourMax = TXUtility::getChildElementInt(magentaElem, "colour_max");
                t->m_magentaCharacteristicMin = TXUtility::getChildElementDouble(magentaElem, "characteristic_min");
                t->m_magentaCharacteristicMax = TXUtility::getChildElementDouble(magentaElem, "characteristic_max");

                TiXmlElement* yellowElem = element->FirstChildElement("characteristics")->FirstChildElement("yellow");
                t->m_yellowProcessName = TXUtility::getChildElementText(yellowElem, "process_name");
                t->m_yellowCharacteristicName = TXUtility::getChildElementText(yellowElem, "characteristic_name");
                t->m_yellowColourMin = TXUtility::getChildElementInt(yellowElem, "colour_min");
                t->m_yellowColourMax = TXUtility::getChildElementInt(yellowElem, "colour_max");
                t->m_yellowCharacteristicMin = TXUtility::getChildElementDouble(yellowElem, "characteristic_min");
                t->m_yellowCharacteristicMax = TXUtility::getChildElementDouble(yellowElem, "characteristic_max");

                wdp->Add2DView(t);
            }
            else if (view3d.CmpNoCase(name) == 0)
            {
                WindowDataPreviewer::ThreeDView* t = new WindowDataPreviewer::ThreeDView;
                // Set struct member values
                std::vector<double> pos;
                TXUtility::getChildElementDoubleArray(element, "view_position", pos);
                t->m_viewPositionX = pos[0];
                t->m_viewPositionY = pos[1];
                t->m_viewPositionZ = pos[2];
                t->m_scale = TXUtility::getChildElementDouble(element, "scale");
                t->m_drawBoundaries = TXUtility::getChildElementBool(element, "boundaries");
                t->m_drawBoxes = TXUtility::getChildElementBool(element, "boxes");
                t->m_drawBoxColours = TXUtility::getChildElementBool(element, "box_colours");
                t->m_rootRadiusMultiplier = TXUtility::getChildElementDouble(element, "root_radius_multiplier");
                t->m_rootColourByBranchOrder = TXUtility::getChildElementBool(element, "root_colour_by_branch_order");
                t->m_highContrastRootColour = TXUtility::getChildElementBool(element, "high_contrast_root_colour");
                t->m_baseRootRed = TXUtility::getChildElementDouble(element, "base_root_red");
                t->m_baseRootGreen = TXUtility::getChildElementDouble(element, "base_root_green");
                t->m_baseRootBlue = TXUtility::getChildElementDouble(element, "base_root_blue");
                t->m_quality = TXUtility::getChildElementText(element, "quality");
                t->m_processes = TXUtility::getChildElementText(element, "processes");
                t->m_scoreboards = TXUtility::getChildElementText(element, "scoreboards");

                wdp->Add3DView(t);
            }
        }
        catch (const TXUtilityException& tue)
        {
            RootMapLogError("Caught TXUtilityException reason: " << tue.what() << ", string: " << tue.whatString() << ", tag: " << tue.whatTag());
            return;
        }
    }
} /* namespace rootmap */

