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
// Module: FPE_ActionFuncs
//
// Description: The task action functions used by the FPE and their support functions.
// 
// Currently five actions are provided.
//
// 1) File copy
// 2) Video file conversion (using handbrake)
// 3) Run shell 
// 4) Email file as attachment (if the server is IMAP then mail message is appended to a mailbox)
// 5) Append file to a ZIP archive
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CTask, CSMTP, CIMAP, CIMAPParse, CZIP, CMIME
// Linux              : Target platform
// Boost              : File system.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <system_error>

//
// Program components.
//

#include "FPE_ActionFuncs.hpp"

//
// Antikythera Classes
//

#include "CZIP.hpp"
#include "CTask.hpp"  
#include "CSMTP.hpp"
#include "CIMAP.hpp"
#include "CIMAPParse.hpp"
#include "CMIME.hpp"

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

namespace FPE_ActionFuncs {

    // =======
    // IMPORTS
    // =======
    
    using namespace std;

    using namespace Antik::IMAP;
    using namespace Antik::File;
    using namespace Antik::SMTP;
    using namespace Antik::Util;
    using namespace Antik::ZIP;

    // ===============
    // LOCAL VARIABLES
    // ===============

    //
    // Task Action Functions
    //
    
    static const vector<TaskActionFunc> taskList {
        { 0, kTaskCopyFileStr, copyFile},
        { 1, kVideoConversionStr, videoConversion},
        { 2, kEmailFileStr, emailFile},
        { 3, kZipFileStr, zipFile},
        { 4, kRunCommandStr, runCommand}
    };
   
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

            throw system_error(error_code(errno, system_category()), "ERROR: forking child process failed:");

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

    static int runShellCommand(const string& shellCommandStr) {

        int exitStatus = 0;
        int argc = 0;

        vector<char *> argvs;
        unique_ptr<char*> argv;
        unique_ptr<char> commandStr{ new char[shellCommandStr.length() + 1]};

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
    // Action function initialization / closedown.
    //
    
    bool actionFuncInit() {

        CSMTP::init();
        CIMAP::init();
    }

    bool actionFuncCloseDown() {

        CSMTP::closedown();
        CIMAP::closedown();

    }
    
    //
    //  Get task details from taskList table
    //
    
    TaskActionFunc getTaskDetails (int taskNumber) {
        if((taskNumber >= 0) && (taskNumber < taskList.size())) {
            return (taskList[taskNumber]);
        }
        return(TaskActionFunc());
    }
    
    //
    // Run a specified command on the file (%1% source, %2% destination)
    //

    bool runCommand(const string &filenamePathStr, const shared_ptr<void>fnData) {

        // ASSERT for any invalid parameters.

        assert(fnData != nullptr);
        assert(filenamePathStr.length() != 0);

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(filenamePathStr);
        fs::path destinationFile(funcData->params["destination"] + sourceFile.filename().string());

        // Create correct command for whether source and destination specified or just source or none

        bool srcFound = (funcData->params["command"].find("%1%") != string::npos);
        bool dstFound = (funcData->params["command"].find("%2%") != string::npos);

        string commandStr;
        if (srcFound && dstFound) {
            commandStr = (boost::format(funcData->params["command"]) % sourceFile.string() % destinationFile.string()).str();
        } else if (srcFound) {
            commandStr = (boost::format(funcData->params["command"]) % sourceFile.string()).str();
        } else {
            commandStr = funcData->params["command"];
        }

        funcData->coutstr({commandStr});

        auto result = 0;
        if ((result = runShellCommand(commandStr)) == 0) {
            bSuccess = true;
            funcData->coutstr({"Command success."});
            if (funcData->bDeleteSource) {
                funcData->coutstr({"DELETING SOURCE [", sourceFile.string(), "]"});
                fs::remove(sourceFile);
            }
        } else {
            funcData->cerrstr({"Command error: ", to_string(result)});
        }

        return (bSuccess);

    }

    //
    // Video file conversion action function. Convert passed in file to MP4 using Handbrake.
    //

    bool videoConversion(const string& filenamePathStr, const shared_ptr<void> fnData) {

        // ASSERT for any invalid parameters.

        assert(fnData != nullptr);
        assert(filenamePathStr.length() != 0);

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(filenamePathStr);
        fs::path destinationFile(funcData->params["destination"]);

        destinationFile /= sourceFile.stem().string();

        if (funcData->params["extension"].length() > 0) {
            destinationFile.replace_extension(funcData->params["extension"]);
        } else {
            destinationFile.replace_extension(".mp4");
        }

        // Convert file

        string commandStr = (boost::format(funcData->params["command"]) % sourceFile.string() % destinationFile.string()).str();

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
            funcData->coutstr({"File conversion error: ", to_string(result)});
        }

        return (bSuccess);

    }

