#include "HOST.hpp"
/*
 * File:   CFileApprise.cpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 * 
 * Class: CFileApprise
 * 
 * Description: A simple C++ class to enable files/folders to be watched and 
 * events generated. Supported events include the addition/deletion of files and
 * directories and the modification of files with a change event. It is recursive 
 * by default and any directories added/removed from the hierarchy will cause new 
 * watches to be added/removed respectively. The current implementation is for 
 * POSIX only or any platform that has inotify or a third party equivalent.
 * 
 * Dependencies: C11++, Classes (CLogger), inotify
 *
 * Copyright 2016.
 *
 */

// =================
// CLASS DEFINITIONS
// =================

#include "CFileApprise.hpp"

// ====================
// CLASS IMPLEMENTATION
// ====================

// ===========================
// PRIVATE TYPES AND CONSTANTS
// ===========================

// inotify events to recieve

const uint32_t CFileApprise::kInofityEvents =  IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM |
                                               IN_DELETE_SELF | IN_CLOSE_WRITE | IN_DELETE | IN_MODIFY;

// inotify event structure size

const uint32_t CFileApprise::kInotifyEventSize = (sizeof (struct inotify_event));

// inotify event read buffer size

const uint32_t CFileApprise::kInotifyEventBuffLen = (1024 * (CFileApprise::kInotifyEventSize + 16));

// CFileApprise logging prefix

const std::string CFileApprise::kLogPrefix = "[CFileApprise] ";

// ===============
// PRIVATE METHODS
// ===============

//
// Clean up inotify.
//

void CFileApprise::destroyWatchTable(void) {

     for (auto it = this->watchMap.begin(); it != this->watchMap.end(); ++it) {

        if (inotify_rm_watch(this->inotifyFd, it->first) == -1) {
            throw std::system_error(std::error_code(errno, std::system_category()), "inotify_rm_watch() error");
        } else {
            coutstr({CFileApprise::kLogPrefix, "Watch[", std::to_string(it->first), "] removed.", "\n"});
        }

    }

    if (close(this->inotifyFd) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify close() error");
    }
    
}

//
// Add watch for newly added directory
//

void CFileApprise::addWatch(const std::string& filePath) {

    int watch;

    // Deeper than max watch depth so ignore.

    if ((this->watchDepth != -1) && (std::count(filePath.begin(), filePath.end(), '/') > this->watchDepth)) {
        return;
    }

    // Add watch to inotify 

    if ((watch = inotify_add_watch(this->inotifyFd, filePath.c_str(), this->inotifyWatchMask)) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_add_watch() error");
    }

    // Add watch to map and reverse map

    this->watchMap.insert({watch, filePath});
    this->revWatchMap.insert({filePath, watch});

    this->coutstr({CFileApprise::kLogPrefix, "Directory add [", filePath, "] watch = [", std::to_string(this->revWatchMap[filePath]), "]"});

}

//
// Initialize inotify and add watch for watchFolder.
//

void CFileApprise::initWatchTable(void) {

    // Initialize inotify 

    if ((this->inotifyFd = inotify_init()) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_init() error");
    }

    this->addWatch(this->watchFolder);

}

//
//  Remove watch for deleted or moved directory
//

void CFileApprise::removeWatch(const std::string& filePath) {

    try {

        int32_t watch;

        watch = this->revWatchMap[filePath];
        if (watch) {

            this->coutstr({CFileApprise::kLogPrefix, "Directory remove [", filePath, "] watch = [", std::to_string(watch), "]"});

            this->watchMap.erase(watch);
            this->revWatchMap.erase(filePath);

            if (inotify_rm_watch(this->inotifyFd, watch) == -1) {
                throw std::system_error(std::error_code(errno, std::system_category()), "inotify_rm_watch() error");
            }

        } else {
            this->cerrstr({CFileApprise::kLogPrefix, "Directory remove failed [", filePath, "]"});
        }


    } catch (std::system_error &e) {
        // Ignore error 22 and carry on. From the documentation on this error the kernel has removed the watch for us.
        if (e.code() != std::error_code(EINVAL, std::system_category())) {
            throw; // Throw exception back up the chain.
        }
    }

    // No more watches (main watch folder removed) so closedown

    if (this->watchMap.size() == 0) {
        this->coutstr({CFileApprise::kLogPrefix, "*** Watch Folder Deleted so terminating watch loop. ***"});
        this->stop();
    }

}

