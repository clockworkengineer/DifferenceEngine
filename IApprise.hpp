/*
 * File:  IApprise.hpp
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

// inotify definitions

#include <sys/inotify.h>

// IApprise options structure (optionally pass to IApprise constructor)

struct IAppriseOptions {
    void (*displayInotifyEvent)(struct inotify_event *event);  // Display inotify event to stdout
    void (*coutstr) (const std::vector<std::string>& outstr);  // coutstr output
    void (*cerrstr) (const std::vector<std::string>& errstr);  // cerrstr output
};

// Event ids

enum IAppriseEventId { 
    Event_none=0,       // None
    Event_add,          // File added
    Event_change,       // File changed
    Event_unlink,       // File deleted
    Event_addir,        // Directory added
    Event_unlinkdir,    // Directory deleted
    Event_error         // Exception error
};

// Event structure

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
             std::shared_ptr<IAppriseOptions> options=nullptr);     // IApprise Options
    
    // DESTRUCTOR

    virtual ~IApprise(); // Task class cleanup

    // PUBLIC FUNCTIONS

    void watch(void);                       // Watch folder for file events to convert for IApprise.
    void stop(void);                        // Stop watch loop/thread
    void getEvent(IAppriseEvent& fileName); // Get IApprise event (waiting if necessary)
    bool stillWatching(void);               // Watcher is still active.
 
private:

    IApprise() = delete;                            // Use only provided constructors
    IApprise(const IApprise & orig) = delete;;
    IApprise(const IApprise && orig )= delete;;   
    
    static int pathDepth(std::string &pathStr);     // Add path to be watched
    std::string prefix(void);                       // Logging output prefix 
    
    void addWatchPath(std::string &pathStr);        // Add path to be watched
    void addWatch(struct inotify_event *event);     // Add a folder to watch
    void removeWatch(struct inotify_event *event);  // Remove a folder watch
    void initWatchTable(void);                      // Create a watch table for watched folders
    void destroyWatchTable(void);                   // Clear watch table
    
    void coutstr(const std::vector<std::string>& outstr);       // std::cout
    void cerrstr(const std::vector<std::string>& outstr);       // std::cerr
    
    void sendEvent(IAppriseEventId id, std::string fileName);   // Queue IApprise event
    
    // CONSTRUCTOR PARAMETERS
    
    std::string  watchFolder;                               // Watch Folder
    int maxWatchDepth;                                      // Watch depth -1=all,0=just watch folder,1=next level down etc.
    std::shared_ptr<IAppriseOptions> options;               // IApprise options
     
    int fdNotify;                                           // inotify file descriptor
    
    std::atomic<bool> bDoWork;                              // doWork=true (run watcher loop) false=(stop watcher loop)
    
    std::condition_variable eventsQueued;                   // Queued events considitional
    std::mutex queuedEventsMutex;                           // Queue Mutex
    std::queue <IAppriseEvent> queuedEvents;                // Queue of IApprise events
    
    std::unordered_map<int32_t, std::string> watchMap;      // Watch table indexed by watch variable
    std::unordered_map<std::string, int32_t> revWatchMap;   // Reverse watch table indexed by path
    
    static const uint32_t kInofityEvents;       // inotify events to monitor
    static const uint32_t kInotifyEventSize;    // inotify read event size
    static const uint32_t kInotifyEventBuffLen; // inotify read buffer length

};
#endif /* IAPPRISE_HPP */