    //
    // Copy file action function. Copy passed file to destination folder/directory 
    // keeping the sources directory structure.
    //

    bool copyFile(const string &filenamePathStr, const shared_ptr<void> fnData) {

        // ASSERT for any invalid parameters.

        assert(fnData != nullptr);
        assert(filenamePathStr.length() != 0);

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(filenamePathStr);

        // Destination file path += ("filename path" - "watch folder path")

        fs::path destinationFile(funcData->params["destination"] +
                filenamePathStr.substr((funcData->params["watch"]).length()));

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

    bool emailFile(const string &filenamePathStr, const shared_ptr<void> fnData) {

        // ASSERT for any invalid parameters.

        assert(fnData != nullptr);
        assert(filenamePathStr.length() != 0);

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
        bool bSuccess = false;

        CSMTP smtp;

        // Form source file path

        fs::path sourceFile(filenamePathStr);

        try {

            smtp.setServer(funcData->params["server"]);
            smtp.setUserAndPassword(funcData->params["user"], funcData->params["password"]);
            smtp.setFromAddress("<" + funcData->params["user"] + ">");
            smtp.setToAddress("<" + funcData->params["recipient"] + ">");

            smtp.setMailSubject("FPE Attached File");
            smtp.addFileAttachment(filenamePathStr, CMIME::getFileMIMEType(filenamePathStr), "base64");

            if (funcData->params["server"].find(string("smtp")) == 0) {

                smtp.postMail();
                funcData->coutstr({"Emailing file [", filenamePathStr, "] ", "to [", funcData->params["recipient"], "]"});
                bSuccess = true;

            } else if (funcData->params["server"].find(string("imap")) == 0) {

                CIMAP imap;
                string mailMessageStr;
                string commandLineStr;

                commandLineStr = "APPEND " + funcData->params["mailbox"] + " (\\Seen) {";
                mailMessageStr = smtp.getMailMessage();
                commandLineStr += to_string(mailMessageStr.length() - 2) + "}" + mailMessageStr;

                imap.setServer(funcData->params["server"]);
                imap.setUserAndPassword(funcData->params["user"], funcData->params["password"]);

                imap.connect();

                string commandResponseStr(imap.sendCommand(commandLineStr));

                CIMAPParse::COMMANDRESPONSE commandResponse(CIMAPParse::parseResponse(commandLineStr));
                if (commandResponse->status == CIMAPParse::RespCode::BAD) {
                    funcData->cerrstr({commandResponse->errorMessageStr});
                } else {
                    funcData->coutstr({"Added file [", filenamePathStr, "] ", "to [" + funcData->params["mailbox"] + "]"});
                    bSuccess = true;
                }

                imap.disconnect();

            }

        } catch (const CSMTP::Exception &e) {
            funcData->cerrstr({e.what()});
        } catch (const CIMAP::Exception &e) {
            funcData->cerrstr({e.what()});
        } catch (const exception & e) {
            funcData->cerrstr({"Standard exception occured: [", e.what(), "]"});
        }

        return (bSuccess);

    }

    //
    // Add file to ZIP archive.
    //

    bool zipFile(const string &filenamePathStr, const shared_ptr<void> fnData) {

        // ASSERT for any invalid parameters.

        assert(fnData != nullptr);
        assert(filenamePathStr.length() != 0);

        ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
        bool bSuccess = false;

        // Form source and zips file paths

        fs::path sourceFile(filenamePathStr);
        fs::path zipFilePath(funcData->params["archive"]);

        // Create path for ZIP archive if needed.

        if (!fs::exists(zipFilePath.parent_path())) {
            if (fs::create_directories(zipFilePath.parent_path())) {
                funcData->coutstr({"CREATED :", zipFilePath.parent_path().string()});
            } else {
                funcData->cerrstr({"CREATED FAILED FOR :", zipFilePath.parent_path().string()});
            }
        }

        // Create archive if doesn't exist

        CZIP zipFile(zipFilePath.string());

        if (!fs::exists(zipFilePath)) {
            funcData->coutstr({"CREATING ARCHIVE ", zipFilePath.string()});
            zipFile.create();
        }

        // Append file to archive

        zipFile.open();

        if (bSuccess = zipFile.add(sourceFile.string(), sourceFile.filename().string())) {
            funcData->coutstr({"APPENDED [", sourceFile.filename().string(), "] TO ARCHIVE [", zipFilePath.string(), "]"});
        }

        zipFile.close();

        return (bSuccess);

    }

} // namespace FPE_ActionFuncs