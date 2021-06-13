/////////////////////////////////////////////////////////////////////////////
// Name:        MacResourceManager.cpp
// Purpose:     
// Created:     DD/MM/YYYY
// Author:      RvH
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "core/macos_compatibility/MacResourceManager.h"
#include "core/macos_compatibility/CropSimStringResources.h"
#include "core/common/Constants.h"

#include <map>
#include <vector>

namespace rootmap
{
    namespace MacResourceManager
    {
        typedef char* CharStar;
        //
        typedef CharStar CharStarArray[99];
        // ye olde type definition for a vector of strings
        typedef std::vector<std::string> CropSimStringListResource;

        // ye olde type definition for a map of that vector of strings, key'd on a long index
        typedef std::map<long, CropSimStringListResource> CropSimStringListResourceMap;

        // static (local) variable for the 'STR#' resource replacements
        static CropSimStringListResourceMap s_cropsimStringListResources;

        // ye olde type definition for a map of strings, key'd on a long index
        typedef std::map<long, std::string> CropSimStringResourceMap;

        // static (local) variable for the 'STR ' resource replacements
        static CropSimStringResourceMap s_cropsimStringResources;

        static void initstringvector(long mapindex, CharStarArray stringarray); //, size_t sizeofarray=4);

        void GetIndString(std::string& s, long stringListNumber, long stringIndex)
        {
            CropSimStringListResourceMap::iterator fnd = s_cropsimStringListResources.find(stringListNumber);

            if (fnd != s_cropsimStringListResources.end())
            {
                const CropSimStringListResource& csa = (*fnd).second;
                s = csa[stringIndex];
            }
            else
            {
                char buff[64];
                sprintf_s(buff, 64, "GetString couldn't find string list number %d", stringListNumber);
                throw MacResourceManagerException(buff);
            }
        }

        std::string GetString(long stringIndex)
        {
            CropSimStringResourceMap::iterator fnd = s_cropsimStringResources.find(stringIndex);

            if (fnd != s_cropsimStringResources.end())
            {
                return (std::string((*fnd).second));
            }
            else
            {
                char buff[64];
                sprintf_s(buff, 64, "GetString couldn't find string for index %d", stringIndex);
                throw MacResourceManagerException(buff);
            }
        }

        void Initialise()
        {
            initstringvector(32034, CSSR_InputLogFileFolderNames);
            initstringvector(kCollectedErrorsStringList, CSSR_CollectedErrors);
            initstringvector(1, CSSR_Product);
            initstringvector(128, CSSR_Common);
            initstringvector(129, CSSR_MemoryWarnings);
            initstringvector(130, CSSR_TaskNames);
            initstringvector(131, CSSR_Exceptions);
            initstringvector(133, CSSR_DialogValidation);
            initstringvector(134, CSSR_StreamErrorMessages);
            initstringvector(302, CSSR_LibValidation);
            initstringvector(STRL_GVTDisplay, CSSR_GVTsOutgoing);
            initstringvector(STRL_GVTInternal, CSSR_GVTsIncoming);
            initstringvector(STRLErr, CSSR_MiscellaneousErrors);
            initstringvector(STRLViewDirections, CSSR_ViewDirections);
            initstringvector(STRLWindowTitles, CSSR_WindowTitles);
            initstringvector(STRLTableAxisLabels, CSSR_TableAxisLabels);
            initstringvector(STRLUnitStrings, CSSR_CharacteristicUnits);
            //initstringvector(STRLCharacteristicNames, CSSR_CharacteristicNames);
            //initstringvector(STRLProcessNames, CSSR_ProcessNames);
            initstringvector(STRL_SummaryRoutines, CSSR_SummaryRoutineNames);
            initstringvector(STRL_BufferErrors, CSSR_BufferErrorDescriptions);
            initstringvector(STRLJInputAssociationInformationLabels, CSSR_InputAssociationInformationLabels);
            initstringvector(STRLJOutputAssociationInformationLabels, CSSR_OutputAssociationInformationLabels);
            initstringvector(STRLJMetaAssociationInformationLabels, CSSR_MetaAssociationInformationLabels);
            initstringvector(STRLJAssociationLabelInformationLabel, CSSR_AssociationLabelInformationLabel);
            initstringvector(STRLMiscellaneous, CSSR_Miscellaneous);
            initstringvector(STRL_PlantBrowserButtonNames, CSSR_PlantBrowserButtonNamesAndCommands);
            initstringvector(STRL_PlantTypeBrowserButtonNames, CSSR_PlantTypeBrowserButtonNamesAndCommands);
            initstringvector(STRL_PlantSummaryBrowserButtonNames, CSSR_PlantSummaryBrowserButtonNamesandCommands);

            s_cropsimStringResources[150] = CSSR_SaveAsPrompt;
            s_cropsimStringResources[300] = CSSR_OSError;
            s_cropsimStringResources[301] = CSSR_OSError2;
            s_cropsimStringResources[617] = CSSR_UntitledFileTitle;
            s_cropsimStringResources[621] = CSSR_DefaultPlantName;
            s_cropsimStringResources[622] = CSSR_DefaultPlantTypeName;
            s_cropsimStringResources[613] = CSSR_DefaultPrefsFileName;
        }

        void initstringvector(long mapindex, CharStarArray stringarray) //, size_t sizeofarray)
        {
            CropSimStringListResource stringvector;

            long i = 0;
            for (; i < 100;) // KLUDGEd upper limit - no string array is larger than this
            {
                std::string s = stringarray[i];
                if (s != ENDOFARRAYINDICATOR)
                {
                    stringvector.push_back(s);
                }
                else
                {
                    break;
                }
                ++i;
            }

            // actually add to the map now
            CropSimStringListResourceMap::value_type value(mapindex, stringvector);
            s_cropsimStringListResources.insert(value);
        }
    } // namespace MacResourceManager
} /* namespace rootmap */
