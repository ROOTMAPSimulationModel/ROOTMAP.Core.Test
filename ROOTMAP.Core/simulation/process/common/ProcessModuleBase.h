#ifndef ProcessModuleBase_H
#define ProcessModuleBase_H
/**
 * ProcessModuleBase
 * NOTE!! Indexing (of characteristics in the Scoreboard) is 1-based.
 * NOTE!! Numbering (of characteristics within the m_characteristicDescriptors) is 1-based.
*/
#include "core/common/Structures.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/scoreboard/ScoreboardBox.h"
#include "core/log/Logger.h"

#include <vector>
#include "wx/object.h"

namespace rootmap
{
    class Scoreboard;
    class ScoreboardCoordinator;
    class CharacteristicDAI;
    struct CharacteristicDescriptor;


    class ProcessModuleBase : public wxObject
    {
        DECLARE_ABSTRACT_CLASS(ProcessModuleBase)

    public:
        ProcessModuleBase();
        virtual ~ProcessModuleBase();

        void AddCharacteristic(CharacteristicDescriptor* cd);
        void AddCharacteristic(CharacteristicDAI& data);

        void RegisterAllCharacteristics(ScoreboardCoordinator* scoreboardcoordinator);

        virtual bool UsesSpecialPerBoxInformation();
        long GetNumCharacteristics(void);

        virtual long FindCharacteristicNumberOfCharacteristicID(long characteristic_id);
        long FindCharacteristicNumberOfCharacteristicIndex(CharacteristicIndex characteristic_index);
        const CharacteristicDescriptor& FindCharacteristicIndex(CharacteristicIndex characteristic_index);
        const CharacteristicDescriptor& FindCharacteristicID(long characteristic_id);
        const CharacteristicDescriptor& FindCharacteristicName(const std::string& name, long* characteristic_number);
        const CharacteristicDescriptor& FindVariantCharacteristicName(const std::string& name, long* characteristic_number);

        long GetCharacteristicID(long characteristic_number);
        /**
         * Returns the index within scoreboard boxes of this process module's
         * "characteristic_number"th summary characteristic
         */
        long GetCharacteristicIndex(long characteristic_number);
        long GetSpecialCharacteristicIndex();

        double GetCharacteristicMinimum(long characteristic_number);
        double GetCharacteristicMaximum(long characteristic_number);
        double GetCharacteristicDefault(long characteristic_number);

        bool CharacteristicIsVisible(long characteristic_number);
        bool CharacteristicIsEdittable(long characteristic_number);
        bool CharacteristicNeedsSaving(long characteristic_number);
        bool CharacteristicIsSpecial(long characteristic_number);

        const ScoreboardStratum& GetCharacteristicScoreboardStratum(long characteristic_number);

        // 
        const std::string& GetCharacteristicUnits(long characteristic_number);
        const std::string& GetCharacteristicName(long characteristic_number);
        void GetCharacteristicUnits(std::string& aString, long characteristic_number);
        void GetCharacteristicName(std::string& aString, long characteristic_number);

        const CharacteristicDescriptor& GetCharacteristicDescriptor(long characteristic_number) const;

        // Setting and/or reading in defaults
        virtual void InitialiseScoreboardDefaultValues(Scoreboard* scoreboard);

        virtual void DisposeSpecialInformation(Scoreboard* scoreboard);

        /** Returns the number of characteristics that should be saved. A legacy
         *  approach, as this value was previously required to be determined to
         *  allow saving into a little bit of pre-information header.
         *  Usually different from GetNumCharacteristics(), which includes
         *  characteristics that need not be saved like interprocess buffers &
         *  requests
         */
        virtual long GetNumSavedCharacteristics();

        /**
        */
        virtual long GetNumVisibleCharacteristics();

        // just abstract placeholder functions, implemented by the "Process" class.
        // Needed in ResetValues()
        virtual void GetProcessName(std::string& aString) const = 0;
        virtual const std::string& GetProcessName() const = 0;
        virtual const ScoreboardStratum& GetProcessStratum() const = 0;

    private:
        RootMapLoggerDeclaration();

        typedef std::vector<CharacteristicDescriptor *> CharacteristicDescriptorCollection;
        CharacteristicDescriptorCollection m_characteristicDescriptors;

        // the number of the characteristic descriptor in which the
        // special per-box info information is kept
        long m_specialInformationNumber;
    };

    inline const CharacteristicDescriptor& ProcessModuleBase::GetCharacteristicDescriptor(long characteristic_number) const
    {
        return (*(m_characteristicDescriptors[--characteristic_number]));
    }

    inline long ProcessModuleBase::GetSpecialCharacteristicIndex()
    {
        return (GetCharacteristicIndex(m_specialInformationNumber));
    }
} /* namespace rootmap */

#endif // #ifndef ProcessModuleBase_H
