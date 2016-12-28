#include "FPE_HOST.hpp"
/*
 * File:   IApprise.cpp
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

// IApprise class

#include "IApprise.hpp"

//
// CLASS CONSTANTS
//

// inotify events to recieve

const uint32_t IApprise::kInofityEvents = IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE_SELF | IN_CLOSE_WRITE | IN_DELETE;

// inotify event structure size

const uint32_t IApprise::kInotifyEventSize = (sizeof (struct inotify_event));

// inotify event read buffer size

const uint32_t IApprise::kInotifyEventBuffLen = (1024 * (IApprise::kInotifyEventSize + 16));

// IApprise logging prefix

const std::string IApprise::kLogPrefix = "[IApprise] ";

//
// PRIVATE METHODS
//

//
// stdout trace output.
//

void IApprise::coutstr(const std::vector<std::string>& outstr) {

    assert(this->options != nullptr);

    if ((this->options)->coutstr != nullptr) {
        (this->options)->coutstr(outstr);
    }

}

//
// stderr trace output.
//

void IApprise::cerrstr(const std::vector<std::string>& errstr) {

    assert(this->options != nullptr);

    if ((this->options)->cerrstr != nullptr) {
        (this->options)->cerrstr(errstr);
    }

}

//
// Clean up inotifier plus its watch variables and clear watch maps.
//

void IApprise::destroyWatchTable(void) {

    for (auto it = this->watchMap.begin(); it != this->watchMap.end(); ++it) {

        if (inotify_rm_watch(this->inotifyFd, it->first) == -1) {
            throw std::system_error(std::error_code(errno, std::system_category()), "inotify_rm_watch() error");
        } else {
            coutstr({IApprise::kLogPrefix, "Watch[", std::to_string(it->first), "] removed.", "\n"});
        }

    }
    if (close(this->inotifyFd) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify close() error");
    }

    this->watchMap.clear();
    this->revWatchMap.clear();

}

//
// Add watch for newly added directory
//

void IApprise::addWatch(std::string& filePath) {

    int watch;

    // Deeper than max watch depth so ignore.

    if ((this->maxWatchDepth != -1) && (std::count(filePath.begin(), filePath.end(), '/') > this->maxWatchDepth)) {
        return;
    }

    // Add watch

    if ((watch = inotify_add_watch(this->inotifyFd, filePath.c_str(), this->inotifyWatchMask)) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_add_watch() error");
    }

    // Add watch to map and reverse map

    this->watchMap.insert({watch, filePath});
    this->revWatchMap.insert({filePath, watch});

    coutstr({IApprise::kLogPrefix, "Directory add [", filePath, "] watch = [", std::to_string(this->revWatchMap[filePath]), "]"});

}

//
// Initialize inotify and add watch for watch folder.
//

void IApprise::initWatchTable(void) {

    // Initialize inotify 

    if ((this->inotifyFd = inotify_init()) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_init() error");
    }

    this->addWatch(this->watchFolder);

}

//
//  Remove watch for deleted or moved directory
//

void IApprise::removeWatch(std::string& filePath) {

    try {

        int32_t watch;

        watch = this->revWatchMap[filePath];
        if (watch) {

            coutstr({IApprise::kLogPrefix, "Directory remove [", filePath, "] watch = [", std::to_string(watch), "]"});

            this->watchMap.erase(watch);
            this->revWatchMap.erase(filePath);

            if (inotify_rm_watch(this->inotifyFd, watch) == -1) {
                throw std::system_error(std::error_code(errno, std::system_category()), "inotify_rm_watch() error");
            }

        } else {
            cerrstr({IApprise::kLogPrefix, "Directory remove failed [", filePath, "]"});
        }


    } catch (std::system_error &e) {

        // Ignote error 22 and carry on. From the documentation on this error the kernel has removed the watch for us.
        if (e.code() != std::error_code(EINVAL, std::system_category())) {
            throw; // Throw exception back up the chain.
        }

    }

    // No more watches (main watch folder removed) so closedown
    
    if (this->watchMap.size() == 0) {
        coutstr({IApprise::kLogPrefix, "*** Watch Folder Deleted so terminating watch loop. ***"});
        this->stop();
    }

}

//
// Queue IApprise event
//

void IApprise::sendEvent(IAppriseEventId id, std::string fileName) {

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);
    this->queuedEvents.push({id, fileName});
    this->queuedEventsWaiting.notify_one();

}

//
// PUBLIC METHODS
//

//
// IApprise object constructor. 
//

IApprise::IApprise(std::string watchFolder, int maxWatchDepth, std::shared_ptr<IAppriseOptions> options) : watchFolder{watchFolder}, maxWatchDepth{maxWatchDepth}, options{options}
{

    // ASSERT if passed parameters invalid

    assert(watchFolder.length() != 0); // Length == 0
    assert(maxWatchDepth >= -1); // < -1

    // No IApprise option passed in so setup default. THIS NEEDS TO BE SETUP FIRST FOR COUTSTR/CERRSTR.

    if (this->options == nullptr) {
        this->options.reset(new IAppriseOptions{nullptr, nullptr});
    }

    this->inotifyWatchMask = IApprise::kInofityEvents;

    // Add ending '/' if missing from path

    if ((this->watchFolder).back() != '/') {
        (this->watchFolder).push_back('/');
    }

    coutstr({IApprise::kLogPrefix, "Watch folder [", this->watchFolder, "]"});
    coutstr({IApprise::kLogPrefix, "Watch Depth [", std::to_string(maxWatchDepth), "]"});

    // Save away max watch depth and modify with watch folder depth value if not all (-1).

    this->maxWatchDepth = maxWatchDepth;
    if (maxWatchDepth != -1) {
        this->maxWatchDepth += std::count(watchFolder.begin(), watchFolder.end(), '/');
    }

    // Watcher up and running

    this->bDoWork = true;

}

//
// IApprise Destructor
//

IApprise::~IApprise() {

    coutstr({IApprise::kLogPrefix, "DESTRUCTOR CALLED."});

}

//
// IApprise still watching folder(s)
//

bool IApprise::stillWatching(void) {

    return (this->bDoWork.load());

}

//
// Check whether termination of IApprise was the result of any thrown exception
//

std::exception_ptr IApprise::getThrownException() {
    
    return (this->thrownException); 

}

//
// Get next IApprise event in queue.
//

void IApprise::getEvent(IAppriseEvent& evt) {

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
        evt.id = Event_none;
        evt.message = "";
    }

}

//
// Flag watch loop to stop.
//

void IApprise::stop(void) {

    coutstr({IApprise::kLogPrefix, "Stop IApprise thread."});

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);

    this->bDoWork = false;
    this->queuedEventsWaiting.notify_one();

    this->destroyWatchTable();

}

//
// Create watch table and loop adding/removing watches for directory structure changes
// and also generating IApprise events from inotify; until stopped.
//

void IApprise::watch(void) {

    std::uint8_t buffer[IApprise::kInotifyEventBuffLen];

    coutstr({IApprise::kLogPrefix, "IApprise watch loop started"});

    try {

        this->initWatchTable();

        while (this->bDoWork.load()) {

            int readLen, currentPos = 0;

            if ((readLen = read(this->inotifyFd, buffer, IApprise::kInotifyEventBuffLen)) == -1) {
                throw std::system_error(std::error_code(errno, std::system_category()), "inotify read() error");
            }

            while (currentPos < readLen) {

                struct inotify_event *event = (struct inotify_event *) &buffer[ currentPos ];

                if ((this->options)->displayInotifyEvent) {
                    (this->options)->displayInotifyEvent(event);
                }

                switch (event->mask) {

                    case (IN_ISDIR | IN_CREATE):
                    case (IN_ISDIR | IN_MOVED_TO):
                    {
                        std::string filePath{ this->watchMap[event->wd] + std::string(event->name) + "/"};
                        this->sendEvent(Event_addir, filePath);
                        this->addWatch(filePath);
                        break;
                    }

                    case (IN_ISDIR | IN_MOVED_FROM):
                    case IN_DELETE_SELF:
                    {
                        std::string filePath{ this->watchMap[event->wd] + std::string((event->len) ? event->name : "")};
                        if (filePath.back() != '/') {
                            filePath.push_back('/');
                        }
                        this->sendEvent(Event_unlinkdir, filePath);
                        this->removeWatch(filePath);
                        break;
                    }

                    case IN_DELETE:
                    {
                        this->sendEvent(Event_unlink, this->watchMap[event->wd] + std::string(event->name));
                        break;
                    }

                    case IN_CLOSE_WRITE: // This could signal an end to update too but just signal as an add for now
                    case IN_MOVED_TO:
                    {
                        this->sendEvent(Event_add, this->watchMap[event->wd] + std::string(event->name));
                        break;
                    }

                    default:
                        break;

                }

                currentPos += IApprise::kInotifyEventSize + event->len;

            }
        }

    //
    // Generate event for any exceptions and also store to be passed up the chain
        
    } catch (std::system_error &e) {
        this->sendEvent(Event_error, IApprise::kLogPrefix + "Caught a runtime_error exception: [" + e.what() + "]");
        this->thrownException = std::current_exception();
    } catch (std::exception &e) {
        this->sendEvent(Event_error, IApprise::kLogPrefix + "General exception occured: [" + e.what() + "]");
        this->thrownException = std::current_exception();
    }

    // If still active then need to close down

    if (this->bDoWork.load()) {
        this->stop();
    }

    coutstr({IApprise::kLogPrefix, "IApprise watch loop stopped."});

}