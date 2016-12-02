/*
 * File:   FPE_ProcCmdLine.hpp
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

#ifndef FPE_PROCCMDLINE_HPP
#define FPE_PROCCMDLINE_HPP

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

// Command line parameter data

struct ParamArgData {
    fs::path     watchFolder;        // Watch Folder
    fs::path     destinationFolder;  // Destination Folder for copies.
    std::string  commandToRun;       // Command to run
    bool         bFileCopy;          // Task file copy
    bool         bVideoConversion;   // Task video conversion
    bool         bRunCommand;        // Task perform command
    int          maxWatchDepth;      // Watch depth -1=all,0=just watch folder,1=next level down etc.
    bool         bDeleteSource;      // Delete source file
};

// Command line exit status

namespace { 
    const size_t ERROR_IN_COMMAND_LINE = 1;
    const size_t SUCCESS = 0;
    const size_t ERROR_UNHANDLED_EXCEPTION = 2;
}

void procCmdLine (int argc, char** argv, ParamArgData &argData);

#endif /* FPE_PROCCMDLINE_HPP */

