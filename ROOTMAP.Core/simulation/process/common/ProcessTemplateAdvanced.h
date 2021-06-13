#ifndef ProcessTemplateAdvanced_H
#define ProcessTemplateAdvanced_H
/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessTemplateAdvanced.h
// Purpose:     Declaration of the ProcessTemplateAdvanced class
// Created:     22-03-2009
// Author:      Your Name
// $Date: 2008-06-16 02:07:36 +0800 (Mon, 16 Jun 2008) $
// $Revision: 1 $
// Copyright:   ©2006 University of Tasmania
//
// File-wide comments can go here, but class comments should go just above the
// start of the class itself (for javadoc/doxygen effectiveness)
/////////////////////////////////////////////////////////////////////////////
//other files whose definitions we use in this file
#include "simulation/process/common/Process.h"
#include "core/common/DoubleCoordinates.h"
// Rootmap log4cpp macros
#include "core/debug/log/Logger.h"

namespace rootmap
{

// Forward declarations.
class ProcessSharedAttributeOwner;
class SharedAttribute;


/**
 * Class description goes here
 */
class ProcessTemplateAdvanced : public Process
{
public:
    // wxWidgets macro for dynamic by-name instantiation
    DECLARE_DYNAMIC_CLASS(ProcessTemplateAdvanced)

    // this is rootmap's macro to force this class to be linked into the executable
    // (otherwise the linker strips it out because there is no explicit use of the class)
    DECLARE_DYNAMIC_CLASS_FORCE_USE(ProcessTemplateAdvanced)

public:
    ProcessTemplateAdvanced();
    ~ProcessTemplateAdvanced();

    /**
     *
     */
    virtual long int StartUp(ProcessActionDescriptor *action);

    /**
     *
     */
    virtual long int Ending(ProcessActionDescriptor *action);
    
    /**
     *
     */
    virtual long int Initialise(ProcessActionDescriptor *action);

    /**
     *
     */
    virtual long int Terminate(ProcessActionDescriptor *action);

    /**
     *
     */
    virtual long int Register(ProcessActionDescriptor *action);

    /**
     * The PlantCoordinator will send your process this message when it has
     * successfully processed the PlantSummary registration. The ActionDescriptor
     * will contain the PSR filled in appropriately.
     */
    virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor *action);

    /**
     *
     */
    virtual long int DoNormalWakeUp(ProcessActionDescriptor *action);


    // Use of GetPeriodicDelay is deprecated: call SetRegularWaking(MessageType, 
    //virtual long int GetPeriodicDelay(long int);

 
    /**
     * Called by the PostOffice when deciding to send useful messages.
     * The base class returns false, we override to return true.
     */
    virtual bool DoesOverride() const;

    // Drawing
    virtual bool DoesDrawing() const;
    virtual void DrawScoreboard(const DoubleRect & area, Scoreboard * scoreboard);



private:
    RootMapLoggerDeclaration();

    ProcessIdentifier myWaterModuleID;
    ProcessIdentifier myPlantCoordinatorModuleID;

    CharacteristicIndex my_first_plant_summary_index;
    long my_number_of_plants;
    long my_number_of_branch_orders;


    //
    // SharedAttribute support
    //
    // functions that we require to override as part of being a
    // SharedAttributeOwner
    ProcessSharedAttributeOwner * mySharedAttributeOwner;

    // 
    void PrepareSharedAttributes(ProcessActionDescriptor * /*action*/);

    CharacteristicIndex saWaterFluxPerPlantPerRootOrderIndex;
    CharacteristicIndex saProcessTemplateAdvancedUptakePerPlantIndex;
    CharacteristicIndex saRootLengthPerPlantIndex;
    CharacteristicIndex saRootLengthPerPlantPerRootOrderIndex;
    CharacteristicIndex saRootRadiusPerPlantPerRootOrderIndex;

    typedef std::vector<DoubleCoordinate> MyArray;

    /**
     *
     */
    MyArray myArray;

};

} /* namespace rootmap */

#endif // #ifndef ProcessTemplateAdvanced_H
