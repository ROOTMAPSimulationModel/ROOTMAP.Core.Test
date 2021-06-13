/* ProcessModuleBase
A class for handling scoreboard storage for process modules. This mainly involves characteristics
(simple floating-point values per-box) and special per-box information */
//#include "Structures.h"
//#include "Utilities.h"

//#include "Process.h"
#include "simulation/process/common/ProcessModuleBase.h"
#include "simulation/process/common/CharacteristicDescriptor.h"

#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/scoreboard/ScoreboardCoordinator.h"

#include "simulation/data_access/interface/CharacteristicDAI.h"

#include "core/common/ExtraStuff.h"
#include "core/common/RmAssert.h"
#include "core/utility/Utility.h"

#include "boost/algorithm/string.hpp"
#include "boost/geometry/geometry.hpp"

#include <stdexcept>
#include <limits>


namespace rootmap
{
    IMPLEMENT_ABSTRACT_CLASS(ProcessModuleBase, wxObject)

        RootMapLoggerDefinition(ProcessModuleBase);

#if defined _DEBUG
#define VALIDATE_INDEX(x) RmAssert((x<=GetNumCharacteristics()),"Characteristic number out of range"); RmAssert((x>0),"Characteristic number below zero");
#else
#define VALIDATE_INDEX(x) if ((x > GetNumCharacteristics()) || (x < 0)) { throw std::range_error("Invalid Characteristic Number"); }
#endif

    // This value is generally returned by functions that in practice would have
    // already throw()n or RmAssert()d, but require a return value to pacify the
    // compiler (and quite rightly, too)
    const double DUMMY_DOUBLE_VALUE = std::numeric_limits<double>::quiet_NaN();

    ProcessModuleBase::ProcessModuleBase()
        : m_specialInformationNumber(0)
    {
        RootMapLoggerInitialisation("rootmap.ProcessModuleBase");
    }


    void ProcessModuleBase::AddCharacteristic(CharacteristicDescriptor* cd)
    {
        m_characteristicDescriptors.push_back(cd);

        LOG_INFO << "Added Characteristic to Process " << GetProcessName()
            << "{Name:" << cd->Name
            << ", Units:" << cd->Units
            << ", ID:" << cd->id
            << ", Stratum:" << cd->GetScoreboardStratum().toString()
            << ", Minimum:" << cd->Minimum
            << ", Maximum:" << cd->Maximum
            << ", Default:" << cd->Default
            << ", Visible:" << Utility::ToString(cd->Visible, Utility::bsrYesNo)
            << ", Editable:" << Utility::ToString(cd->Edittable, Utility::bsrYesNo)
            << "}";
    }

    void ProcessModuleBase::AddCharacteristic(CharacteristicDAI& data)
    {
        CharacteristicDescriptor* cd = new CharacteristicDescriptor;

        cd->SetID(data.getIdentifier());
        cd->SetName(data.getName());
        cd->SetUnits(data.getUnits());
        cd->SetScoreboardStratum(data.getStratum());
        cd->SetMinimum(data.getMinimum());
        cd->SetMaximum(data.getMaximum());
        cd->SetDefault(data.getDefault());
        cd->SetVisible(data.isVisible());
        cd->SetEdittable(data.isEdittable());
        cd->SetSavable(data.isSavable());
        cd->SetSpecialPerBoxInfo(data.hasSpecialPerBoxInfo());

        AddCharacteristic(cd);
    }


    //
    // Function:
    //  RegisterAllCharacteristics
    //
    // Description:
    //  Iterate through all the characteristics currently in storage and register
    //  them with the scoreboardcoordinator
    //
    // Working Notes:
    //  Performed once, just after the scoreboard is constructed and just before
    //  the scoreboard is filled with defaults.
    //
    // Modifications:
    //
    void ProcessModuleBase::RegisterAllCharacteristics(ScoreboardCoordinator* scoreboardcoordinator)
    {
        std::vector<CharacteristicDescriptor *>::iterator it = m_characteristicDescriptors.begin();
        while (it != m_characteristicDescriptors.end())
        {
            scoreboardcoordinator->RegisterCharacteristic(*it);
            ++it;
        }
    }

