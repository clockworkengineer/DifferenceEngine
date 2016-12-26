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

// STL definitions

#include <system_error>
#include <cassert>

// IApprise class

#include "IApprise.hpp"

//
// CLASS CONSTANTS
//

// inotify events to recieve

const uint32_t IApprise::kInofityEvents = IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE | IN_MODIFY;

// inotify event structure size

const uint32_t IApprise::kInotifyEventSize = (sizeof (struct inotify_event));

// inotify event read buffer size

const uint32_t IApprise::kInotifyEventBuffLen = (1024 * (IApprise::kInotifyEventSize + 16));

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
// Count '/' to find directory depth
//

int IApprise::pathDepth(std::string &pathStr) {

    auto i = 0;
    auto pos = pathStr.find("/");
    while (pos != std::string::npos) {
        i++;
        pos++;
        pos = pathStr.find("/", pos);
    }

    return (i);

}

//
// Prefix string for any task logging.
//

std::string IApprise::prefix(void) {

    return ("[IApprise] ");

}

//
// Clean up inotifier plus its watch variables and clear watch maps.
//

void IApprise::destroyWatchTable(void) {

    for (auto it = this->watchMap.begin(); it != this->watchMap.end(); ++it) {

        if (inotify_rm_watch(this->fdNotify, it->first) == -1) {
            throw std::system_error(std::error_code(errno, std::system_category()), "inotify_rm_watch() error");
        } else {
            coutstr({this->prefix(), "Watch[", std::to_string(it->first), "] removed.", "\n"});
        }

    }
    if (close(this->fdNotify) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify close() error");
    }

    this->watchMap.clear();
    this->revWatchMap.clear();

}

//
// Create a inotify watch for the passed in path.
//

void IApprise::addWatchPath(std::string &pathStr) {

    int watch;

    // Deeper than max watch depth so ignore.

    if ((this->maxWatchDepth != -1) && (pathDepth(pathStr) > this->maxWatchDepth)) {
        return;
    }

    // Add watch

    if ((watch = inotify_add_watch(this->fdNotify, pathStr.c_str(), IApprise::kInofityEvents)) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_add_watch() error");
    }

    // Add watch to map and reverse map

    this->watchMap.insert({watch, pathStr});
    this->revWatchMap.insert({pathStr, watch});

    coutstr({this->prefix(), "Directory add [", pathStr, "] watch = [", std::to_string(this->revWatchMap[pathStr]), "]"});

}

//
// Initialize inotify and add watch for watch folder.
//

void IApprise::initWatchTable(void) {

    // Initialize inotify 

    if ((this->fdNotify = inotify_init()) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_init() error");
    }

    this->addWatchPath(this->watchFolder);

}

//
// Add watch for newly added directory
//

void IApprise::addWatch(struct inotify_event *event) {

    // ASSERT if event pointer NULL

    assert(event != nullptr);

    std::string filename = event->name;
    std::string pathStr = this->watchMap[event->wd] + filename + "/";

    this->addWatchPath(pathStr);

}

//
//  Remove watch for deleted or moved directory
//

void IApprise::removeWatch(struct inotify_event *event) {

    try {

        // ASSERT if event pointer NULL

        assert(event != nullptr);

        std::string filename = (event->len) ? event->name : "";
        std::string pathStr = this->watchMap[event->wd];
        int32_t watch;

        if (filename.compare("") != 0) {
            pathStr += filename + "/";
        }

        watch = this->revWatchMap[pathStr];
        if (watch) {

            coutstr({this->prefix(), "Directory remove [", pathStr, "] watch = [", std::to_string(watch), "] File [", filename, "]"});

            this->watchMap.erase(watch);
            this->revWatchMap.erase(pathStr);

            if (inotify_rm_watch(this->fdNotify, watch) == -1) {
                throw std::system_error(std::error_code(errno, std::system_category()), "inotify_rm_watch() error");
            }

        } else {
            cerrstr({this->prefix(), "Directory remove failed [", pathStr, "] File [", filename, "]"});
        }


    } catch (std::system_error &e) {

        // Ignote error 22 and carry on. From the documentation on this error the kernel has removed the watch for us.
        if (e.code() != std::error_code(EINVAL, std::system_category())) {
            throw; // Throw exception back up the chain.
        }

    }

    if (this->watchMap.size() == 0) {
        coutstr({this->prefix(), "*** Watch Folder Deleted so terminating watch loop. ***"});
        this->stop();
    }

}

