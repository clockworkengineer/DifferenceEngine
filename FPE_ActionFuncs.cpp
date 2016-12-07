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

// Task class

#include "FPE_Task.hpp" 

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Process wait definitions

#include <sys/wait.h>

//
// Fork and execute Shell command
//

int forkCommand(char *argv[]) {
    
    pid_t   pid;
    int     status;
    int     exitStatus=0;

    if ((pid = fork()) < 0) { /* fork a child process           */
        std::stringstream errStream;
        errStream << " ERROR: forking child process failed: " << errno;
        throw std::runtime_error(errStream.str());
    } else if (pid == 0) {                  /* for the child process: */
        if (execvp(*argv, argv) < 0) {      /* execute the command  */
            std::cerr << "*** ERROR: exec() failed ***" << std::endl;
            exit(1);
        }
    } else { /* for the parent:      */
        while (wait(&status) != pid) { /* wait for completion  */
            continue;
        }
        if (WIFEXITED(status)) {  // Set any exit status
                exitStatus = WEXITSTATUS(status);
        }
    }
    return (exitStatus);
}

//
// Run shell command. Split command into argv components before passing onto exec.
// NOTE: This function calls new and delete [] for the command and argv memory but
// we are dealing with low level system calls so keep to a minimum.
//

int runShellCommand(std::string shellCommand) {

    int exitStatus=0;
    int argc = 0;
    std::vector<char *> argvs;
    char **argv;
    char *commandStr;

    // Take a 'C' string copy
    
    commandStr = new char[shellCommand.length() + 1];
    commandStr[shellCommand.length()] = 0;
    strncpy(commandStr, shellCommand.c_str(), shellCommand.length());

    // Loop through command splitting into substrings for argv
    
    char *p2 = strtok(commandStr, " ");

    while (p2) {
        argvs.push_back(p2);
        p2 = strtok(0, " ");
    }

    // Allocate argv array and copy vector over
    
    argv = new char*[argvs.size() + 1];

    for (char *arg : argvs) {
        argv[argc++] = arg;
    }

    argv[argc] = 0; // Last element null
    
    // Fork command

    exitStatus = forkCommand(argv);

    // Clean up.
    
    if (argv) {
        delete [] argv;
    }
    if (commandStr) {
        delete [] commandStr;
    }
    
    return (exitStatus);

}

//
// Run a specified command on the file (%1% source, %2% destination)
//

bool runCommand(std::string filenamePathStr, std::string filenameStr, std::shared_ptr<void>fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder.string() + filenameStr);

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

    std::cout << command << std::endl;

    auto result = 0;
    if ((result = runShellCommand(command)) == 0) {
        bSuccess = true;
        std::cout << "Command success." << std::endl;
        if (funcData->bDeleteSource) {
            std::cout << "DELETING SOURCE [" << sourceFile << "]" << std::endl;
            fs::remove(sourceFile);
        }
    } else {
        std::cout << "Command error: " << result << std::endl;
    }

    return (bSuccess);

}

//
// Video file conversion action function. Convert passed in file to MP4 using Handbrake.
//

bool handBrake(std::string filenamePathStr, std::string filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder.string());

    destinationFile /= sourceFile.stem().string();
    destinationFile.replace_extension(".mp4");

    // Convert file

    std::string command = (boost::format(funcData->commandToRun) % sourceFile.string() % destinationFile.string()).str();

    std::cout << command << std::endl;

    auto result = 0;
    if ((result = runShellCommand(command)) == 0) {
        bSuccess = true;
        std::cout << "File conversion success." << std::endl;
        if (funcData->bDeleteSource) {
            std::cout << "DELETING SOURCE [" << sourceFile << "]" << std::endl;
            fs::remove(sourceFile);
        }

    } else {
        std::cout << "File conversion error: " << result << std::endl;
    }

    return (bSuccess);

}

//
// Copy file action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.
//

bool copyFile(std::string filenamePathStr, std::string filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool bSuccess = false;

    // Destination file path += ("filename path" - "watch folder path")

    std::string destinationPathStr(funcData->destinationFolder.string() +
            filenamePathStr.substr((funcData->watchFolder.string()).length()));

    // Construct full destination path if needed

    if (!fs::exists(destinationPathStr)) {
        if (fs::create_directories(destinationPathStr)) {
            std::cout << "CREATED :" + destinationPathStr << std::endl;
        } else {
            std::cerr << "CREATED FAILED FOR :" + destinationPathStr << std::endl;
        }
    }

    // Add filename to source and destination paths

    filenamePathStr += filenameStr;
    destinationPathStr += filenameStr;

    // Currently only copy file if it doesn't already exist.

    if (!fs::exists(destinationPathStr)) {
        std::cout << "COPY FROM [" << filenamePathStr << "] TO [" << destinationPathStr << "]" << std::endl;
        fs::copy_file(filenamePathStr, destinationPathStr, fs::copy_option::none);
        bSuccess = true;
        if (funcData->bDeleteSource) {
            std::cout << "DELETING SOURCE [" << filenamePathStr << "]" << std::endl;
            fs::remove(filenamePathStr);
        }

    } else {
        std::cout << "DESTINATION ALREADY EXISTS : " + destinationPathStr << std::endl;
    }

    return (bSuccess);

}
