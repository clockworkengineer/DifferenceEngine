#include "HOST.hpp"
/*
 * File:   FPE_ActionFuncs.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 * 
 * Description: The task action functions used by the FPE and their support functions.
 * 
 * Currently three actions are provided.
 *
 * 1) File copy
 * 2) Video file conversion (using handbrake)
 * 3) Run shell command
 * 
 * Dependencies: C11++, classes (CFileTask, CLogger), Linux, Boost C++ Libraries.
 *
 * Copyright 2016.
 *
 */

// =============
// INCLUDE FILES
// =============

// C++ STL definitions

#include <system_error>

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Task class

#include "CFileTask.hpp" 

// Process wait definitions

#include <sys/wait.h>

// Boost file system and format libraries definitions

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

static int runShellCommand(const std::string& shellCommand) {

    int exitStatus = 0;
    int argc = 0;

    std::vector<char *> argvs;
    std::unique_ptr<char*> argv;
    std::unique_ptr<char> commandStr{ new char[shellCommand.length() + 1]};

    // Take a 'C' string copy

     shellCommand.copy(commandStr.get(), shellCommand.length());
     commandStr.get()[shellCommand.length()] = 0; // null terminate

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

bool runCommand(const std::string &filenamePath, const std::shared_ptr<void>fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePath.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePath);
    fs::path destinationFile(funcData->destinationFolder + sourceFile.filename().string());

    // Create correct command for whether source and destination specified or just source or none

    bool srcFound = (funcData->commandToRun.find("%1%") != std::string::npos);
    bool dstFound = (funcData->commandToRun.find("%2%") != std::string::npos);

    std::string command;
    if (srcFound && dstFound) {
        command = (boost::format(funcData->commandToRun) % sourceFile.string() % destinationFile.string()).str();
    } else if (srcFound) {
        command = (boost::format(funcData->commandToRun) % sourceFile.string()).str();
    } else {
        command = funcData->commandToRun;
    }

    funcData->coutstr({command});

    auto result = 0;
    if ((result = runShellCommand(command)) == 0) {
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

bool handBrake(const std::string& filenamePath, const std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePath.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePath);
    fs::path destinationFile(funcData->destinationFolder);

    destinationFile /= sourceFile.stem().string();

    if (funcData->extension.length() > 0) {
        destinationFile.replace_extension(funcData->extension);
    } else {
        destinationFile.replace_extension(".mp4");
    }

    // Convert file

    std::string command = (boost::format(funcData->commandToRun) % sourceFile.string() % destinationFile.string()).str();

    funcData->coutstr({"Converting file [", sourceFile.string(), "] To [", destinationFile.string(), "]"});

    auto result = 0;
    if ((result = runShellCommand(command)) == 0) {
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

bool copyFile(const std::string &filenamePath, const std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePath.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePath);

    // Destination file path += ("filename path" - "watch folder path")

    fs::path destinationFile(funcData->destinationFolder +
            filenamePath.substr((funcData->watchFolder).length()));

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
