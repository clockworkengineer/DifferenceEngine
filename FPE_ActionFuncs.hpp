/*
 * File:   FPE_ActionFuncs.hpp
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

#ifndef FPE_ACTIONFUNCS_HPP
#define FPE_ACTIONFUNCS_HPP

#include <string>
#include <memory>
#include <vector>

// Task action function

typedef bool (*TaskActionFcn)(const std::string &filenamePathStr,  
                              const std::string &filenameStr, 
                              std::shared_ptr<void>fnData);

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
//  TaskActionFcn sampleLambda = [] (auto filenamePathStr, auto filenameStr, auto fnData) 
//  -> bool { std::cout << "[" << filenamePathStr+filenameStr << "]" << std::endl; return true; };

// Action function prototypes

bool handBrake(const std::string &filenamePathStr, const std::string &filenameStr, std::shared_ptr<void> fnData);
bool copyFile(const std::string &filenamePathStr, const std::string &filenameStr, std::shared_ptr<void> fnData);
bool runCommand(const std::string &filenamePathStr, const std::string &filenameStr, std::shared_ptr<void> fnData);

#endif /* FPE_ACTIONFUNCS_HPP */

