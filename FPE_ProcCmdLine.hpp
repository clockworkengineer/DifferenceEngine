/*
 * File:   FPE_ProcCmdLine.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 * 
 */

#ifndef FPE_PROCCMDLINE_HPP
#define FPE_PROCCMDLINE_HPP

//
// C++ STL definitions
//

#include <string>
#include <unordered_map>

//
// Program components.
//

#include "FPE_ActionFuncs.hpp"

// =========
// NAMESPACE
// =========

namespace FPE_ProcCmdLine {
    
    //
    // Command line parameter data
    //

    struct ParamArgData {
        FPE_ActionFuncs::TaskActionFunc taskFunc; // Task action function details
        std::unordered_map<std::string, std::string> params; // Parameter map
        int maxWatchDepth{ -1};         // Watch depth -1=all,0=just watch folder,1=next level down etc.
        bool bDeleteSource{ false};     // Delete source file
        bool bQuiet{ false};            // Quiet mode no tracing output.
        int killCount {0};              // Kill Count
        bool bSingleThread{ false};     // Run task in the main thread
    };

    //
    // Handbrake command
    //
    
    constexpr const char *kHandbrakeCommandStr = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";
     
    // Get command line parameters

    ParamArgData fetchCommandLineArgumentData(int argc, char** argv);
    

} // namespace FPE_ProcCmdLine

#endif /* FPE_PROCCMDLINE_HPP */