    //
    // Function:
    //  Destructor
    //
    // Description:
    //
    // Working Notes:
    //
    // Modifications:
    // 
    ProcessModuleBase::~ProcessModuleBase()
    {
        for (std::vector<CharacteristicDescriptor *>::iterator it = m_characteristicDescriptors.begin();
            it != m_characteristicDescriptors.end();
            ++it)
        {
            delete (*it);
        }
    }

    //
    // Function:
    //  GetNumCharacteristics
    //
    // Description:
    //
    // Modifications:
    //
    long ProcessModuleBase::GetNumCharacteristics(void)
    {
        return m_characteristicDescriptors.size();
    }

    bool ProcessModuleBase::UsesSpecialPerBoxInformation()
    {
        return (m_specialInformationNumber > 0);
    }


    //
    // Function:
    //  GetNumSavedCharacteristics
    //
    // Description:
    //
    // Created:
    //  19980902 RvH
    //
    // Modifications:
    //  19980909 RvH - Multifix
    //
    long ProcessModuleBase::GetNumSavedCharacteristics()
    {
        long total_characteristics = GetNumCharacteristics();
        long number_savable_characteristics = 0;
        long characteristic_number = 1;

        while (characteristic_number <= total_characteristics)
        {
            if (CharacteristicNeedsSaving(characteristic_number))
                ++number_savable_characteristics;

            ++characteristic_number;
        }

        return (number_savable_characteristics);
    }

    //
    // Function:
    //  GetNumVisibleCharacteristics
    //
    // Description:
    //
    // Modifications:
    //  19990112 RvH - copied from GetNumSavedCharacteristics and modified
    //
    long ProcessModuleBase::GetNumVisibleCharacteristics()
    {
        long total_characteristics = GetNumCharacteristics();
        long number_visible_characteristics = 0;
        long characteristic_number = 1;

        while (characteristic_number <= total_characteristics)
        {
            if (CharacteristicIsVisible(characteristic_number))
                ++number_visible_characteristics;

            ++characteristic_number;
        }

        return (number_visible_characteristics);
    }

    //
    // Function:
    //  GetCharacteristicIndex
    //
    // Description:
    //  Returns the index of the characteristic with the ID = characteristic_id.
    //
    // Modifications:
    //  19980902 RvH - MultiFix; Merged the "if==1" and "if>1" clauses
    //  19980912 RvH - Obsolete
    //
    //long Process::GetCharacteristicNumber(long characteristic_id)
    //{
    //    if (GetNumCharacteristics() >= 1)
    //        return (characteristic_id - pID);
    //    //else            PZeroCharacteristics (and PlantCoordinator, while this isnÕt overridden)
    //        return ((long)kProcessInvalidCharacteristicNumber);
    //}

    long ProcessModuleBase::FindCharacteristicNumberOfCharacteristicID(long characteristic_id)
    {
        long characteristic_number = 1;
        for (CharacteristicDescriptorCollection::iterator iter = m_characteristicDescriptors.begin();
            iter != m_characteristicDescriptors.end();
            ++iter
            )
        {
            if (GetCharacteristicID(characteristic_number) == characteristic_id)
                return (characteristic_number);

            ++characteristic_number;
        }

        return (-1);
    }

    long ProcessModuleBase::FindCharacteristicNumberOfCharacteristicIndex(CharacteristicIndex characteristic_index)
    {
        long characteristic_number = 1;
        for (CharacteristicDescriptorCollection::iterator iter = m_characteristicDescriptors.begin();
            iter != m_characteristicDescriptors.end();
            ++iter
            )
        {
            if (GetCharacteristicIndex(characteristic_number) == characteristic_index)
                return (characteristic_number);

            ++characteristic_number;
        }

        return (-1);
    }

    const CharacteristicDescriptor& ProcessModuleBase::FindCharacteristicIndex(CharacteristicIndex characteristic_index)
    {
        for (CharacteristicDescriptorCollection::iterator iter = m_characteristicDescriptors.begin();
            iter != m_characteristicDescriptors.end();
            ++iter
            )
        {
            if ((*iter)->ScoreboardIndex == characteristic_index)
            {
                return (*(*iter));
            }
        }

        return (CharacteristicDescriptor::Null);
    }

