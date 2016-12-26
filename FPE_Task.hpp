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

#include <thread>

// IApprise file event watcher

#include "IApprise.hpp"

// Task options structure (OPTIONALLY PASSED TO TASK)

struct TaskOptions {
    int killCount;      // After killCount files processed stop task (0 = disabled)
    void (*coutstr) (const std::vector<std::string>& outstr);  // coutstr output
    void (*cerrstr) (const std::vector<std::string>& errstr);  // cerrstr output
};

// Task class

class FPE_Task {
public:

    // CONSTRUCTOR

    FPE_Task(std::string taskName,                                  // Task name
            std::string watchFolder,                                // Watch folder path
            TaskActionFcn taskActFcn,                               // Task action function
            std::shared_ptr<void> fnData,                           // Task file process function data
            int maxWatchDepth,                                      // Maximum watch depth -1= all, 0=just watch folder
            std::shared_ptr<TaskOptions> taskOptions=nullptr);      //  Task options. 
    
    // DESTRUCTOR

    virtual ~FPE_Task(); // Task class cleanup

    // PUBLIC FUNCTIONS

    void monitor(void);     // Monitor watch folder for file events and process added files
    void stop(void);        // Stop task
 
private:

    FPE_Task() = delete;                            // Use only provided constructors
    FPE_Task(const FPE_Task & orig) = delete;
    FPE_Task(const FPE_Task && orig )= delete;   

    std::shared_ptr<IApprise> watcher;              // Folder watcher
    std::shared_ptr<IAppriseOptions> watchOpt;      // folder watcher options
    std::unique_ptr<std::thread> watcherThread;     // Folder watcher thread
    
    std::string prefix(void);                       // Logging output prefix  
    
    void coutstr(const std::vector<std::string>& outstr);   // std::cout
    void cerrstr(const std::vector<std::string>& outstr);   // std::cerr
    
    // CONSTRUCTOR PARAMETERS
    
    std::string  taskName;                                  // Task name
    std::string  watchFolder;                               // Watch Folder
    TaskActionFcn taskActFcn;                               // Task action function 
    std::shared_ptr<void> fnData;                           // Task action function data
    std::shared_ptr<TaskOptions> taskOptions;               // Task passed options

};
#endif /* FPETASK_HPP */

