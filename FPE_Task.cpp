#include "FPE_HOST.hpp"
/*
 * File:   FPE_Task.cpp
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

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Task class

#include "FPE_Task.hpp"

//
// CLASS CONSTANTS
//

// inotify events to recieve

const uint32_t FPE_Task::kInofityEvents = IN_ACCESS | IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM |
        IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO;

// inotify event structure size

const uint32_t FPE_Task::kInotifyEventSize = (sizeof (struct inotify_event));

// inotify event read buffer size

const uint32_t FPE_Task::kInotifyEventBuffLen = (1024 * (FPE_Task::kInotifyEventSize + 16));

//
// stdout trace output.
//

void FPE_Task::coutstr(const std::vector<std::string>& outstr) {

    assert(this->taskOptions != nullptr);

    if ((this->taskOptions)->coutstr != nullptr) {
        (this->taskOptions)->coutstr(outstr);
    }

}

//
// stderr trace output.
//

void FPE_Task::cerrstr(const std::vector<std::string>& errstr) {

    assert(this->taskOptions != nullptr);

    if ((this->taskOptions)->cerrstr != nullptr) {
        (this->taskOptions)->cerrstr(errstr);
    }

}

//
// Destructor
//

FPE_Task::~FPE_Task() {

    coutstr({this->prefix(), "DESTRUCTOR CALLED."});

}

//
// PRIVATE METHODS
//

//
// Count '/' to find directory depth
//

int FPE_Task::pathDepth(std::string &pathStr) {

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

std::string FPE_Task::prefix(void) {

    return ("TASK [" + this->taskName + "] ");

}

//
// Clean up inotifier plus its watch variables and clear watch maps.
//

void FPE_Task::destroyWatchTable(void) {

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

void FPE_Task::addWatchPath(std::string &pathStr) {

    int watch;

    // Deeper than max watch depth so ignore.

    if ((this->maxWatchDepth != -1) && (pathDepth(pathStr) > this->maxWatchDepth)) {
        return;
    }

    // Add watch

    if ((watch = inotify_add_watch(this->fdNotify, pathStr.c_str(), FPE_Task::kInofityEvents)) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_add_watch() error");
    }

    // Add watch to map and reverse map

    this->watchMap.insert({watch, pathStr});
    this->revWatchMap.insert({pathStr, watch});

    coutstr({this->prefix(), "Directory add [", pathStr, "] watch = [", std::to_string(this->revWatchMap[pathStr]), "]"});

}

//
// Initialize inotify and add watches for any existing directory structure.
//

void FPE_Task::createWatchTable(void) {

    // Initialize inotify 

    if ((this->fdNotify = inotify_init()) == -1) {
        throw std::system_error(std::error_code(errno, std::system_category()), "inotify_init() error");
    }

    this->addWatchPath(this->watchFolder);

}

//
// Add watch for newly added directory
//

void FPE_Task::addWatch(struct inotify_event *event) {

    // ASSERT if event pointer NULL

    assert(event != nullptr);

    std::string filename = event->name;
    std::string pathStr = this->watchMap[event->wd] + filename + "/";

    this->addWatchPath(pathStr);

}

//
//  Remove watch for deleted or moved directory
//

void FPE_Task::removeWatch(struct inotify_event *event) {

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

        if (this->watchMap.size() == 0) {
            coutstr({this->prefix(), "*** Watch Folder Deleted so terminating task. ***"});
            this->stop();
        }

    } catch (std::system_error &e) {
        // Report error 22 and carry on. From the documentation on this error the kernel has removed the watch for us.
        if (e.code() == std::error_code(EINVAL, std::system_category())) {
            if (this->watchMap.size() == 0) {
                coutstr({this->prefix(), "*** Watch Folder Deleted so terminating task. ***"});
                this->stop();
            }
        } else {
            throw; // Throw exception back up the chain.
        }
    }

}

//
// Worker thread. Remove path/filename from queue and process.
// Access to fileNames queue controlled by mutex fileNamesMutex
// and condition variable filesQueued. The loop is controlled by the 
// atomic bool doWork flag (if set to false then stop thread).
//

void FPE_Task::worker(void) {

    std::string filenamePathStr;
   // std::string filenameStr;

    coutstr({this->prefix(), "Worker thread started... "});

    while (this->bDoWork.load()) {

        try {

            // Wait for files to be queued and then process. (Note also wait on
            // bDoWork which can be set to false to stop the worker thread).
            
            std::unique_lock<std::mutex> locker(this->fileNamesMutex);

            this->filesQueued.wait(locker, [&]() {
                return (!this->fileNames.empty() || !this->bDoWork.load());
            });

            if (this->bDoWork.load()) {
                filenamePathStr = this->fileNames.front();
                this->fileNames.pop();
                this->taskActFcn(filenamePathStr,this->fnData);
            }

        } catch (std::system_error &e) {
            cerrstr({this->prefix(), "Caught a runtime_error exception: [", e.what(), "]"});
        } catch (std::exception &e) {
            cerrstr({this->prefix(), "General exception occured: [", e.what(), "]"});
        } catch (...) {
            cerrstr({this->prefix(), "unknown exception occured."});
        }

        if ((this->taskOptions->killCount != 0) && (--(this->taskOptions->killCount) == 0)) {
            coutstr({this->prefix(), "FPE_Task Kill Count reached."});
            this->stop();
        }

    }

    coutstr({this->prefix(), "Worker thread stopped. "});

}

//
// PUBLIC METHODS
//

//
// Task object constructor. 
//

FPE_Task::FPE_Task(std::string taskNameStr, std::string watchFolder, TaskActionFcn taskActFcn, std::shared_ptr<void> fnData,
        int maxWatchDepth, std::shared_ptr<TaskOptions> taskOptions) :
taskName{taskNameStr}, watchFolder{watchFolder}, taskActFcn{taskActFcn},
fnData{fnData}, maxWatchDepth{maxWatchDepth}, taskOptions{taskOptions}
{

    // ASSERT if passed parameters invalid

    assert(taskNameStr.length() != 0); // Length == 0
    assert(watchFolder.length() != 0); // Length == 0
    assert(maxWatchDepth >= -1); // < -1
    assert(taskActFcn != nullptr); // nullptr
    assert(fnData != nullptr); // nullptr

    // No task option passed in so setup default. THIS NEEDS TO BE SETUP FIRST FOR COUTSTR/CERRSTR.

    if (this->taskOptions == nullptr) {
        this->taskOptions.reset(new TaskOptions{0, nullptr, nullptr});
    }

    coutstr({this->prefix(), "Watch folder [", watchFolder, "]"});
    coutstr({this->prefix(), "Watch Depth [", std::to_string(maxWatchDepth), "]"});

    // Save away max watch depth and modify with watch folder depth value if not all (-1).

    this->maxWatchDepth = maxWatchDepth;
    if (maxWatchDepth != -1) {
        this->maxWatchDepth += pathDepth(watchFolder);
    }

    // All threads start working

    this->bDoWork = true;

}

//
// Flag thread loops to stop. This involves seding a notify to the worker thread.
//

void FPE_Task::stop(void) {

    coutstr({this->prefix(), "Stop task threads."});

    std::unique_lock<std::mutex> locker(this->fileNamesMutex);
    this->bDoWork = false;
    this->filesQueued.notify_one();

    coutstr({this->prefix(), "Close down folder watcher thread"});

    this->destroyWatchTable();

}

//
// Create watch table and loop forever adding/removing watches for directory structure changes
// and also activate task processing for any non-directory file added.
//

void FPE_Task::monitor(void) {

    std::uint8_t buffer[FPE_Task::kInotifyEventBuffLen];

    std::thread::id this_id = std::this_thread::get_id();

    coutstr({this->prefix(), "FPE_Task monitor on Thread started [", "]"});

    try {

        this->createWatchTable();

        this->bDoWork = true;
        this->workerThread.reset(new std::thread(&FPE_Task::worker, this));

        while (this->bDoWork.load()) {

            int readLen, currentPos = 0;

            if ((readLen = read(this->fdNotify, buffer, FPE_Task::kInotifyEventBuffLen)) == -1) {
                throw std::system_error(std::error_code(errno, std::system_category()), "inotify read() error");
            }

            while (currentPos < readLen) {

                struct inotify_event *event = (struct inotify_event *) &buffer[ currentPos ];

                switch (event->mask) {

                    case (IN_ISDIR | IN_CREATE):
                    case (IN_ISDIR | IN_MOVED_TO):
                        this->addWatch(event);
                        break;
                    case (IN_ISDIR | IN_MOVED_FROM):
                    case IN_DELETE_SELF:
                        this->removeWatch(event);
                        break;
                    case IN_CLOSE_WRITE:
                    case IN_MOVED_TO:
                        std::unique_lock<std::mutex> locker(this->fileNamesMutex);
                        this->fileNames.push(this->watchMap[event->wd]+ std::string(event->name));
                        this->filesQueued.notify_one();
                        break;

                }

                currentPos += FPE_Task::kInotifyEventSize + event->len;

            }
        }

    } catch (std::system_error &e) {
        cerrstr({this->prefix(), "Caught a runtime_error exception: [", e.what(), "]"});
    } catch (std::exception &e) {
        cerrstr({this->prefix(), "General exception occured: [", e.what(), "]"});
    } catch (...) {
        cerrstr({this->prefix(), "unknown exception occured."});
    }

    // Wait for worker thread to exit

    if (this->workerThread!=nullptr) {
        this->workerThread->join();
    }

    coutstr({this->prefix(), "FPE_Task monitor on thread stopped."});

}