    const CharacteristicDescriptor& ProcessModuleBase::FindCharacteristicID(long characteristic_id)
    {
        for (CharacteristicDescriptorCollection::iterator iter = m_characteristicDescriptors.begin();
            iter != m_characteristicDescriptors.end();
            ++iter
            )
        {
            if ((*iter)->id == characteristic_id)
            {
                return (*(*iter));
            }
        }

        return (CharacteristicDescriptor::Null);
    }

    const CharacteristicDescriptor& ProcessModuleBase::FindCharacteristicName(const std::string& name, long* characteristic_number)
    {
        // follows different logic because we need to return the characteristic
        // number as an out parameter
        for (unsigned long cnum = 1; cnum <= m_characteristicDescriptors.size(); ++cnum)
        {
            const std::string& str = GetCharacteristicName(cnum);
            if (str == name)
            {
                *characteristic_number = cnum;
                return (GetCharacteristicDescriptor(cnum));
            }
        }
        //     for ( CharacteristicDescriptorCollection::iterator iter = m_characteristicDescriptors.begin();
        //             iter != m_characteristicDescriptors.end();
        //             ++iter
        //         )
        //     {
        //         if ((*iter)->Name == name)
        //         {
        //             return (*(*iter));
        //         }
        //     }

        return (CharacteristicDescriptor::Null);
    }

    const CharacteristicDescriptor& ProcessModuleBase::FindVariantCharacteristicName(const std::string& name, long* characteristic_number)
    {
        for (unsigned long cnum = 1; cnum <= m_characteristicDescriptors.size(); ++cnum)
        {
            const std::string& str = GetCharacteristicName(cnum);
            if (str == name
                ||
                (str.find(name) != std::string::npos)) // Actual Characteristic name contains the parameter string
            {
                *characteristic_number = cnum;
                return (GetCharacteristicDescriptor(cnum));
            }
        }

        return (CharacteristicDescriptor::Null);
    }

    long ProcessModuleBase::GetCharacteristicID(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->id;
    }

    double ProcessModuleBase::GetCharacteristicMinimum(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Minimum;
    }

    double ProcessModuleBase::GetCharacteristicMaximum(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Maximum;
    }

    double ProcessModuleBase::GetCharacteristicDefault(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Default;
    }

    long ProcessModuleBase::GetCharacteristicIndex(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->ScoreboardIndex;
    }

    const std::string& ProcessModuleBase::GetCharacteristicUnits(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Units;
    }

    const std::string& ProcessModuleBase::GetCharacteristicName(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Name;
    }

    void ProcessModuleBase::GetCharacteristicName(std::string& aString, long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            aString = GetCharacteristicName(characteristic_number);
    }

    void ProcessModuleBase::GetCharacteristicUnits(std::string& aString, long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            aString = GetCharacteristicUnits(characteristic_number);
    }

    Boolean ProcessModuleBase::CharacteristicIsVisible(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Visible;
    }

    Boolean ProcessModuleBase::CharacteristicIsEdittable(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Edittable;
    }

    Boolean ProcessModuleBase::CharacteristicNeedsSaving(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->Savable;
    }

    Boolean ProcessModuleBase::CharacteristicIsSpecial(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->SpecialPerBoxInfo;
    }

    const ScoreboardStratum& ProcessModuleBase::GetCharacteristicScoreboardStratum(long characteristic_number)
    {
        VALIDATE_INDEX(characteristic_number)
            return (m_characteristicDescriptors[--characteristic_number])->GetScoreboardStratum();
    }


    void ProcessModuleBase::InitialiseScoreboardDefaultValues(Scoreboard* scoreboard)
    {
        for (CharacteristicDescriptorCollection::iterator iter = m_characteristicDescriptors.begin();
            iter != m_characteristicDescriptors.end();
            ++iter)
        {
            scoreboard->RegisterCharacteristic(*iter);
            if (!(*iter)->SpecialPerBoxInfo)
            {
                scoreboard->FillCharacteristicWithValue((*iter)->ScoreboardIndex, (*iter)->Default);
            }
            else
            {
                scoreboard->FillSpecialDataWithNull((*iter)->ScoreboardIndex);
            }
        }
    }


