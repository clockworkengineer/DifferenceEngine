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

//
// Antikythera Classes
//

#include "CTask.hpp"
#include "CLogger.hpp"

// =========
// NAMESPACE
// =========

namespace FPE_ActionFuncs {
    
    // =======
    // IMPORTS
    // =======

    using namespace Antik::Util;
    using namespace Antik::File;

    //
    // Task Action Function Names
    //
    
    constexpr const char *kTaskCopyFileStr      { "Copy File" };
    constexpr const char *kVideoConversionStr   { "Video Conversion" };
    constexpr const char *kEmailFileStr         { "Email Attachment" };
    constexpr const char *kZipFileStr           { "ZIP Archive" };
    constexpr const char *kRunCommandStr        { "Run Command" };

    //
    // Task Action Function Table
    //
    
    struct TaskActionFunc {
        int number;                     // Ordinal Value for task
        std::string name;               // Task Name
        CTask::TaskActionFcn actFcn;    // Action Function Pointer
    };
    
    //
    // Action function data
    //
    
    struct ActFnData {
        std::string watchFolderStr;         // Watch Folder
        std::string destinationFolderStr;   // Destination Folder for copies.
        std::string commandToRunStr;        // Command to run
        bool bDeleteSource;                 // Delete source file
        std::string extensionStr;           // Override extension for destination file
        std::string userNameStr;            // Email account user name
        std::string userPasswordStr;        // Email account user name password
        std::string serverURLStr;           // Email server URL
        std::string emailRecipientStr;      // Email recipient
        std::string mailBoxNameStr;         // IMAP Mailbox name
        std::string zipArchiveStr;          // ZIP archive file
        CLogger::LogStringsFn coutstr;      // coutstr output
        CLogger::LogStringsFn cerrstr;      // cerrstr output
    };

    //
    // Action function initialization / closedown.
    //
    
    bool actionFuncInit(void);
    bool actionFuncCloseDown(void);

    //
    //  Get task details from taskList table
    //

    TaskActionFunc getTaskDetails(int taskNumber);
     
    //  Example lambda to use as action function. Note pre-C++11 you need to put full type in instead of auto
    //
    //  TaskActionFcn sampleLambda = [] (auto filenamePath, auto fnData) 
    //  -> bool { std::cout << "[" << filenamePath+filenameStr << "]" << std::endl; return true; };
    
    //
    // Action function prototypes
    //
    
    bool videoConversion(const std::string& filenamePathStr, const std::shared_ptr<void> fnData);
    bool copyFile(const std::string& filenamePathStr, const std::shared_ptr<void> fnData);
    bool runCommand(const std::string& filenamePathStr, const std::shared_ptr<void> fnData);
    bool emailFile(const std::string& filenamePathStr, const std::shared_ptr<void> fnData);
    bool zipFile(const std::string &filenamePathStr, const std::shared_ptr<void> fnData);

} // namespace FPE_ActionFuncs

#endif /* FPE_ACTIONFUNCS_HPP */

