#ifndef Process_H
#define Process_H
/////////////////////////////////////////////////////////////////////////////
// Name:        Process.h
// Purpose:     Declaration of the Process class
// Created:     22-04-1994
// Author:      RvH
// $Date: 2009-07-07 02:40:29 +0800 (Tue, 07 Jul 2009) $
// $Revision: 67 $
// Copyright:   ©2002-2008 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/Types.h"
#include "simulation/process/common/ProcessModuleBase.h"
#include "simulation/process/common/ProcessDrawingException.h"
#include "simulation/process/common/Process_Dictionary.h"

#include "core/common/Structures.h"
#include "core/common/DoubleCoordinates.h"

#include "core/log/Logger.h"

#include <list>

namespace rootmap
{
    class InterprocessDataDescriptor;
    class InterprocessDescriptorList;
    class PostOffice;

    class Scoreboard;

    class PlantSummaryRegistration;
    class ProcessConfigurationData;
    class ProcessDAI;
    class ProcessSpecialDAI;
    class RaytracerData;
    class ProcessDrawing;

    class VolumeObjectCoordinator;

    //#define TimeNow(x) (GetDateTime(&x))

    class Process : public ProcessModuleBase
    {
        DECLARE_DYNAMIC_CLASS(Process)

    public:
        /// required for wxCreateDynamicObject instatiation
        Process();
        /// Initialisation
        virtual void Initialise(const ProcessDAI& data);
        virtual void InitialiseSpecial(const ProcessSpecialDAI& data);
        /// Destruction
        virtual ~Process();


        // /////////////////////////////////////////////////
        // Accessors
        // /////////////////////////////////////////////////
#if defined ROOTMAP_TODO // remove the "Process" from these basic functions
#endif
        ProcessIdentifier GetProcessID() const;
        const ScoreboardStratum& GetProcessStratum() const;
        ProcessActivity GetProcessActivity() const;
        virtual void GetProcessName(std::string& s) const;
        virtual const std::string& GetProcessName() const;
        virtual bool DoesOverride() const;
        ProcessTime_t GetPreviousTimeStamp() const;

    protected:
        void SetName(const std::string& name);

    public:

        // /////////////////////////////////////////////////
        //    Data Access
        // /////////////////////////////////////////////////
        virtual bool DoesSaveProcessData() const;


        // /////////////////////////////////////////////////
        //    Inter-Process Communication
        // /////////////////////////////////////////////////

    public:
        /* Return values for the following two functions :
        < 0    some error
        0        no error
        > 0    some message

        Functional Pairs:
        Initialise() and Terminate()
        StartUp() and Ending()*/
        virtual long int Register(ProcessActionDescriptor* action);
        virtual long int Initialise(ProcessActionDescriptor* action);
        virtual long int Terminate(ProcessActionDescriptor* action);
        virtual long int StartUp(ProcessActionDescriptor* action);
        virtual long int Ending(ProcessActionDescriptor* action);

        /**
         * This is the central function that is called every time a
         */
        virtual long int WakeUp(ProcessActionDescriptor* action);

        // Responses to "Standard Interprocess Communications Suite" messages
        virtual long int DoNormalWakeUp(ProcessActionDescriptor* action); // 'Nrml'
        virtual long int DoExternalWakeUp(ProcessActionDescriptor* action); // 'XWak'

        virtual long int DoRegisterRequest(ProcessActionDescriptor* action); // 'RgRq'
        virtual long int DoNormalCoughUp(ProcessActionDescriptor* action); // 'NmCu' 
        virtual long int DoImmediateCoughUp(ProcessActionDescriptor* action); // 'CuIm'
        virtual long int DoCoughUp(ProcessActionDescriptor* action);
        virtual long int SendCoughUps(ProcessActionDescriptor* action);

