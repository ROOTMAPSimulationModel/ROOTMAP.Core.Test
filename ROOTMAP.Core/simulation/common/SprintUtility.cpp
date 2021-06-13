/////////////////////////////////////////////////////////////////////////////
// Name:        SprintUtility.cpp
// Purpose:     Implementation of the SprintUtility sub-namespace utility functions
// Created:     2006
// Author:      RvH
// $Date: 2009-07-10 06:43:25 +0800 (Fri, 10 Jul 2009) $
// $Revision: 69 $
// Copyright:   ©2006 University of Tasmania
/////////////////////////////////////////////////////////////////////////////
#include "simulation/common/SprintUtility.h"
#include "simulation/process/common/Process_Dictionary.h"
#include "simulation/common/Types.h"
#include "core/macos_compatibility/MacResourceManager.h"
#include "core/scoreboard/ScoreboardStratum.h"
#include "core/utility/Utility.h"


namespace rootmap
{
    // this declared in Types.h
    wxString& operator<<(CMYSelect cmysel, wxString& str)
    {
        switch (cmysel)
        {
        case Cyan:
            str << "Cyan";
            break;
        case Magenta:
            str << "Magenta";
            break;
        case Yellow:
            str << "Yellow";
            break;
        case Undisplayed:
            str << "Undisplayed";
            break;
        default:
            str << "Unknown";
            break;
        }
        return str;
    }

    std::string& operator+=(CMYSelect cmysel, std::string& str)
    {
        switch (cmysel)
        {
        case Cyan:
            str += "Cyan";
            break;
        case Magenta:
            str += "Magenta";
            break;
        case Yellow:
            str += "Yellow";
            break;
        case Undisplayed:
            str += "Undisplayed";
            break;
        default:
            str += "Unknown";
            break;
        }
        return str;
    }

    namespace SprintUtility
    {
        /*
        sprintLayerSubmenuItem

        CCropSimDoc::BuildLayerTextLists::AddStringToList. That is, it is used to make the strings
        used in all the "Layer" submenus.
        */
        /*
        StringOfViewDir
         */
        std::string StringOfViewDir(ViewDirection aDirection)
        {
            std::string s = "";
            long strindex = STRLvdNoDirection;
            switch (aDirection)
            {
            case vFront: strindex = STRLvdFront;
                break;
            case vSide: strindex = STRLvdSide;
                break;
            case vTop: strindex = STRLvdTop;
                break;
            case vNONE:
            default: strindex = STRLvdNoDirection;
                break;
            }
            MacResourceManager::GetIndString(s, STRLViewDirections, strindex);
            return (s);
        }

        /*
        StringOfViewDir
         */
        std::string StringOfTableAxis(ViewDirection aDirection)
        {
            std::string s = "";
            long strindex = STRLtalNONE;
            switch (aDirection)
            {
            case vFront: strindex = STRLtalFront;
                break;
            case vSide: strindex = STRLtalSide;
                break;
            case vTop: strindex = STRLtalDepth;
                break;
            case vNONE:
            default: strindex = STRLvdNoDirection;
                break;
            }
            MacResourceManager::GetIndString(s, STRLTableAxisLabels, strindex);
            return (s);
        }


        std::string StringOf(SummaryRoutine sr)
        {
            std::string summaryroutingstring;
            sprintSummaryRoutine(summaryroutingstring, sr);
            return (summaryroutingstring);
        }

        void sprintSummaryRoutine(std::string& s, SummaryRoutine sr)
        {
            short string_list_index = STRLsrNone;
            switch (sr)
            {
            case srRootLength: string_list_index = STRLsrRootLength;
                break;
            case srRootLengthDensity: string_list_index = STRLsrRootLengthDensity;
                break;
            case srRootTipCount: string_list_index = STRLsrTipCount;
                break;
            case srRootTipCountDensity: string_list_index = STRLsrTipCountDensity;
                break;
            case srRootBranchCount: string_list_index = STRLsrBranchCount;
                break;
            case srRootBranchCountDensity: string_list_index = STRLsrBranchCountDensity;
                break;
            case srRootVolume: string_list_index = STRLsrVolume;
                break;
            case srRootSurfaceArea: string_list_index = STRLsrSurfaceArea;
                break;
            case srNone:
            default: string_list_index = STRLsrNone;
                break;
            }

            MacResourceManager::GetIndString(s, STRL_SummaryRoutines, string_list_index);
        }

