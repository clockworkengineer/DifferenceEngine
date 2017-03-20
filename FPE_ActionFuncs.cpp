#include "HOST.hpp"
/*
 * File:   FPE_ActionFuncs.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: Task Action Functions 
//
// Description: The task action functions used by the FPE and their support functions.
// 
// Currently four actions are provided.
//
// 1) File copy
// 2) Video file conversion (using handbrake)
// 3) Run shell 
// 4) Email file as attachment (if the server is IMAP then mail message is appended to a mailbox)
// 5) Append file to a ZIP archive
// 
// Dependencies: C11++, classes (CFileTask, CMailSMTP, CMailIMAP, CMailIMAPParse,
//               CFileZIP, CFileMIME, CLogger), Linux, Boost C++ Libraries.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <system_error>

//
// Task Action functions
//

#include "FPE_ActionFuncs.hpp"

//
// Antikythera Classes
//

#include "CFileZIP.hpp"
#include "CFileTask.hpp"  
#include "CMailSMTP.hpp"
#include "CMailIMAP.hpp"
#include "CMailIMAPParse.hpp"
#include "CFileMIME.hpp"

using namespace Antik;
        
//
//
// Process wait definitions
//

#include <sys/wait.h>

//
// Boost file system and format libraries definitions
//

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

// ===============
// LOCAL FUNCTIONS
// ===============

//
// Fork and execute Shell command
//

static int forkCommand(char *const argv[]) {

    pid_t pid; // Process id
    int status; // wait status
    int exitStatus = 0; // child exit status

    if ((pid = fork()) < 0) { /* fork a child process           */
        
        throw std::system_error(std::error_code(errno, std::system_category()), "ERROR: forking child process failed:");
 
    } else if (pid == 0) { /* for the child process: */

        // Redirect stdout/stderr

        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        if (execvp(*argv, argv) < 0) { /* execute the command  */
            exit(1);
        }

    } else { /* for the parent:      */

        while (wait(&status) != pid) { /* wait for completion  */
            continue;
        }

        if (WIFEXITED(status)) { // Set any exit status
            exitStatus = WEXITSTATUS(status);
        }

    }

    return (exitStatus);
}

//
// Run shell command. Split command into argv components before passing onto exec.
// All heap memory cleaned up when function returns due to unique_pointers.
//

static int runShellCommand(const std::string& shellCommandStr) {

    int exitStatus = 0;
    int argc = 0;

    std::vector<char *> argvs;
    std::unique_ptr<char*> argv;
    std::unique_ptr<char> commandStr{ new char[shellCommandStr.length() + 1]};

    // Take a 'C' string copy

     shellCommandStr.copy(commandStr.get(), shellCommandStr.length());
     commandStr.get()[shellCommandStr.length()] = 0; // null terminate

    // Loop through command splitting into substrings for argv

    char *p2 = strtok(commandStr.get(), " ");

    while (p2) {
        argvs.push_back(p2);
        p2 = strtok(0, " ");
    }

    // Allocate argv array and copy vector over

    argv.reset(new char*[argvs.size() + 1]);

    for (char *arg : argvs) {
        argv.get()[argc++] = arg;
    }

    argv.get()[argc] = 0; // Last element null

    // Fork command

    exitStatus = forkCommand(argv.get());

    return (exitStatus);

}

// ================
// PUBLIC FUNCTIONS
// ================

//
// Run a specified command on the file (%1% source, %2% destination)
//

bool runCommand(const std::string &filenamePathStr, const std::shared_ptr<void>fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr);
    fs::path destinationFile(funcData->destinationFolderStr + sourceFile.filename().string());

    // Create correct command for whether source and destination specified or just source or none

    bool srcFound = (funcData->commandToRunStr.find("%1%") != std::string::npos);
    bool dstFound = (funcData->commandToRunStr.find("%2%") != std::string::npos);

    std::string commandStr;
    if (srcFound && dstFound) {
        commandStr = (boost::format(funcData->commandToRunStr) % sourceFile.string() % destinationFile.string()).str();
    } else if (srcFound) {
        commandStr = (boost::format(funcData->commandToRunStr) % sourceFile.string()).str();
    } else {
        commandStr = funcData->commandToRunStr;
    }

    funcData->coutstr({commandStr});

    auto result = 0;
    if ((result = runShellCommand(commandStr)) == 0) {
        bSuccess = true;
        funcData->coutstr({"Command success." });
        if (funcData->bDeleteSource) {
            funcData->coutstr({"DELETING SOURCE [", sourceFile.string(), "]"});
            fs::remove(sourceFile);
        }
    } else {
           funcData->cerrstr({"Command error: ", std::to_string(result)});
    }

    return (bSuccess);

}

//
// Video file conversion action function. Convert passed in file to MP4 using Handbrake.
//

bool handBrake(const std::string& filenamePathStr, const std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr);
    fs::path destinationFile(funcData->destinationFolderStr);

    destinationFile /= sourceFile.stem().string();

    if (funcData->extensionStr.length() > 0) {
        destinationFile.replace_extension(funcData->extensionStr);
    } else {
        destinationFile.replace_extension(".mp4");
    }

    // Convert file

    std::string commandStr = (boost::format(funcData->commandToRunStr) % sourceFile.string() % destinationFile.string()).str();

    funcData->coutstr({"Converting file [", sourceFile.string(), "] To [", destinationFile.string(), "]"});

    auto result = 0;
    if ((result = runShellCommand(commandStr)) == 0) {
        bSuccess = true;
        funcData->coutstr({"File conversion success."});
        if (funcData->bDeleteSource) {
            funcData->coutstr({"DELETING SOURCE [", sourceFile.string(), "]"});
            fs::remove(sourceFile);
        }

    } else {
        funcData->coutstr({"File conversion error: ", std::to_string(result)});
    }

    return (bSuccess);

}