        virtual long int DoImmediateReceival(ProcessActionDescriptor* action); // 'SCui', 'OCui'
        virtual long int DoNormalReceival(ProcessActionDescriptor* action); // 'RgCu'
        virtual long int DoUnsolicitedReceival(ProcessActionDescriptor* action); // 'UnCu'
        virtual long int DoDelayedReceivalReaction(ProcessActionDescriptor* action); // 'ReDR'
        virtual long int DoReceive(ProcessActionDescriptor* action);
        virtual long int DoGlobalPlantSummaryRegistered(ProcessActionDescriptor* action); // 'GpSr'
        virtual long int DoScoreboardPlantSummaryRegistered(ProcessActionDescriptor* action); // 'SpSr'

        virtual long int DoSpecialInput(ProcessActionDescriptor* action); // 'Inpu'
        virtual long int DoSpecialOutput(ProcessActionDescriptor* action); // 'Outp'
        virtual long int DoRaytracerOutput(RaytracerData* raytracerData);
        virtual long int DoActivateRaytracerOutput();

        virtual long int DoGeneralPurpose(ProcessActionDescriptor* action); // 'GPm*'
        virtual long int DoGeneralPurpose0(ProcessActionDescriptor* action); // 'GPm0'
        virtual long int DoGeneralPurpose1(ProcessActionDescriptor* action); // 'GPm1'
        virtual long int DoGeneralPurpose2(ProcessActionDescriptor* action); // 'GPm2'
        virtual long int DoGeneralPurpose3(ProcessActionDescriptor* action); // 'GPm3'
        virtual long int DoGeneralPurpose4(ProcessActionDescriptor* action); // 'GPm4'
        virtual long int DoGeneralPurpose5(ProcessActionDescriptor* action); // 'GPm5'
        virtual long int DoGeneralPurpose6(ProcessActionDescriptor* action); // 'GPm6'
        virtual long int DoGeneralPurpose7(ProcessActionDescriptor* action); // 'GPm7'
        virtual long int DoGeneralPurpose8(ProcessActionDescriptor* action); // 'GPm8'
        virtual long int DoGeneralPurpose9(ProcessActionDescriptor* action); // 'GPm9'

    protected:
        // Use of the following SetPeriodicWaking/GetPeriodicDelay is deprecated.
        // New classes will use the MessageType based functionality below.
        // 
        // These 2 functions will be removed in a future "API Release".
        //virtual long int GetPeriodicDelay(long int characteristic_number=1);
        //virtual void SetPeriodicWaking(ProcessActionDescriptor *action);

        /**
         * Set a value for a message type. When the base class Do[MessageType]
         * function is called, it will resend a message of the same type to self.
         *
         * NOTE: After using this method, you need to send the initial message
         *      to cause the periodic wake up in WakeUp().
         */
        void SetPeriodicDelay(MessageType_t mType, ProcessTime_t t);

        /**
         * Retrieve the periodic delay for a particular MessageType.
         */
        ProcessTime_t GetPeriodicDelay(MessageType_t mType) const;

        /**
         *
         */
        void SetTimeDeltaTracking(MessageType_t mType = kNormalMessage);

        /**
         * Retrieves the dT of the message type. This only works if the derived
         * class does not override WakeUp(), as the dT is calculated in that
         * function before calling Do[MessageType]() specifics.
         */
        ProcessTime_t GetTimeDelta(MessageType_t mType = kNormalMessage) const;

        /**
         * Alias for GetTimeDelta(kNormalMessage);
         */
        ProcessTime_t dT() const;

        // most commonly used version - for when the server's characteristic_name is known
        InterprocessDataDescriptor* MakeRequestDescriptor(const std::string& requested_characteristic_name, long int client_requested_number, long int client_request_rate_number, long int client_receive_buffer_number, ProcessActionDescriptor* action);
        // not usually used. Used by the above version
        InterprocessDataDescriptor* MakeRequestDescriptor(Process* server_process, long int server_requested_number, long int client_requested_number, long int client_request_rate_number, long int client_receive_buffer_number, ProcessActionDescriptor* action);
        // intended for Unsolicited cough ups
        InterprocessDataDescriptor* MakeCoughUpDescriptor(long server_source_number, const std::string& client_destination_name, const std::string& client_buffer_name, ProcessActionDescriptor* action);
        // used by DoCoughUp()
        InterprocessDataDescriptor* MakeCoughUpDescriptor(InterprocessDataDescriptor* idd, long time);

