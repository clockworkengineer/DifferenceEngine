/*
 * File:  FPE_Task.hpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
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

#ifndef FPETASK_HPP
#define FPETASK_HPP

// STL definitions

#include <string>
#include <iostream>
#include <cstdlib>
#include <exception>
#include <unordered_map>
#include <thread>
#include <fstream>
#include <chrono>
#include <queue>
#include <mutex>
#include <atomic>
#include <utility>
#include <sstream> 

// inotify definitions

#include <sys/inotify.h>

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

class FPE_Task {
public:

    // CONSTRUCTOR

    FPE_Task(std::string taskNameStr,                       // Task name
            std::string watchFolder,                       // Watch folder path
            int maxWatchDepth,                             // Maximum watch depth -1= all, 0=just watch folder
            void (*taskFcn)(std::string filenamePathStr,   // Task file process function
                            std::string filenameStr, 
                            std::shared_ptr<void>fnData), 
            std::shared_ptr<void> fnData);                  // Task file process function data
            


        
    // DESTRUCTOR

    virtual ~FPE_Task(); // Task class cleanup

    // PUBLIC FUNCTIONS

    void monitor(void);     // Monitor watch folder for file events and process added files
    void stop(void);        // Stop all threads
 
private:

    FPE_Task() = delete;                                      // Use only provided constructors
    FPE_Task(const FPE_Task & orig) = delete;;
    FPE_Task(const FPE_Task && orig )= delete;;   
    
    static int pathDepth(std::string pathStr);      // Add path to be watched
    std::string prefix(void);                       // Logging output prefix function
    void addWatchPath(std::string pathStr);         // Add path to be watched
    void addWatch(struct inotify_event *event);     // Add a folder to watch
    void removeWatch(struct inotify_event *event);  // Remove a folder watch
    void createWatchTable(void);                    // Create a watch table for existing watch folder structure
    void destroyWatchTable(void);                   // Clear watch table
    void worker(void);                              // Worker thread

    std::string  taskName;                                  // Task name
    std::string  watchFolder;                               // Watch Folder
    int maxWatchDepth;                                      // Watch depth -1=all,0=just watch folder,1=next level down etc.
    int fdNotify;                                           // inotify file descriptor
    std::mutex fileNamesMutex;                              // Queue Mutex
    std::queue <std::string> fileNames;                     // Queue of path/file names
    std::atomic<bool> bDoWork;                               // doWork=true (run thread loops) false=(stop thread loops)
    std::unique_ptr<std::thread> workerThread;              // Worker thread for task to be performed.
    std::unordered_map<int32_t, std::string> watchMap;      // Watch table indexed by watch variable
    std::unordered_map<std::string, int32_t> revWatchMap;   // Reverse watch table indexed by path
    void (*taskProcessFcn)(std::string filenamePathStr,     // Task file process function 
                           std::string filenameStr,
                           std::shared_ptr<void> fnData);
    
    std::shared_ptr<void> fnData;                           // Task action function data

    static const uint32_t kInofityEvents;       // inotify events to monitor
    static const uint32_t kInotifyEventSize;    // inotify read event size
    static const uint32_t kInotifyEventBuffLen; // inotify read buffer length

};
#endif /* FPETASK_HPP */