        std::string StringOf(WrapDirection wd)
        {
            std::string wraporderstring;
            sprintWrapOrder(wraporderstring, wd);
            return (wraporderstring);
        }

        std::string StringOf(MessageType_t mt, bool rawIfUnknown)
        {
            std::string s;

            switch (mt)
            {
            case kNullMessage:
                return (std::string("None"));
                break;
            case kInitialiseMessage:
                return (std::string("Initialising"));
                break;
            case kStartMessage:
                return (std::string("Start"));
                break;
            case kPauseMessage:
                return (std::string("Pause"));
                break;
            case kEndMessage:
                return (std::string("End"));
                break;
            case kTerminateMessage:
                return (std::string("Terminating"));
                break;
            case kPreNormalMessage:
                return (std::string("PreNormal"));
                break;
            case kNormalMessage:
                return (std::string("Normal"));
                break;
            case kRegisterRequestMessage:
                return (std::string("Register Request"));
                break;
            case kRegisteredCoughUpMessage:
                return (std::string("Registered Cough Up"));
                break;
            case kUnsolicitedCoughUpMessage:
                return (std::string("Unsolicited Cough Up"));
                break;
            case kReceiverDelayedReactionMessage:
                return (std::string("Delayed Reaction"));
                break;
            case kCoughUpImmediateMessage:
                return (std::string("Cough Up Demand"));
                break;
            case kGeneralPurposeMessage0:
            case kGeneralPurposeMessage1:
            case kGeneralPurposeMessage2:
            case kGeneralPurposeMessage3:
            case kGeneralPurposeMessage4:
            case kGeneralPurposeMessage5:
            case kGeneralPurposeMessage6:
            case kGeneralPurposeMessage7:
            case kGeneralPurposeMessage8:
            case kGeneralPurposeMessage9:
                s = "General Purpose Message ";
                s += Utility::ToString((long)(mt)-(long)(kGeneralPurposeMessage0));
                break;
            case kExternalWakeUpMessage:
                return (std::string("External Wake Up"));
                break;
            case kSpecialInputDataMessage:
                return (std::string("Special Data Input"));
                break;
            case kSpecialOutputDataMessage:
                return (std::string("Special Data Output"));
                break;
            case kSelfCoughedUpImmediateMessage:
            case kOtherCoughedUpImmediateMessage:
                return (std::string("ImmediateCough Up"));
                break;
            case kRegisterGlobalPlantSummaryMessage:
                return (std::string("Register Global Plant Summary"));
                break;
            case kGlobalPlantSummaryRegisteredMessage:
                return (std::string("Global Plant Summary Registered"));
                break;
            case kRegisterScoreboardPlantSummaryMessage:
                return (std::string("Register Scoreboard Plant Summary"));
                break;
            case kScoreboardPlantSummaryRegisteredMessage:
                return (std::string("Scoreboard Plant Summary Registered"));
                break;

            default:
                s = "UNKNOWN";
                if (rawIfUnknown)
                {
                    s.append(" (");
                    s.append(Utility::OSTypeToString((OSType)mt));
                    s.append(1, ')');
                }
                break;
            }

            return s;
        }

        std::string StringOf(const ScoreboardStratum& stratum)
        {
            std::string s;
            sprintGroundVolumeType(s, stratum);
            return s;
        }

        void sprintWrapOrder(std::string& s, WrapDirection wd)
        {
            switch (wd)
            {
            case wraporderBoth:
                s = "Wrap X+Y";
                break;
            case wraporderX:
                s = "Wrap X";
                break;
            case wraporderY:
                s = "Wrap Y";
                break;
            case wraporderNone:
                s = "Wrap None";
                break;
            default:
                s = "Wrap INVALID";
                break;
            }
        }