        virtual void SendRequest(InterprocessDataDescriptor* request, bool is_registered_not_immediate, ProcessActionDescriptor* action);
        virtual void SendUnsolicitedCoughUp(InterprocessDataDescriptor* coughup, ProcessActionDescriptor* action);
        virtual void AppendRequest(InterprocessDataDescriptor* new_request);
        virtual void AppendReceival(InterprocessDataDescriptor* new_receival);

        virtual double ProportionAllocated(double amount_requested, double amount_available);

        void SetCharacteristicIndices(ProcessCoordinator* processcoordinator, VolumeObjectCoordinator* volumeobjectcoordinator, CharacteristicIndices& characteristicIndices, const std::string& name);

    private:
        RootMapLoggerDeclaration();

        std::string m_name; // 
        ScoreboardStratum m_scoreboardStratum;
        ProcessIdentifier m_id; // Identification Number - what it is
        ProcessActivity m_activity; // what it does
        ProcessTime_t m_previousTimeStamp;
        ProcessTime_t m_lastCoughUp;


        /**
         * Type definition for a map of time, key'd by MessageType
         */
        typedef std::map<MessageType_t, ProcessTime_t> MessageTimeMap;

        /**
         * A map of periodic waking times for MessageTypes
         */
        MessageTimeMap m_periodicWaking;
        /**
         * A map of time deltas for MessageTypes
         */
        MessageTimeMap m_timeDeltaTracking;

        InterprocessDescriptorList*
            m_registeredRequests;
        InterprocessDescriptorList*
            m_registeredReceivals;

        ///
        /// the object that does the drawing. Null if this process does not draw
        ProcessDrawing* m_drawing;


        bool m_delayedReactionActivated;


        // Drawing routines
    public:
        /**
         * override to return true if your process wishes to perform drawing
         *
         * IMPORTANT NOTE:
         * Because this function is virtual, you must implement your override
         * in your cpp file, NOT INLINE in your header file.
         */
        virtual bool DoesDrawing() const;

        /**
         *  MSA Override to return true if your process creates visual output
         *  suitable for raytracing.
         *
         *  Same deal as above, don't implement overrides inline.
         */
        virtual bool DoesRaytracerOutput() const;

        /**
         * Draw all elements in the given scoreboard
         */
        virtual void DrawScoreboard(const DoubleRect& area, Scoreboard* scoreboard);

        /**
         * accessor for the drawing object
         */
        ProcessDrawing& getDrawing();

        /**
         * Accessors for the RaytracerData object
         */
         //issue109msg118
         //virtual RaytracerData * GetRaytracerData() const;
         //virtual void SetRaytracerData(RaytracerData * rdp);

         /**
          *  Accessor for a Process's origin point, i.e.
          *  the point around which an orbital animation will revolve.
          *  Only meaningful for Processes doing Raytracer output, of course.
          */
        virtual DoubleCoordinate GetOrigin() const;

        // MSA 10.11.03 This method is used to supply Processes which take barrier modelling into account
        // with access to the VolumeObjectCoordinator. Most Process derived classes won't bother overriding it.
        virtual void SetVolumeObjectCoordinator(const VolumeObjectCoordinator& voc);
    };

    inline const ScoreboardStratum& Process::GetProcessStratum(void) const
    {
        return (m_scoreboardStratum);
    }

    inline ProcessActivity Process::GetProcessActivity(void) const
    {
        return (m_activity);
    }

    inline void Process::SetName(const std::string& name)
    {
        m_name = name;
    }

    inline ProcessTime_t Process::GetPreviousTimeStamp() const
    {
        return m_previousTimeStamp;
    }

    inline ProcessDrawing& Process::getDrawing()
    {
        return *m_drawing;
    }


    //{ if (m_drawing==0) { throw ProcessDrawingException("No ProcessDrawing for this Process"); } return *m_drawing; }
} /* namespace rootmap */


#define DECLARE_DYNAMIC_CLASS_FORCE_USE(cname)

#endif // #ifndef Process_H
