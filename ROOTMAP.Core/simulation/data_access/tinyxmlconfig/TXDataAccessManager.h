#ifndef TXDataAccessManager_H
#define TXDataAccessManager_H
/////////////////////////////////////////////////////////////////////////////
// Name:        TXDataAccessManager.h
// Purpose:     Declaration of the TXDataAccessManager class
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-20 10:30:16 +0800 (Fri, 20 Jun 2008) $
// $Revision: 7 $
// Copyright:   ©2006 University of Tasmania
//
// The TXDataAccessManager is an abstract class that represents what is used by
// the SimulationEngine to form its simulation. The visitor pattern is used.
/////////////////////////////////////////////////////////////////////////////
#include "simulation/data_access/common/DataAccessManager.h"
#include "simulation/data_access/common/ConfigurationDataStore.h"
#include "simulation/data_access/interface/ViewDAI.h"

#include "core/log/Logger.h"

#include "tinyxml.h"

#include <string>

namespace rootmap
{
    class TXConfigurationDataStore;
    class ScoreboardCoordinator;
    class DataOutputCoordinator;
    class ProcessCoordinator;
    class RenderCoordinator;
    class VolumeObjectCoordinator;
    class TXCharacteristicDA;
    class ScoreboardStratum;
    class PossibleAxis;
    class PostOffice;

    class ITableDataUser;
    class IViewDataUser;
    class IView3DDataUser;

    class TXDataAccessManager : public DataAccessManager
    {
    public:

        void previewScoreboards(ScoreboardDataPreviewer* sdp);
        void previewVolumeObjects(VolumeObjectDataPreviewer* vodp);
        void previewProcesses(ProcessDataPreviewer* pdp);
        void previewPlants(PlantDataPreviewer* pdp);
        void previewOutputs(OutputDataPreviewer* odp);
        void previewPostOffice(PostOfficeDataPreviewer* podp);
        void previewSharedAttributes(SharedAttributeDataPreviewer* sadp);
        void previewWindows(WindowDataPreviewer* wdp);

        void constructScoreboards(ScoreboardCoordinator* scoord);
        void constructVolumeObjects(VolumeObjectCoordinator* vocoord);
        void constructProcesses(ProcessCoordinator* pcoord);
        void constructPlants(PlantCoordinator* pcoord);
        void constructOutputs(DataOutputCoordinator* docoord);
        void constructPostOffice(PostOffice* sengine);
        void constructSharedAttributes(SharedAttributeManager* sam);

        void initialiseScoreboardData(SimulationEngine* sengine);
        void initialiseOutputs(DataOutputCoordinator* dcoord);
        void initialiseProcessData(Process* process);
        void initialiseRenderer(RenderCoordinator* rcoord);

        void visualiseTables(ITableDataUser* tdatauser);
        void visualiseViews(IViewDataUser* vdatauser);
        void visualiseView3D(IView3DDataUser* v3datauser);


        TXDataAccessManager(std::string absoluteConfigDir, std::string absoluteOutputDir, std::string configFile = "rootmap.xml", std::string randomSeed = "");
        virtual ~TXDataAccessManager();

        /**
         * Stuff about the what the function does relative to the scheme of things.
         * A good temporary place for design-level documentation.
         *
         * @param
         * @return
         */
    protected:

    private:
        typedef bool (ConfigurationDataStore::*HasOwnerDataFunction)(const std::string& owner);
        typedef NameLocationDataList(ConfigurationDataStore::*GetOwnerDataFunction)(const std::string& owner);

        //template < class T > void ConstructionFunction(T * target, const char * ownerName, const char * itemName);
        //template < class T > void InitialisationFunction(T * target, const char * ownerName, const char * itemName);
        template <class T>
        void GenericFunction(T* target, const char* ownerName, const char* childType, const char* itemName, HasOwnerDataFunction hasDataFunction, GetOwnerDataFunction getDataFunction);

        //template < class T > void ConstructionSubFunction(T * target, const char * ownerName, const char * itemName, TiXmlElement * rootElement, const char * location);
        //template < class T > void InitialisationSubFunction(T * target, const char * ownerName, const char * itemName, TiXmlElement * rootElement, const char * location);
        template <class T>
        void GenericSubFunction(T* target, const char* ownerName, const char* childType, const char* itemName, TiXmlElement* rootElement, const char* location);

        //template < class T > void ConstructElementFunction(T * target, TiXmlElement * element, const char * location);
        //template < class T > void InitialiseElementFunction(T * target, TiXmlElement * element, const char * location);
        template <class T>
        void GenericElementFunction(T* target, TiXmlElement* element, const char* location);

        /**
         * @param
         * @return
         */
        TXCharacteristicDA* CreateCharacteristicDA(TiXmlNode* characteristic_node, const ScoreboardStratum& stratum, const char* location);

        void GetCharacteristicColourData(TiXmlNode* characteristic_node, const char* colourTag, ViewDAI::CharacteristicColourData& ccd, const char* location);
        void GetCharacteristicColourData(TiXmlNode* theChildNode, ViewDAI::CharacteristicColourData& ccd);

        void PreviewPlantType(PlantDataPreviewer* pdp, TiXmlElement* element, const char* location);
        void PreviewPlant(PlantDataPreviewer* pdp, TiXmlElement* element, const char* location);
        void PreviewPlantSummary(PlantDataPreviewer* pdp, TiXmlElement* element, const char* location);
        //PossibleAxis * PreviewPossibleAxes(TiXmlElement * element, const char * location, const char * axistype);

        void CreatePlantType(PlantCoordinator* pcoord, TiXmlElement* element, const char* location);
        void CreatePlant(PlantCoordinator* pcoord, TiXmlElement* element, const char* location);
        void CreatePlantSummary(PlantCoordinator* pcoord, TiXmlElement* element, const char* location);
        PossibleAxis* CreatePossibleAxes(TiXmlElement* element, const char* location, const char* axistype);

        void PreviewVolumeObject(VolumeObjectDataPreviewer* vodp, TiXmlElement* element, const char* location);
        void CreateVolumeObject(VolumeObjectCoordinator* vocoord, TiXmlElement* element, const char* location);

        RootMapLoggerDeclaration();

        ///
        /// member declaration
        TXConfigurationDataStore* m_txDataStore;
        std::string m_outputDir;
        std::string m_randomSeed;
    }; // class TXDataAccessManager
} /* namespace rootmap */

#endif // #ifndef TXDataAccessManager_H
