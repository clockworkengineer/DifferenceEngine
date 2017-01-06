/*
 * File:   FPE_ActionFuncs.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

#ifndef FPE_ACTIONFUNCS_HPP
#define FPE_ACTIONFUNCS_HPP

// C++ STL definitions

#include <string>
#include <memory>
#include <vector>

// Action function data

struct ActFnData {
    std::string  watchFolder;        // Watch Folder
    std::string  destinationFolder;  // Destination Folder for copies.
    std::string  commandToRun;       // Command to run
    bool         bDeleteSource;      // Delete source file
    std::string  extension;          // Override extension for destination file
    void (*coutstr) (const std::vector<std::string>& outstr);  // coutstr output
    void (*cerrstr) (const std::vector<std::string>& errstr);  // cerrstr output
};

//  Example lambda to use as action function. Note pre-C++11 you need to put full type in instead of auto
//
//  TaskActionFcn sampleLambda = [] (auto filenamePath, auto fnData) 
//  -> bool { std::cout << "[" << filenamePath+filenameStr << "]" << std::endl; return true; };

// Action function prototypes

bool handBrake(const std::string& filenamePath, const std::shared_ptr<void> fnData);
bool copyFile(const std::string& filenamePath,  const std::shared_ptr<void> fnData);
bool runCommand(const std::string& filenamePath,  const std::shared_ptr<void> fnData);

#endif /* FPE_ACTIONFUNCS_HPP */

