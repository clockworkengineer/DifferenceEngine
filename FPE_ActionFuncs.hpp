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

//
// C++ STL definitions
//

#include <string>
#include <memory>
#include <vector>

//
// CLogger trace output
//

#include "CLogger.hpp"

using namespace Antik;

// Action function data

struct ActFnData {
    std::string  watchFolderStr;        // Watch Folder
    std::string  destinationFolderStr;  // Destination Folder for copies.
    std::string  commandToRunStr;       // Command to run
    bool         bDeleteSource;         // Delete source file
    std::string  extensionStr;          // Override extension for destination file
    std::string  userNameStr;           // Email account user name
    std::string  userPasswordStr;       // Email account user name password
    std::string  serverURLStr;          // Email SMTP server URL
    std::string  emailRecipientStr;     // Email recipient
    std::string  mailBoxNameStr;        // IMAP Mailbox name
    std::string  zipArchiveStr;         // ZIP archive file
    CLogger::LogStringsFn coutstr;      // coutstr output
    CLogger::LogStringsFn cerrstr;      // cerrstr output
};

//  Example lambda to use as action function. Note pre-C++11 you need to put full type in instead of auto
//
//  TaskActionFcn sampleLambda = [] (auto filenamePath, auto fnData) 
//  -> bool { std::cout << "[" << filenamePath+filenameStr << "]" << std::endl; return true; };

// Action function prototypes

bool handBrake(const std::string& filenamePathStr, const std::shared_ptr<void> fnData);
bool copyFile(const std::string& filenamePathStr,  const std::shared_ptr<void> fnData);
bool runCommand(const std::string& filenamePathStr,  const std::shared_ptr<void> fnData);
bool emailFile(const std::string& filenamePathStr,  const std::shared_ptr<void> fnData);
bool zipFile(const std::string &filenamePathStr, const std::shared_ptr<void> fnData);

#endif /* FPE_ACTIONFUNCS_HPP */

