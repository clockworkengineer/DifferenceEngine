#include "FPE_HOST.hpp"
/*
 * File:   FPE_ActionFuncs.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * The MIT License
 *
 * Copyright 2016.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// STL definitions

#include <sstream>

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Task class

#include "FPE_Task.hpp" 

// Process wait definitions

#include <sys/wait.h>

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

//
// stdout trace output.
//

static void coutstr(ActFnData *funcData, const std::vector<std::string>& outstr) {

    assert(funcData != nullptr);

    if (funcData->coutstr != nullptr) {
        (funcData->coutstr)(outstr);
    }

}

//
// stderr trace output.
//

static void cerrstr(ActFnData *funcData, const std::vector<std::string>& errstr) {

    assert(funcData != nullptr);

    if (funcData->cerrstr != nullptr) {
        (funcData->cerrstr)(errstr);
    }

}

//
// Fork and execute Shell command
//

static int forkCommand(char *argv[]) {

    pid_t pid; // Process id
    int status; // wait status
    int exitStatus = 0; // child exit status

    if ((pid = fork()) < 0) { /* fork a child process           */

        std::stringstream errStream;
        errStream << " ERROR: forking child process failed: " << errno;
        throw std::runtime_error(errStream.str());

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
//

static int runShellCommand(std::string shellCommand) {

    int exitStatus = 0;
    int argc = 0;

    std::vector<char *> argvs;
    std::unique_ptr<char*> argv;
    std::unique_ptr<char> commandStr{ new char[shellCommand.length() + 1]};

    // Take a 'C' string copy

    commandStr.get()[shellCommand.length()] = 0;
    strncpy(commandStr.get(), shellCommand.c_str(), shellCommand.length());

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

//
// Run a specified command on the file (%1% source, %2% destination)
//

bool runCommand(const std::string &filenamePathStr, const std::string &filenameStr, std::shared_ptr<void>fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder + filenameStr);

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

    coutstr(funcData, {command});

    auto result = 0;
    if ((result = runShellCommand(command)) == 0) {
        bSuccess = true;
        coutstr(funcData, {"Command success." });
        if (funcData->bDeleteSource) {
            coutstr(funcData, {"DELETING SOURCE [", sourceFile.string(), "]"});
            fs::remove(sourceFile);
        }
    } else {
           cerrstr(funcData, {"Command error: ", std::to_string(result)});
    }

    return (bSuccess);

}

//
// Video file conversion action function. Convert passed in file to MP4 using Handbrake.
//

bool handBrake(const std::string &filenamePathStr, const std::string &filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder);

    destinationFile /= sourceFile.stem().string();

    if (funcData->extension.length() > 0) {
        destinationFile.replace_extension(funcData->extension);
    } else {
        destinationFile.replace_extension(".mp4");
    }

    // Convert file

    std::string command = (boost::format(funcData->commandToRun) % sourceFile.string() % destinationFile.string()).str();

    coutstr(funcData, {"Converting file [", sourceFile.string(), "] To [", destinationFile.string(), "]"});

    auto result = 0;
    if ((result = runShellCommand(command)) == 0) {
        bSuccess = true;
        coutstr(funcData,{"File conversion success."});
        if (funcData->bDeleteSource) {
            coutstr(funcData,{"DELETING SOURCE [", sourceFile.string(), "]"});
            fs::remove(sourceFile);
        }

    } else {
        coutstr(funcData, {"File conversion error: ", std::to_string(result)});
    }

    return (bSuccess);

}

//
// Copy file action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.
//

bool copyFile(const std::string &filenamePathStr, const std::string &filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr);

    // Destination file path += ("filename path" - "watch folder path")

    fs::path destinationFile(funcData->destinationFolder +
            filenamePathStr.substr((funcData->watchFolder).length()));

    // Construct full destination path if needed

    if (!fs::exists(destinationFile)) {
        if (fs::create_directories(destinationFile)) {
            coutstr(funcData,{"CREATED :", destinationFile.string()});
        } else {
            cerrstr(funcData,{"CREATED FAILED FOR :", destinationFile.string()});
        }
    }

    // Add filename to source and destination paths

    sourceFile /= filenameStr;
    destinationFile /= filenameStr;

    // Currently only copy file if it doesn't already exist.

    if (!fs::exists(destinationFile)) {
        coutstr(funcData,{"COPY FROM [", sourceFile.string(), "] TO [", destinationFile.string(), "]"});
        fs::copy_file(sourceFile, destinationFile, fs::copy_option::none);
        bSuccess = true;
        if (funcData->bDeleteSource) {
            coutstr(funcData,{"DELETING SOURCE [", sourceFile.string(), "]"});
            fs::remove(sourceFile);
        }

    } else {
        coutstr(funcData,{"DESTINATION ALREADY EXISTS : ", destinationFile.string()});
    }

    return (bSuccess);

}