//
// Copy file action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.
//

bool copyFile(const std::string &filenamePathStr, const std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr);

    // Destination file path += ("filename path" - "watch folder path")

    fs::path destinationFile(funcData->destinationFolderStr +
            filenamePathStr.substr((funcData->watchFolderStr).length()));

    // Construct full destination path if needed

   if (!fs::exists(destinationFile.parent_path())) {
        if (fs::create_directories(destinationFile.parent_path())) {
            funcData->coutstr({"CREATED :", destinationFile.parent_path().string()});
        } else {
            funcData->cerrstr({"CREATED FAILED FOR :", destinationFile.parent_path().string()});
        }
    }

    // Currently only copy file if it doesn't already exist.

    if (!fs::exists(destinationFile)) {
        funcData->coutstr({"COPY FROM [", sourceFile.string(), "] TO [", destinationFile.string(), "]"});
        fs::copy_file(sourceFile, destinationFile, fs::copy_option::none);
        bSuccess = true;
        if (funcData->bDeleteSource) {
            funcData->coutstr({"DELETING SOURCE [", sourceFile.string(), "]"});
            fs::remove(sourceFile);
        }

    } else {
        funcData->coutstr({"DESTINATION ALREADY EXISTS : ", destinationFile.string()});
    }

    return (bSuccess);

}

//
// Email file action function.
//

bool emailFile(const std::string &filenamePathStr, const std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    CMailSMTP smtp;
    CMailIMAP imap;

    // Form source file path

    fs::path sourceFile(filenamePathStr);

    try {
        
        smtp.setServer(funcData->serverURLStr);
        smtp.setUserAndPassword(funcData->userNameStr, funcData->userPasswordStr);
        smtp.setFromAddress("<" + funcData->userNameStr + ">");
        smtp.setToAddress("<" + funcData->emailRecipientStr + ">");

        smtp.setMailSubject("FPE Attached File");
        smtp.addFileAttachment(filenamePathStr, CFileMIME::getFileMIMEType(filenamePathStr),  "base64");
        
        if(funcData->serverURLStr.find(std::string("smtp")) == 0) {
            
           smtp.postMail();
           funcData->coutstr({"Emailing file [", filenamePathStr, "] ", "to [", funcData->emailRecipientStr, "]"});
           bSuccess = true;
           
        } else if (funcData->serverURLStr.find(std::string("imap")) == 0) {
            
            std::string mailMessageStr;
            std::string commandLineStr;

            commandLineStr = "APPEND " + funcData->mailBoxNameStr + " (\\Seen) {";
            mailMessageStr = smtp.getMailMessage();
            commandLineStr += std::to_string(mailMessageStr.length() - 2) + "}" + mailMessageStr;

            imap.setServer(funcData->serverURLStr);
            imap.setUserAndPassword(funcData->userNameStr, funcData->userPasswordStr);
            
            imap.connect();

            std::string commandResponseStr(imap.sendCommand(commandLineStr));
            
            CMailIMAPParse::COMMANDRESPONSE commandResponse(CMailIMAPParse::parseResponse(commandLineStr));
            if (commandResponse->status == CMailIMAPParse::RespCode::BAD) {
                funcData->cerrstr({commandResponse->errorMessageStr});
            } else {
                funcData->coutstr({"Added file [", filenamePathStr, "] ", "to [" + funcData->mailBoxNameStr + "]"});
                bSuccess = true;
            }
            
            imap.disconnect();
            
       }

    } catch (const CMailSMTP::Exception &e) {
        funcData->cerrstr({e.what()});
    } catch (const CMailIMAP::Exception &e) {
        funcData->cerrstr({e.what()});
    } catch (const std::exception & e) {
        funcData->cerrstr({"Standard exception occured: [", e.what(), "]"});
    }

    return (bSuccess);

}

//
// Add file to ZIP archive.
//

bool zipFile(const std::string &filenamePathStr, const std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and zips file paths

    fs::path sourceFile(filenamePathStr);
    fs::path zipFilePath(funcData->zipArchiveStr);

    // Create path for ZIP archive if needed.

    if (!fs::exists(zipFilePath.parent_path())) {
        if (fs::create_directories(zipFilePath.parent_path())) {
            funcData->coutstr({"CREATED :", zipFilePath.parent_path().string()});
        } else {
            funcData->cerrstr({"CREATED FAILED FOR :", zipFilePath.parent_path().string()});
        }
    }
    
    // Create archive if doesn't exist
    
    CFileZIP zipFile(zipFilePath.string());
    
    if (!fs::exists(zipFilePath)) {
        funcData->coutstr({"CREATING ARCHIVE ", zipFilePath.string()});
        zipFile.create();
    }
    
    // Append file to archive
    
    zipFile.open();
    
    if (bSuccess = zipFile.append(sourceFile.string(), sourceFile.filename().string())) {
        funcData->coutstr({"APPENDED [", sourceFile.filename().string(), "] TO ARCHIVE [",zipFilePath.string(), "]" });
    }  

    zipFile.close();

    return (bSuccess);

}
