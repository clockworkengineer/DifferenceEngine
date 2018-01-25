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
// C++ STL
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
    // Command line option data. Note all option values are treated as strings
    // so they may be stored in the optionsMap unordered map.
    //

    struct FPEOptions {
        FPE_ActionFuncs::TaskActionFunc taskFunc;                // Task action function details
        std::unordered_map<std::string, std::string> optionsMap; // Options map
     };

    //
    // Handbrake command
    //
    
    constexpr const char *kHandbrakeCommand = "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";
     
    // Get command line options

    FPEOptions fetchCommandLineOptionData(int argc, char** argv);

    //
    //  Get option map value and return as type T
    //
    
    template <typename T>
    T getOption(const FPEOptions& optionData, const std::string& option) {

        T value;
        
        auto entry = optionData.optionsMap.find(option);
        if (entry != optionData.optionsMap.end()) {
            std::istringstream optionStingStream {entry->second};
            optionStingStream >> value;
            return (value);
        } else {
            return ( T { }); // Return default for type
        }
    }

} // namespace FPE_ProcCmdLine

#endif /* FPE_PROCCMDLINE_HPP */