        void sprintGroundVolumeType(std::string& s, const ScoreboardStratum& volume)
        {
            MacResourceManager::GetIndString(s, STRL_GVTDisplay, volume.value());
        }


        void sprintBoolean(std::string& s, bool fBooleanValue)
        {
            s = ((fBooleanValue) ? "Yes" : "No");
        }

        /////////////////////////////////////////////////////////////////////////////
        //
        //
        // BELOW HERE are functions still in Pascal-String style
        //
        //
        /////////////////////////////////////////////////////////////////////////////

#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintDimension(ScoreboardDimension dimensn, StringPtr the_string)
        {
            switch (dimensn)
            {
            case X: TCLpstrcpy(the_string, "\pX"); break;
            case Y: TCLpstrcpy(the_string, "\pY"); break;
            case Z: TCLpstrcpy(the_string, "\pZ"); break;
            default: TCLpstrcpy(the_string, "\p?"); break;
            }
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

        /*
        sprintCardinal
        Not used yet, either. Planned for the Axes editting dialogs
         */
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintCardinal(long aNumber, StringPtr theString)
        {
            StringPtr after_str;

            switch (aNumber)
            {
            case 0:
                TCLpstrcpy(theString, "\pPrimary ");
                break;
                /*        case 1:
                            theString = "\pFirst ";
                            break;
                        case 2:
                            theString = "\pSecond ";
                            break;
                        case 3:
                            theString = "\pThird ";
                            break;
                        case 4:
                            theString = "\pFourth ";
                            break;*/
            default:
            {
                NumToString(aNumber, theString);

                if ((aNumber > 20) || (aNumber < 4))
                {
                    if ((aNumber % 10) == 1)
                        after_str = "\pst ";            // 21st, 31st, 41st,… 
                    else if ((aNumber % 10) == 2)
                        after_str = "\pnd ";        // 22nd, 32nd, 42nd,… 
                    else if ((aNumber % 10) == 3)
                        after_str = "\prd ";            // 23rd, 33rd, 43rd,… 
                    else // ((aNumber % 10) >3) or ((aNumber % 10) == 0)
                        after_str = "\pth ";            // 24th…30th, 34th…40th, 44th…50th, …
                }
                else // (4 ≤ aNumber ≤ 20) 
                    after_str = "\pth ";                // 5th, 6th,…,19th, 20th 

                TCLpstrcat(theString, after_str);
                break;
            } // default :
            } // switch (aNumber)
        } // sprintCardinal
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintRowLabelAxis(ViewDirection viewdir, Boolean fAboveGround, StringPtr s)
        {
            short index;
            switch (viewdir) {
            case vFront:
            case vSide:
                index = (fAboveGround) ? STRLtalHeight : STRLtalDepth;
                break;
            case vTop:
                index = STRLtalSide;
                break;
            default:
                index = STRLtalNONE;
                break;
            }
            GetIndString(s, (short int)STRLTableAxisLabels, index);
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS


#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintColumnLabelAxis(ViewDirection viewdir, Boolean fAboveGround, StringPtr s)
        {
#pragma unused (fAboveGround)
            short index;
            switch (viewdir) {
            case vFront:
            case vTop:
                index = STRLtalFront;
                break;
            case vSide:
                index = STRLtalSide;
                break;
            default:
                index = STRLtalNONE;
                break;
            }
            GetIndString(s, (short int)STRLTableAxisLabels, index);
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintColumnLabelInterval(ViewDirection viewdir, Boolean fAboveGround, StringPtr s)
        {
            short strl_index = viewdir;
            if ((viewdir == vTop) && (fAboveGround))
            {
                ++strl_index; // changes the string to "Height (cm)" rather than "Depth (cm)"
            }

            GetIndString(s, (short int)STRLTableAxisLabels, (short)(kTableLabelIntervalNONE + strl_index));
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintCMY(CMYSelect cmy, Str255 s)
        {
            switch (cmy)
            {
            case kCyan:
                TCLpstrcpy(s, "\pCyan");
                break;
            case kMagenta:
                TCLpstrcpy(s, "\pMagenta");
                break;
            case kYellow:
                TCLpstrcpy(s, "\pYellow");
                break;
            default:
                TCLpstrcpy(s, "\pNone");
                break;
            }
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

        /*
        Created
        02-Oct-1999

        Description
        places in "s" the string "hh:mm:ss" according to the value of "time".

        Working Notes
        designed as a preliminary "relative-to-zero" time display for the PostOffice/JQuickStatusDirector

        Will need an overload for when "relative to time xx:xx:xx" is required
        */
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintTime(unsigned long int time, Str255 s, bool rawsecs)
        {
            long int seconds, minutes, hours;
            Str255 st;
            Str255 colon = "\p:";

            seconds = time % 60;
            minutes = ((long int)(time / 60)) % 60;
            hours = ((long int)(time / 3600)) % 24;

            NumToString(hours, s);
            TCLpstrcat(s, colon);
            NumToString(minutes, st);
            TCLpstrcat(s, st);
            TCLpstrcat(s, colon);
            NumToString(seconds, st);
            TCLpstrcat(s, st);

            if (rawsecs)
            {
                Str255 open_parentheses = "\p (";
                Str255 close_parentheses = "\p)";

                NumToString(time, st);
                TCLpstrcat(s, open_parentheses);
                TCLpstrcat(s, st);
                TCLpstrcat(s, close_parentheses);
            }
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

        /*
        Created
        02-Oct-1999

        Description
        places in "s" the string "y yrs dd days" according to the value of "time".

        Working Notes
        designed as a preliminary "relative-to-zero" time display for the PostOffice/JQuickStatusDirector.
        Since months are not relevant or even calculatable when this relative to zero is being used, they
        aren't included. However if the number of years is a multiple of 4, i reckon maybe this function should
        use a value of 366 for the number of days in the year.

        Will need an overload for when "relative to time xx/xx/xx" is required
        */
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintDate(unsigned long int time, Str255 s)
        {
            long int days, years;
            Str255 st;
            Str255 days_str = "\p days";
            Str255 years_str = "\p yrs ";

            days = (long)((time / (60 * 60 * 24)) % 365);
            years = (long)((time / (60 * 60 * 24 * 365)));

            NumToString(years, s);
            TCLpstrcat(s, years_str);
            NumToString(days, st);
            TCLpstrcat(s, st);
            TCLpstrcat(s, days_str);
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintSimulationTimeDate(unsigned long int time, Str255 s)
        {
            Str255 time_str = "\p "; // a space, used between the strings, before the date is sprint'd

            sprintDate(time, s);
            TCLpstrcat(s, time_str); // append the space
            sprintTime(time, time_str);
            TCLpstrcat(s, time_str);

        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintLayerSubmenuItem(double floatnum, Str255 astring)
        {
#pragma message (HERE "TODO Fix Pascal-style strings")
            if (Utility::isInfinite(floatnum))
                TCLpstrcpy(astring, "\p∞ ");
            else {
                long integerPart = (long)floatnum;
                long decimalPart = (long)((floatnum - (double)integerPart) * 100);
                Str31 integerStr = "\p";
                Str31 decimalStr = "\p";

                NumToString(integerPart, integerStr);

                if (decimalPart != 0) {
                    TCLpstrcat(integerStr, "\p.");            // decimal point

                    if (decimalPart < 0) decimalPart = -decimalPart;
                    if (decimalPart < 10)
                        TCLpstrcat(integerStr, "\p0");        // Add a leading decimal zero if needed

                    NumToString(decimalPart, decimalStr);    // decimal digits
                    if ((decimalPart % 10) == 0)
                    { // remove the trailing zero by making the length of the string = 1
                        *(char*)decimalStr = 1;
                    }
                    TCLpstrcat(integerStr, decimalStr);
                }

                TCLpstrcpy(astring, integerStr);
            }
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

        /*
        sprintCellText
        In TScoreboardTable::GetCellText. Used ONLY in the drawing of cells in the scoreboard tables.
        Now features its own Number-to-String routine because of the severe failure by sprintf to
        actually convert numbers itself. Thank goodness for the Mac Toolbox.

        1996    now uses the ANSI sprintf, which suddenly works
        97.04.08    depending on the magnitude of the number, uses different sprintf formatting
        97.04.10    for 1-e2 ≤ dd ≤ 1+e5, now calls sprintf form 'g' with 6 significant digits, instead
                of trying to figure out the precision ourself. This eliminates trailing zeros which
                were otherwise being generated by the 'f' format.
         */
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintCellText
        (double        double_num,
            short        cellWidth,
            StringPtr    aString)
        {
#pragma unused(cellWidth)
            double dd = double_num;
            if (isInfinite(dd))
                TCLpstrcpy(aString, "\p∞");
            else
            {
                char c_string[20];

                if (dd > 10000)
                    sprintf(c_string, "%.2e", dd);
                else if ((dd < 0.01) && (dd > 0.000))
                    sprintf(c_string, "%.2e", dd);
                else
                {
                    /*            long mod_val = (long)dd;
                                long precision = 6;

                                // reduce the precision depending on how much room there is
                                while (mod_val >= 10)
                                {
                                    precision--;
                                    mod_val /= 10;
                                }
                                sprintf(c_string, "%.*f", precision, dd);*/
                    sprintf(c_string, "%.5g", dd);
                }

                TCLctopstrcpy(aString, c_string);
            } // end of ‘the number is not infinite’
        } // end of ‘sprintCellText’
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS


        /*
        sprintEditCellText
        In TEditCell::SetValue. Used ONLY in the drawing of EditCells in the scoreboard tables. This routine
        is used instead of sprintCellText (which is very similar) because the ANSI++ sprintf routine seems
        to perform very badly when converting numbers with "+e" in them.

        1996    now uses the ANSI sprintf, which seems to work
        97.04.10    now calls sprintCellText for a unified/harmonised/coupled approach
         */
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintEditCellText
        (double    aDouble,
            StringPtr aString)
        {
            sprintCellText(aDouble, 0, aString);
            return;
        } // end sprintEditCellText
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

        // Function
        //  ConvertWhenToExport
        // 
        // Description
        //  Converts the rules about when to export into human - readable format for
        //  the right hand pane of the IO Display window, when this" is selected.
        // 
        // Working Notes
        // 
        // Input Parameters
        //  s - the Str255 buffer into which to copy the human-readable text
        // 
        // 
        // Modifications :
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintWhenToExport(long when_to_export, long repeat_time, long how_many_times, StringPtr s)
        {
            if (when_to_export == wte_Timestamp)
            {
                if (how_many_times <= 0)
                {
                    TCLpstrcpy(s, "\pNever");
                }
                else if (how_many_times == 1)
                {
                    TCLpstrcpy(s, "\pOnce, at ");
                }
                else if (how_many_times > 1)
                {
                    TCLpstrcpy(s, "\pEvery ");
                }

                // do again, this time add the time for "Once, at " and "Every " occurances,
                if (how_many_times >= 1)
                {
                    Str255 str_how_many_times;
                    Str255 how_often = "\p";
                    Str255 str_years; long years;
                    Str255 str_months; long months;
                    Str255 str_days; long days;
                    Str255 str_hours; long hours;
                    Str255 str_minutes; long minutes;
                    Str255 str_seconds; long seconds;

                    years = repeat_time / (PROCESS_YEARS(1));
                    months = repeat_time / (repeat_time - (PROCESS_YEARS(years)));
                    days = repeat_time / (repeat_time - (PROCESS_YEARS(years)) - (PROCESS_MONTHS(months)));
                    hours = repeat_time / (repeat_time - (PROCESS_YEARS(years)) - (PROCESS_MONTHS(months)) - (PROCESS_DAYS(days)));
                    minutes = repeat_time / (repeat_time - (PROCESS_YEARS(years)) - (PROCESS_MONTHS(months)) - (PROCESS_DAYS(days)) - (PROCESS_HOURS(hours)));
                    seconds = repeat_time / (repeat_time - (PROCESS_YEARS(years)) - (PROCESS_MONTHS(months)) - (PROCESS_DAYS(days)) - (PROCESS_HOURS(hours)) - (PROCESS_MINUTES(minutes)));

#define ADD_PLURAL_AND_SPACE(rr) if ( rr > 1 ) { TCLpstrcat( how_often, "\ps" ); } TCLpstrcat( how_often, "\p " )
                    if (years > 0)
                    {
                        NumToString(years, str_years);
                        TCLpstrcat(how_often, str_years);
                        TCLpstrcat(how_often, "\p year");
                        ADD_PLURAL_AND_SPACE(years);
                    }

                    if (months > 0)
                    {
                        NumToString(months, str_months);
                        TCLpstrcat(how_often, str_months);
                        TCLpstrcat(how_often, "\p month");
                        ADD_PLURAL_AND_SPACE(months);
                    }

                    if (days > 0)
                    {
                        NumToString(days, str_days);
                        TCLpstrcat(how_often, str_days);
                        TCLpstrcat(how_often, "\p day ");
                        ADD_PLURAL_AND_SPACE(days);
                    }

                    if (hours > 0)
                    {
                        NumToString(hours, str_hours);
                        TCLpstrcat(how_often, str_hours);
                        TCLpstrcat(how_often, "\p hour");
                        ADD_PLURAL_AND_SPACE(hours);
                    }

                    if (minutes > 0)
                    {
                        NumToString(minutes, str_minutes);
                        TCLpstrcat(how_often, str_minutes);
                        TCLpstrcat(how_often, "\p minute");
                        ADD_PLURAL_AND_SPACE(minutes);
                    }

                    if (seconds > 0)
                    {
                        NumToString(seconds, str_seconds);
                        TCLpstrcat(how_often, str_seconds);
                        TCLpstrcat(how_often, "\p second");
                        ADD_PLURAL_AND_SPACE(seconds);
                    }

                    TCLpstrcat(s, how_often);

                    if (isInfinite(how_many_times))
                    {
                        TCLpstrcpy(str_how_many_times, "\p (forever)");
                    }
                    else
                    {
                        NumToString(how_many_times, str_how_many_times);
                        TCLpstrcat(s, "\p (");
                        TCLpstrcat(s, str_how_many_times);
                        TCLpstrcat(s, "\p times)");
                    }

                }
            }
            else // export at one of the other time points defined by the WhenToExport enum
            {
#ifdef _CS_DEBUG_
                Debugger();
#endif
            }
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS

        //    +    +    +    +    +    +    +    +    +    +    +    +    +    +
        // Function :        sprintMultiFileExtraRule
        // 
        // Description :    converts the information stored by a JOutputAssociation
        //                for how to print the prepended/appended data onto a filename,
        //                into human readable text (sort of)
        // 
        // Working Notes :    currently doesn't make any attempt to humanify the format string
        // 
        // Input Parameters :
        //                rule -    the MultiFileExtraRule (defined in "JOutputAssociation.h")
        //                format -    the string used to format the string that is appended/prepended/...
        //                s -        the string buffer into which the human readable text is placed
        // 
        // Return Value :    N/A
        // 
        // Modifications :
#if defined ROOTMAP_FIX_PASCAL_STRINGS
        void sprintMultiFileExtraRule(MultiFileExtraRule rule, Str63 format, StringPtr s)
        {
            if (rule & mfer_Append)
            {
                TCLpstrcpy(s, "\pappend ");
            }
            else if (rule & mfer_Prepend)
            {
                TCLpstrcpy(s, "\pprepend ");
            }

            if (rule & mfer_Counter)
            {
                TCLpstrcpy(s, "\pcounter ");
            }
            else if (rule & mfer_Timestamp)
            {
                TCLpstrcpy(s, "\ptimestamp ");
            }

            if (rule & mfer_Append_Same_File)
            {
                TCLpstrcpy(s, "\pappend data to the same file");
            }
            else
            {
                TCLpstrcat(s, "\p<");
                TCLpstrcat(s, format);
                TCLpstrcat(s, "\p>");
            }
        }
#endif // #if defined ROOTMAP_FIX_PASCAL_STRINGS
    } // namespace rmAppUtility
} /* namespace rootmap */