//
// Queue IApprise event
//

void IApprise::sendEvent(IAppriseEventId id, std::string fileName) {

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);
    this->queuedEvents.push({id, fileName});
    this->eventsQueued.notify_one();

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
        this->options.reset(new IAppriseOptions{nullptr, nullptr, nullptr});
    }

    // Add ending '/' if mssing from path

    if ((this->watchFolder).back() != '/') {
        (this->watchFolder).push_back('/');
    }

    coutstr({this->prefix(), "Watch folder [", this->watchFolder, "]"});
    coutstr({this->prefix(), "Watch Depth [", std::to_string(maxWatchDepth), "]"});

    // Save away max watch depth and modify with watch folder depth value if not all (-1).

    this->maxWatchDepth = maxWatchDepth;
    if (maxWatchDepth != -1) {
        this->maxWatchDepth += pathDepth(watchFolder);
    }

    // Watcher up and running

    this->bDoWork = true;

}

//
// IApprise Destructor
//

IApprise::~IApprise() {

    coutstr({this->prefix(), "DESTRUCTOR CALLED."});

}

//
// IApprise still watching folder(s)
//

bool IApprise::stillWatching(void) {

    return (this->bDoWork.load());

}

//
// Get next IApprise event in queue.
//

void IApprise::getEvent(IAppriseEvent& evt) {

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);

    // Wait for something to happen. Either an event or stop running

    this->eventsQueued.wait(locker, [&]() {
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

    coutstr({this->prefix(), "Stop IApprise thread."});

    std::unique_lock<std::mutex> locker(this->queuedEventsMutex);

    this->bDoWork = false;
    this->eventsQueued.notify_one();

    this->destroyWatchTable();

}

//
// Create watch table and loop adding/removing watches for directory structure changes
// and also generating IApprise events from inotify; until stopped.
//

void IApprise::watch(void) {

    std::uint8_t buffer[IApprise::kInotifyEventBuffLen];

    std::thread::id this_id = std::this_thread::get_id();

    coutstr({this->prefix(), "IApprise watch loop started [", "]"});

    try {

        this->initWatchTable();

        while (this->bDoWork.load()) {

            int readLen, currentPos = 0;

            if ((readLen = read(this->fdNotify, buffer, IApprise::kInotifyEventBuffLen)) == -1) {
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
                        this->sendEvent(Event_addir, this->watchMap[event->wd] + std::string(event->name));
                        this->addWatch(event);
                        break;

                    case (IN_ISDIR | IN_MOVED_FROM):
                    case IN_DELETE_SELF:
                        this->sendEvent(Event_unlinkdir, this->watchMap[event->wd]);
                        this->removeWatch(event);
                        break;

                    case IN_DELETE:
                        this->sendEvent(Event_unlink, this->watchMap[event->wd] + std::string(event->name));
                        break;

                    case IN_CLOSE_WRITE:
                    case IN_MOVED_TO:
                        this->sendEvent(Event_add, this->watchMap[event->wd] + std::string(event->name));
                        break;

                }

                currentPos += IApprise::kInotifyEventSize + event->len;

            }
        }

    } catch (std::system_error &e) {
        this->sendEvent(Event_error, this->prefix() + "Caught a runtime_error exception: [" + e.what() + "]");
    } catch (std::exception &e) {
        this->sendEvent(Event_error, this->prefix() + "General exception occured: [" + e.what() + "]");
    }

    // If still active then need to close down

    if (this->bDoWork.load()) {
        this->stop();
    }

    coutstr({this->prefix(), "IApprise watch loop stopped."});

}