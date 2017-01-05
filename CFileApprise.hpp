#include "HOST.hpp"
/*
 * File:   CFileApprise.hpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 *
 * Copyright 2016.
 
 */

#ifndef IAPPRISE_HPP
#define IAPPRISE_HPP

//
// C++ STL definitions
//

#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <system_error>
#include <cassert>
#include <algorithm>
#include <set>

//
// Implementation specific definitions
//
// 1) inotify -  is a Linux kernel subsystem that acts to extend filesystems 
//    to notice changes to the filesystem, and report those changes to 
//    applications
//

#include <sys/inotify.h>

//
// ================
// CLASS DEFINITION
// ================
//

class CFileApprise {
    
public:
    
    // ==========================
    // PUBLIC TYPES AND CONSTANTS
    // ==========================

    //
    // CFileApprise options structure (optionally passed to CFileApprise constructor)
    //

    struct Options {
        uint32_t inotifyWatchMask;                                // inotify watch event mask
        void (*displayInotifyEvent)(struct inotify_event *event); // Display inotify event to stdout
        void (*coutstr) (const std::vector<std::string>& outstr); // coutstr output
        void (*cerrstr) (const std::vector<std::string>& errstr); // cerrstr output
    };
    
    //
    // CFileApprise Event ids
    //

    enum EventId {
        Event_none = 0,     // None
        Event_add,          // File added to watched folder hierarchy
        Event_change,       // File changed (experimental)
        Event_unlink,       // File deleted from watched folder hierarchy
        Event_addir,        // Directory added to watched folder hierarchy
        Event_unlinkdir,    // Directory deleted from watched folder hierarchy
        Event_error         // Exception error
    };

    //
    // CFileApprise Event structure
    //
    
    struct Event {
        CFileApprise::EventId id;  // Event id
        std::string message;       // Event file name / error message string
    };
    
    // ============
    // CONSTRUCTORS
    // ============
    
    CFileApprise(const std::string& watchFolder,                        // Watch folder path
            int watchDepth,                                             // Watch depth -1=all,0=just watch folder,1=next level down etc.
            std::shared_ptr<CFileApprise::Options> options = nullptr);  // CFileApprise Options (OPTIONAL)
    
    // ==========
    // DESTRUCTOR
    // ==========
    
    virtual ~CFileApprise();

    // =======================
    // PUBLIC MEMBER FUNCTIONS
    // =======================

    // Control
    
    void watch(void);                               // Watch folder(s) for file events to convert for CFileApprise.
    void stop(void);                                // Stop watch loop/thread
    
    // Queue access
    
    void getEvent(CFileApprise::Event& message);                  // Get CFileApprise event (waiting if necessary)
    
    // Private data accessors
    
    bool stillWatching(void);                       // Is watcher loop till active ?.
    std::exception_ptr getThrownException(void);    // Get any exception thrown by watcher to pass down chain
    
    // ================
    // PUBLIC VARIABLES (Are the Devil. Use accessor functions).
    // ================
    
private:

    // ===========================
    // PRIVATE TYPES AND CONSTANTS
    // ===========================

    static const std::string kLogPrefix;        // Logging output prefix 
    static const uint32_t kInofityEvents;       // inotify events to monitor
    static const uint32_t kInotifyEventSize;    // inotify read event size
    static const uint32_t kInotifyEventBuffLen; // inotify read buffer length

    // =====================
    // DISABLED CONSTRUCTORS
    // =====================
    
    CFileApprise() = delete;
    CFileApprise(const CFileApprise & orig) = delete;
    CFileApprise(const CFileApprise && orig) = delete;
 
    // ========================
    // PRIVATE MEMBER FUNCTIONS
    // ========================

    void addWatch(const std::string& filePath);     // Add path to be watched
    void removeWatch(const std::string& filePath);  // Remove path being watched
    void initWatchTable(void);                      // Initialise table for watched folders
    void destroyWatchTable(void);                   // Tare down watch table

    void sendEvent(CFileApprise::EventId id, const std::string& message); // Queue CFileApprise event

    // =================
    // PRIVATE VARIABLES
    // =================
    
    // Constructor passed in and intialised
    
    std::string watchFolder;    // Watch Folder
    int watchDepth;             // Watch depth -1=all,0=just watch folder,1=next level down etc.

    // Inotify
    
    int inotifyFd;                                              // file descriptor for read
    uint32_t inotifyWatchMask = CFileApprise::kInofityEvents;   // watch event mask
    std::unique_ptr<std::uint8_t> inotifyBuffer;                // read buffer
    std::unordered_map<int32_t, std::string> watchMap;          // Watch table indexed by watch variable
    std::unordered_map<std::string, int32_t> revWatchMap;       // Reverse watch table indexed by path
    std::set<std::string> inProcessOfCreation;                  // Set to hold files being created.
    
    // Publicly accessed via accessors
    
    std::exception_ptr thrownException = nullptr;   // Pointer to any exception thrown
    std::atomic<bool> bDoWork;                      // doWork=true (run watcher loop) false=(stop watcher loop)

    // Event queue
    
    std::condition_variable queuedEventsWaiting;    // Queued events conditional
    std::mutex queuedEventsMutex;                   // Queued events mutex
    std::queue <CFileApprise::Event> queuedEvents;  // Queue of CFileApprise events

    // Trace functions default do nothing (Im sure a batter solution exists but fix later).

    void (*displayInotifyEvent)(struct inotify_event *event) = [] (struct inotify_event *event) {
    };
    void (*coutstr) (const std::vector<std::string>& outstr) = [] (const std::vector<std::string>& outstr) {
    };
    void (*cerrstr) (const std::vector<std::string>& errstr) = [] (const std::vector<std::string>& errstr) {
    };


};
#endif /* IAPPRISE_HPP */