//
// Queue CFileApprise event
//

void CFileApprise::sendEvent(CFileApprise::EventId id, const std::string& fileName) {

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);
    this->queuedEvents.push({id, fileName});
    this->queuedEventsWaiting.notify_one();

}

// ==============
// PUBLIC METHODS
// ==============

//
// CFileApprise object constructor. 
//

CFileApprise::CFileApprise(const std::string& watchFolder, int watchDepth, std::shared_ptr<CFileApprise::Options> options) : watchFolder{watchFolder}, watchDepth{watchDepth}
{

    // ASSERT if passed parameters invalid

    assert(watchFolder.length() != 0); // Length == 0
    assert(watchDepth >= -1); // < -1

    // If options passed then setup trace functions and event mask

    if (options) {
        if (options->inotifyWatchMask) {
            this->inotifyWatchMask = options->inotifyWatchMask;
        }
        if (options->displayInotifyEvent) {
            this->displayInotifyEvent = options->displayInotifyEvent;
        }
        if (options->coutstr) {
            this->coutstr = options->coutstr;
        }
        if (options->cerrstr) {
            this->cerrstr = options->cerrstr;
        }
    } 

    // Add ending '/' if missing from path

    if ((this->watchFolder).back() != '/') {
        (this->watchFolder).push_back('/');
    }

    this->coutstr({CFileApprise::kLogPrefix, "Watch folder [", this->watchFolder, "]"});
    this->coutstr({CFileApprise::kLogPrefix, "Watch Depth [", std::to_string(watchDepth), "]"});

    // Save away max watch depth and modify with watch folder depth value if not all (-1).

    this->watchDepth = watchDepth;
    if (watchDepth != -1) {
        this->watchDepth += std::count(watchFolder.begin(), watchFolder.end(), '/');
    }

    // Allocate inotify read buffer

    this->inotifyBuffer.reset(new u_int8_t [CFileApprise::kInotifyEventBuffLen]);

    // Create watch table

    this->initWatchTable();

    // Watcher up and running

    this->bDoWork = true;

}

//
// CFileApprise Destructor
//

CFileApprise::~CFileApprise() {

    this->coutstr({CFileApprise::kLogPrefix, "DESTRUCTOR CALLED."});

}

//
// CFileApprise still watching folder(s)
//

bool CFileApprise::stillWatching(void) {

    return (this->bDoWork.load());

}

//
// Check whether termination of CFileApprise was the result of any thrown exception
//

std::exception_ptr CFileApprise::getThrownException(void) {

    return (this->thrownException);

}

//
// Get next CFileApprise event in queue.
//

void CFileApprise::getEvent( CFileApprise::Event& evt) {

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);

    // Wait for something to happen. Either an event or stop running

    this->queuedEventsWaiting.wait(locker, [&]() {
        return (!this->queuedEvents.empty() || !this->bDoWork.load());
    });

    // return next event from queue

    if (!this->queuedEvents.empty()) {
        evt = this->queuedEvents.front();
        this->queuedEvents.pop();
    } else {
        evt.id = CFileApprise::Event_none;
        evt.message = "";
    }

}

//
// Flag watch loop to stop.
//

void CFileApprise::stop(void) {

    this->coutstr({CFileApprise::kLogPrefix, "Stop CFileApprise thread."});

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);
    this->bDoWork = false;
    this->queuedEventsWaiting.notify_one();

    this->destroyWatchTable();

}

//
// Loop adding/removing watches for directory hierarchy  changes
// and also generating CFileApprise events from inotify; until stopped.
//

