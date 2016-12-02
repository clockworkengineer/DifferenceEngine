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
//

void runCommand(std::string filenamePathStr, std::string filenameStr, std::shared_ptr<void>fnData) {

    // ASSERT if passed in pointers are NULL

    assert(fnData != nullptr);
    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    assert(funcData != nullptr);

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder.string());

    try {

        // Create destination file name

        destinationFile /= filenameStr;

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
            std::cout << "Command success." << std::endl;
            if (funcData->bDeleteSource) {
                std::cout << "DELETING SOURCE [" << sourceFile << "]" << std::endl;
                fs::remove(sourceFile);
            }
        } else {
            std::cout << "Command error: " << result << std::endl;
        }

        //
        // Catch any errors locally and report so that thread keeps running.
        //   

    } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
    }

}

//
// Video file conversion action function. Convert passed in file to MP4 using Handbrake.
//

void handBrake(std::string filenamePathStr, std::string filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT if passed in pointers are NULL

    assert(fnData != nullptr);
    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    assert(funcData != nullptr);

    fs::path sourceFile(filenamePathStr + filenameStr);
    fs::path destinationFile(funcData->destinationFolder.string());

    try {

        // Create destination file name

        destinationFile /= sourceFile.stem().string();
        destinationFile.replace_extension(".mp4");

        // Convert file

        std::string command = (boost::format(funcData->commandToRun) % sourceFile.string() % destinationFile.string()).str();

        std::cout << command << std::endl;

        auto result = 0;
        if ((result = std::system(command.c_str())) == 0) {
            std::cout << "File conversion success." << std::endl;
            if (funcData->bDeleteSource) {
                std::cout << "DELETING SOURCE [" << sourceFile << "]" << std::endl;
                fs::remove(sourceFile);
            }

        } else {
            std::cout << "File conversion error: " << result << std::endl;
        }

        //
        // Catch any errors locally and report so that thread keeps running.
        //   

    } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
    }

}

//
// Copy file action function. Copy passed file to destination folder/directory 
// keeping the sources directory structure.
//

void copyFile(std::string filenamePathStr, std::string filenameStr, std::shared_ptr<void> fnData) {

    // ASSERT if passed in pointers are NULL

    assert(fnData != nullptr);
    ActFnData *funcData = static_cast<ActFnData *> (fnData.get());
    assert(funcData != nullptr);

    // Destination file path += ("filename path" - "watch folder path")

    std::string destinationPathStr(funcData->destinationFolder.string() +
            filenamePathStr.substr((funcData->watchFolder.string()).length()));


    try {

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
            if (funcData->bDeleteSource) {
                std::cout << "DELETING SOURCE [" << filenamePathStr << "]" << std::endl;
                fs::remove(filenamePathStr);
            }

        } else {
            std::cout << "DESTINATION ALREADY EXISTS : " + destinationPathStr << std::endl;
        }

        //
        // Catch any errors locally and report so that thread keeps running.
        // 

    } catch (const fs::filesystem_error & e) {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::exception & e) {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown exception occured" << std::endl;
    }

}
