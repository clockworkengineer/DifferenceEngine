/*
 * File:   CFileTask.hpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 *
 * Copyright 2016.
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
// CFileApprise file event watcher
//

#include "CFileApprise.hpp"

// ================
// CLASS DEFINITION
// ================

class CFileTask { 
    
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
    // Task options structure (optionally pass to CFileTask constructor)
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

    CFileTask(const std::string& taskName,                      // Task name
            const std::string& watchFolder,                     // Watch folder path
            TaskActionFcn taskActFcn,                           // Task action function
            std::shared_ptr<void> fnData,                       // Task file process function data
            int watchDepth,                                     // Watch depth -1= all, 0=just watch folder
            std::shared_ptr<TaskOptions> options=nullptr);      // Task options. 
    
    // ==========
    // DESTRUCTOR
    // ==========
    
    virtual ~CFileTask(); // Task class cleanup

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
    
    CFileTask() = delete;                            // Use only provided constructors
    CFileTask(const CFileTask & orig) = delete;
    CFileTask(const CFileTask && orig )= delete;   
    
    // ========================
    // PRIVATE MEMBER FUNCTIONS
    // ========================
    
    // =================
    // PRIVATE VARIABLES
    // =================
  
    // Constructor passed in and intialized
    
    std::string  taskName;                          // Task name
    std::string  watchFolder;                       // Watch Folder
    TaskActionFcn taskActFcn;                       // Task action function 
    std::shared_ptr<void> fnData;                   // Task action function data   
    int killCount=0;                                // Task Kill Count

    // CFileApprise file watcher
    
    std::shared_ptr<CFileApprise> watcher;              // Folder watcher
    std::shared_ptr<CFileApprise::Options> watcherOptions;// folder watcher options
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