void CFileApprise::watch(void) {

    std::uint8_t *buffer = this->inotifyBuffer.get();
    struct inotify_event *event; 
    std::string  filePath;

    this->coutstr({CFileApprise::kLogPrefix, "CFileApprise watch loop started"});

    try {

        // Loop until told to stop
        
        while (this->bDoWork.load()) {

            int readLen, currentPos = 0;

            // Read in events
            
            if ((readLen = read(this->inotifyFd, buffer, CFileApprise::kInotifyEventBuffLen)) == -1) {
                throw std::system_error(std::error_code(errno, std::system_category()), "inotify read() error");
            }

            // Loop until all read processed
            
            while (currentPos < readLen) {

                // Point to next event & display if necessary
                
                event = (struct inotify_event *) &buffer[ currentPos ];
                currentPos += CFileApprise::kInotifyEventSize + event->len;

                this->displayInotifyEvent(event);

                // IGNORE so move onto next event
                
                if (event->mask == IN_IGNORED) {
                    continue;
                }

                // Create full filename path
                
                filePath = this->watchMap[event->wd] + ((event->len) ? event->name : "");
   
                // Process event
                
                switch (event->mask) {

                    // Flag file as being created
                    
                    case IN_CREATE:
                    {
                        this->inProcessOfCreation.insert(filePath);
                        break;
                    }

                    // If file not being created send Event_change
                    
                    case IN_MODIFY:
                    {
                        auto beingCreated = this->inProcessOfCreation.find(filePath);
                        if (beingCreated == this->inProcessOfCreation.end()) {
                            this->sendEvent(CFileApprise::Event_change, filePath);
                        }
                        break;
                    }

                    // Add watch for new directory and send Event_addir
                    
                    case (IN_ISDIR | IN_CREATE):
                    case (IN_ISDIR | IN_MOVED_TO):
                    {
                        if (filePath.back() != '/') {
                            filePath.push_back('/');
                        }
                        this->sendEvent(CFileApprise::Event_addir, filePath);
                        this->addWatch(filePath);
                        break;
                    }

                    // Remove watch for removed directory and send Event_unlinkdir
                    
                    case (IN_ISDIR | IN_MOVED_FROM):
                    case IN_DELETE_SELF:
                    {
                        if (filePath.back() != '/') {
                            filePath.push_back('/');
                        }
                        this->sendEvent(CFileApprise::Event_unlinkdir, filePath);
                        this->removeWatch(filePath);
                        break;
                    }

                    // File deleted send Event_unlink
                    
                    case IN_DELETE:
                    {
                        this->sendEvent(CFileApprise::Event_unlink, filePath);
                        break;
                    }
                    
                    // File closed/moved. If being created send Event_add otherwise Event_change.

                    case IN_CLOSE_WRITE:
                    case IN_MOVED_TO:
                    {
                        auto beingCreated = this->inProcessOfCreation.find(filePath);
                        if (beingCreated == this->inProcessOfCreation.end()) {
                            this->sendEvent(CFileApprise::Event_change, filePath);
                        } else {
                            this->inProcessOfCreation.erase(filePath);
                            this->sendEvent(CFileApprise::Event_add, filePath);
                        }
                        break;
                    }

                    default:
                        break;

                }

            }

        }

    //
    // Generate event for any exceptions and also store to be passed up the chain
    //

    } catch (std::system_error &e) {
        this->sendEvent(CFileApprise::Event_error, CFileApprise::kLogPrefix + "Caught a runtime_error exception: [" + e.what() + "]");
        this->thrownException = std::current_exception();
        
    } catch (std::exception &e) {
        this->sendEvent(CFileApprise::Event_error, CFileApprise::kLogPrefix + "General exception occured: [" + e.what() + "]");
        this->thrownException = std::current_exception();
    }

    // If still active then need to close down

    if (this->bDoWork.load()) {
        this->stop();
    }

    this->coutstr({CFileApprise::kLogPrefix, "CFileApprise watch loop stopped."});

}