    //
    // Function:
    //  DisposeSpecialInformation
    //
    //
    // Description:
    //  This function is called by the default Terminate() for all process modules
    //  that have declared that they store special information in the scoreboard
    //  boxes. Naturally, then, this function does nothing because by default
    //  process modules do not store special per-box information.
    //
    //
    // Modifications:
    //  YYYYMMDD RvH - 
    //
    void ProcessModuleBase::DisposeSpecialInformation(Scoreboard* /*scoreboard*/)
    {
    }


#if defined ROOTMAP_TODO
    void ProcessModuleBase::FillCharacteristicDescriptor(CharacteristicDescriptor * d, long characteristic_number)
    {
        d->characteristic_id = GetCharacteristicID(characteristic_number);
        d->characteristic_number = characteristic_number;
        d->characteristic_minimum = GetCharacteristicMinimum(characteristic_number);
        d->characteristic_maximum = GetCharacteristicMaximum(characteristic_number);
        d->characteristic_default = GetCharacteristicDefault(characteristic_number);
        d->characteristic_visible = (long)CharacteristicIsVisible(characteristic_number);
        d->characteristic_edittable = (long)CharacteristicIsEdittable(characteristic_number);
        d->characteristic_savable = (long)CharacteristicNeedsSaving(characteristic_number);
        d->characteristic_special = (long)CharacteristicIsSpecial(characteristic_number);
    }

    /* ReadCharacteristicData
    Subclasses should override this member function if they store information primarily in a format
    OTHER than sumarised per scoreboard box. Some (most?) process modules will store information
    only in summary-per-scoreboard-box form. This is the default functionality that this member provides.

    98.09.12    updated to use CharacteristicDataDescription and Scoreboard::GetNumberOfBoxes().
            renamed from ReadProcessData */
    void ProcessModuleBase::ReadCharacteristicData
    (JSimulationFile *file,
        Scoreboard *scoreboard,
        long characteristic_number)
    {
        //    if (description.characteristic_data_length > scoreboard_size)
        //    {
        // raise an alert stating that the file data is longer than we expected, and hence
        // some data will be ignored (and lost). However, since we don't expect to deal
        // with this as yet, and hence haven't decided HOW to deal with it, we will just
        // do our trick of filling the scoreboard with default values. */
        //        ResetValues(scoreboard, __nullptr);
        // Of course, we also need to read in the data that we can't or aren't going to use.
        //        input_file->Pass_Over(description.characteristic_data_length);
        //    }
        //    else if (description.characteristic_data_length < scoreboard_size)
        //    {
        // raise an alert stating that there is less file data than we expected, and hence
        // some data will be repeated. However, since we don't expect to deal with this
        // as yet, and hence haven't decided HOW to deal with it, we will just do our
        // trick of filling the scoreboard with default values.
        //        ResetValues(scoreboard, __nullptr);
        // Of course, we also need to read in the data that we can't or aren't going to use.
        //        input_file->Pass_Over(description.characteristic_data_length);
        //    }
        //    else
        //    {
        // Read the data and stuff it straight into the scoreboard.
        //        characteristic_number = GetCharacteristicNumber(description.characteristic_id);
        CharacteristicIndex ci = GetCharacteristicIndex(characteristic_number);
        scoreboard->FillFromFile(ci, file);
        //    }
    }

    /* SaveCharacteristicData
    Called by Simulation::SaveProcessData().

    OVERRIDE NOTE: if your process module wants to save special per-box information to the file,
    this is the function to override. Special data that is not stored in scoreboard boxes but
    that you want to save should be saved in your override of Process::SaveProcessData()

    98.09.09    'P?xt' merge. moved to ProcessModuleBase
    98.09.12    fixed/updated
    1999.08.27 updated to actually do some saving. Nearly empty, now */
    void ProcessModuleBase::SaveCharacteristicData
    (JSimulationFile *output_file,
        Scoreboard *scoreboard,
        long characteristic_number)
    {
        // now write the scoreboard data to the file
        scoreboard->EmptyToFile(GetCharacteristicIndex(characteristic_number), output_file);
    }
#endif // #if defined ROOTMAP_TODO
} /* namespace rootmap */


