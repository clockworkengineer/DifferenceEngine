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

//
// Run a specified command on the file (%1% source, %2% destination)
// At present this uses system ( but this doesn't return and error and isn't safe
// so change in future.
//

bool runCommand(std::string &filenamePathStr, std::string &filenameStr, std::shared_ptr<void>fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool      bSuccess=false;

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
    if ((result = std::system(command.c_str())) == 0) {
        bSuccess=true;
        std::cout << "Command success." << std::endl;
        if (funcData->bDeleteSource) {
            std::cout << "DELETING SOURCE [" << sourceFile << "]" << std::endl;
            fs::remove(sourceFile);
        }
    } else {
        std::cout << "Command error: " << result << std::endl;
    }
    
    return(bSuccess);

}

//
// Video file conversion action function. Convert passed in file to MP4 using Handbrake.
// At present this uses system ( but this doesn't return and error and isn't safe
// so change in future.
//

bool handBrake(std::string &filenamePathStr, std::string &filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool      bSuccess=false;

    // Form source and destination file paths

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder.string());

    destinationFile /= sourceFile.stem().string();
    destinationFile.replace_extension(".mp4");

    // Convert file

    std::string command = (boost::format(funcData->commandToRun) % sourceFile.string() % destinationFile.string()).str();

    std::cout << command << std::endl;

    auto result = 0;
    if ((result = std::system(command.c_str())) == 0) {
        bSuccess = true;
        std::cout << "File conversion success." << std::endl;
        if (funcData->bDeleteSource) {
            std::cout << "DELETING SOURCE [" << sourceFile << "]" << std::endl;
            fs::remove(sourceFile);
        }

    } else {
        std::cout << "File conversion error: " << result << std::endl;
    }

    return(bSuccess);

}

//
// Copy file action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.
//

bool copyFile(std::string &filenamePathStr, std::string &filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT for any invalid parameters.

    assert(fnData != nullptr);
    assert(filenamePathStr.length() != 0);
    assert(filenameStr.length() != 0);

    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    bool      bSuccess=false;

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
        bSuccess=true;
        if (funcData->bDeleteSource) {
            std::cout << "DELETING SOURCE [" << filenamePathStr << "]" << std::endl;
            fs::remove(filenamePathStr);
        }

    } else {
        std::cout << "DESTINATION ALREADY EXISTS : " + destinationPathStr << std::endl;
    }

    return(bSuccess);

}
