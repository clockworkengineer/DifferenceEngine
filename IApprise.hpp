#include "HOST.hpp"
/*
 * File:   IApprise.hpp
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

#ifndef IAPPRISE_HPP
#define IAPPRISE_HPP

// STL definitions

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

// inotify definitions

#include <sys/inotify.h>

// IApprise options structure (optionally pass to IApprise constructor)

struct IAppriseOptions {
    void (*displayInotifyEvent)(struct inotify_event *event);  // Display inotify event to stdout
    void (*coutstr) (const std::vector<std::string>& outstr);  // coutstr output
    void (*cerrstr) (const std::vector<std::string>& errstr);  // cerrstr output
};

// Apprise Event ids

enum IAppriseEventId { 
    Event_none=0,       // None
    Event_add,          // File added
    Event_change,       // File changed (not supported at present)
    Event_unlink,       // File deleted
    Event_addir,        // Directory added
    Event_unlinkdir,    // Directory deleted
    Event_error         // Exception error
};

// Apprise Event structure

struct IAppriseEvent {
    IAppriseEventId     id;        // Event id
    std::string         message;   // Event file name / error message string
};

// IApprise class

class IApprise {
    
public:

    // CONSTRUCTOR

    IApprise(std::string watchFolder,                               // Watch folder path
             int maxWatchDepth,                                     // Maximum watch depth -1= all, 0=just watch folder
             std::shared_ptr<IAppriseOptions> options=nullptr);     // IApprise Options (OPTIONAL)
    
    // DESTRUCTOR

    virtual ~IApprise(); // Task class cleanup

    // PUBLIC MEMBER FUNCTIONS

    void watch(void);                           // Watch folder for file events to convert for IApprise.
    void stop(void);                            // Stop watch loop/thread
    void getEvent(IAppriseEvent& message);      // Get IApprise event (waiting if necessary)
    bool stillWatching(void);                   // Watcher is still active.
    std::exception_ptr getThrownException();    // Get any exception thrown by watcher to pass down chain
 
private:
    
    // DISABLED CONSTRUCTORS

    IApprise() = delete; 
    IApprise(const IApprise & orig) = delete;
    IApprise(const IApprise && orig )= delete;
    
     // PRIVATE MEMBER FUNCTIONS
    
    void addWatch(std::string& filePath);           // Add path to be watched
    void removeWatch(std::string& filePath);        // Remove path being watched
    void initWatchTable(void);                      // Initialise table for watched folders
    void destroyWatchTable(void);                   // Tare down watch table
    
    void coutstr(const std::vector<std::string>& outstr);       // std::cout
    void cerrstr(const std::vector<std::string>& outstr);       // std::cerr
    
    void sendEvent(IAppriseEventId id, std::string fileName);   // Queue IApprise event
    
    // PRIVATE VARIABLES
    
    std::string  watchFolder;                               // Watch Folder
    int maxWatchDepth;                                      // Watch depth -1=all,0=just watch folder,1=next level down etc.
    std::shared_ptr<IAppriseOptions> options;               // IApprise options
     
    int inotifyFd;                                          // inotify file 
    uint32_t inotifyWatchMask;                              // inotify watch event mask
    
    std::exception_ptr thrownException=nullptr;             // Pointer to any exception thrown
    
    std::atomic<bool> bDoWork;                              // doWork=true (run watcher loop) false=(stop watcher loop)
    
    std::condition_variable queuedEventsWaiting;            // Queued events considitional
    std::mutex queuedEventsMutex;                           // Queued events mutex
    std::queue <IAppriseEvent> queuedEvents;                // Queue of IApprise events
    
    std::unordered_map<int32_t, std::string> watchMap;      // Watch table indexed by watch variable
    std::unordered_map<std::string, int32_t> revWatchMap;   // Reverse watch table indexed by path
    
    // CONSTANTS
    
    static const std::string kLogPrefix;        // Logging output prefix 
    
    static const uint32_t kInofityEvents;       // inotify events to monitor
    static const uint32_t kInotifyEventSize;    // inotify read event size
    static const uint32_t kInotifyEventBuffLen; // inotify read buffer length

};
#endif /* IAPPRISE_HPP */

