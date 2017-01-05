/*
 * File:   FPE_Task.hpp
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

/*
 * Class:       FPE_Task
 * 
 * Description: This class uses the CFileApprise class to generate file add events
 * on a watch folder and to process each file added with a task action function 
 * provided as a parameter in its constructor.
 * 
 * Dependencies: C11++, Classes (CFileApprise).
 * 
 */

#ifndef FPETASK_HPP
#define FPETASK_HPP

//
// C++ STL definitions
//

#include <thread>
#include <cassert>

//
// IApprise file event watcher
//

#include "IApprise.hpp"

//
// ================
// CLASS DEFINITION
// ================
//

class FPE_Task {
    
public:

    // ==========================
    // PUBLIC TYPES AND CONSTANTS
    // ==========================

    //
    // Task action function
    //
    
    typedef bool (*TaskActionFcn)(const std::string& filenamePath,
            const std::shared_ptr<void>fnData);

    //
    // Task options structure (optionally pass to FPE_Task constructor)
    // Note:After killCount files processed stop task (0 = disabled)
    //
    
    struct TaskOptions {
        int killCount;                                            // file kill count
        void (*coutstr) (const std::vector<std::string>& outstr); // coutstr output
        void (*cerrstr) (const std::vector<std::string>& errstr); // cerrstr output
    };
    
    // ===========
    // CONSTRUCTOR
    // ===========

    FPE_Task(const std::string& taskName,                       // Task name
            const std::string& watchFolder,                     // Watch folder path
            TaskActionFcn taskActFcn,                           // Task action function
            std::shared_ptr<void> fnData,                       // Task file process function data
            int watchDepth,                                     // Watch depth -1= all, 0=just watch folder
            std::shared_ptr<TaskOptions> options=nullptr);      // Task options. 
    
    // ==========
    // DESTRUCTOR
    // ==========
    
    virtual ~FPE_Task(); // Task class cleanup

    // =======================
    // PUBLIC MEMBER FUNCTIONS
    // =======================

    // Control
    
    void monitor(void);                         // Monitor watch folder for file events and process added files
    void stop(void);                            // Stop task

   // Private data accessors
    
    std::exception_ptr getThrownException(void);// Get any exception thrown by task to pass down chain
 
private:
    
    // ===========================
    // PRIVATE TYPES AND CONSTANTS
    // ===========================

    // =====================
    // DISABLED CONSTRUCTORS
    // =====================
    
    FPE_Task() = delete;                            // Use only provided constructors
    FPE_Task(const FPE_Task & orig) = delete;
    FPE_Task(const FPE_Task && orig )= delete;   
    
    // ========================
    // PRIVATE MEMBER FUNCTIONS
    // ========================
    
    // =================
    // PRIVATE VARIABLES
    // =================
  
    // Constructor passed in and intialised
    
    std::string  taskName;                          // Task name
    std::string  watchFolder;                       // Watch Folder
    TaskActionFcn taskActFcn;                       // Task action function 
    std::shared_ptr<void> fnData;                   // Task action function data   
    int killCount=0;                                // Task Kill Count

    // CFileApprise file watcher
    
    std::shared_ptr<IApprise> watcher;              // Folder watcher
    std::shared_ptr<IAppriseOptions> watcherOptions;// folder watcher options
    std::unique_ptr<std::thread> watcherThread;     // Folder watcher thread

    // Publicly accessed via accessors
    
    std::exception_ptr thrownException=nullptr;     // Pointer to any exception thrown
    

    
    // Trace functions default do nothing (Im sure a batter solution exists but fix later).

    std::string prefix; // Task trace prefix

    void (*coutstr) (const std::vector<std::string>& outstr) = [] (const std::vector<std::string>& outstr) {
    };
    void (*cerrstr) (const std::vector<std::string>& errstr) = [] (const std::vector<std::string>& errstr) {
    }; 

};
#endif /* FPETASK_HPP */

