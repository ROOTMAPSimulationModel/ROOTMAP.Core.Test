#include "simulation/common/Types.h"
#include "core/macos_compatibility/macos_compatibility.h"
#include "core/common/Types.h"
#include "core/common/Structures.h"
#include "core/common/Constants.h"

//#include "core/scoreboard/ScoreboardStratum.h"
//#include "file/output/OutputAssociation.h"


namespace rootmap
{
    class ScoreboardStratum;

    namespace SprintUtility
    {
        // String Input
        std::string StringOfViewDir(ViewDirection viewdir);
        std::string StringOfTableAxis(ViewDirection aDirection);

        std::string StringOf(SummaryRoutine sr);
        std::string StringOf(WrapDirection wd);

        std::string StringOf(const ScoreboardStratum& stratum);

        /**
         * @param mt the MessageType_t
         * @param rawIfUnknown if the message type is unknown, appends a
         *        string conversion of the 4 bytes to char
         */
        std::string StringOf(MessageType_t mt, bool rawIfUnknown = false);

        void sprintSummaryRoutine(std::string& s, SummaryRoutine sr);
        void sprintWrapOrder(std::string& s, WrapDirection wd);
        void sprintGroundVolumeType(std::string& s, const ScoreboardStratum& volume);
        void sprintBoolean(std::string& s, bool fBooleanValue);

#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintLayerSubmenuItem(double floatnum, Str255 astring);
        void sprintCellText(double aDouble, short cellWidth, StringPtr aString);
        void sprintEditCellText(double aDouble, StringPtr aString);
        void sprintDimension(ScoreboardDimension dimensn, StringPtr the_string);
        void sprintCardinal(long aNumber, StringPtr theString);
        void sprintRowLabelAxis(ViewDirection viewdir, Boolean fAboveGround, StringPtr s);
        void sprintColumnLabelAxis(ViewDirection viewdir, Boolean fAboveGround, StringPtr s);
        void sprintColumnLabelInterval(ViewDirection viewdir, Boolean fAboveGround, StringPtr s);
        void sprintCMY(CMYSelect cmy, Str255 s);
        void sprintTime(unsigned long int time, Str255 s, bool rawsecs = true);
        void sprintDate(unsigned long int time, Str255 s);
        void sprintSimulationTimeDate(unsigned long int time, Str255 s);
        void sprintWhenToExport(long when_to_export, long repeat_time, long how_many_times, StringPtr s);
        void sprintMultiFileExtraRule(MultiFileExtraRule rule, Str63 format, StringPtr s);
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS
    } // namespace rmAppUtility
} /* namespace rootmap */
