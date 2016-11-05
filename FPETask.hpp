/*
 * The MIT License
 *
 * Copyright 2016 Robert Tizzard.
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

/*
 * File:   FPETask.hpp
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:33 PM
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include <exception>
#include <unordered_map>
#include <thread>
#include <fstream>

#include <boost/filesystem.hpp>

#define INOTIFY_THREAD_SAFE // Compile so thread safe.
#include "inotify-cxx.h"

#ifndef FPETASK_HPP
#define FPETASK_HPP

namespace fs = boost::filesystem;

class FPETask {
public:

    // CONSTRUCTOR

    FPETask(std::string taskNameStr,                       // Task name
            std::string watchFolder,                       // Watch folder path (absolute path for present)
            void (*taskFcn)(std::string filenamePathStr,   // Task file process function
            std::string filenameStr));

    // DESTRUCTOR

    virtual ~FPETask(); // Task class cleanup

    // One public function to monitor watch folder for file events and process added files

    void monitor(void);

private:

    FPETask(); // Use only provided constructor
    FPETask(const FPETask & orig);

    std::string prefix(void);               // Logging output prefix function
    void addWatch(InotifyEvent event);      // Add a folder to watch
    void removeWatch(InotifyEvent event);   // Remove a folder watch
    void createWatchTable(void);            // Create a watch table for existing watch folder structure
    void destroyWatchTable(void);           // Clear watch table

    std::string  taskName;         // Task name
    std::string  watchFolder;      // Watch Folder 
    Inotify * notify;              // File watch notifier

    std::unordered_map<InotifyWatch *, std::string> watchMap;       // Watch table indexed by watch variable
    std::unordered_map<std::string, InotifyWatch *> revWatchMap;    // Reverse watch table indexed by path

    void (*taskProcessFcn)(std::string filenamePathStr, // Task file process function 
                           std::string filenameStr);

    static const uint32_t InofityEvents; // Inotify events to monitor

};
#endif /* FPETASK_HPP */

