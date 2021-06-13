/**
 *    MSA 11.04.11
 *    Implementing precompiled header for Boost functionality.
 *    Boost::geometry in particular produces a LOT of compilation warnings, which are harmless - 
 *    unreferenced formal parameters and the like - but which clog the output window with noise.
 *    By precompiling them once we can reduce compilation noise and compilation time.
 */

#ifndef ScoreboardPrecomp_H
#define ScoreboardPrecomp_H

#include "simulation/process/common/Process.h"
#include "simulation/scoreboard/Scoreboard.h"
#include "simulation/process/modules/VolumeObject.h"

#endif
