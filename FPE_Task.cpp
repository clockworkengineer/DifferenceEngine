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
// Destructor
//

FPE_Task::~FPE_Task() {

    std::cout << this->prefix() << "DESTRUCTOR CALLED." << std::endl;

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
            std::stringstream errStream;
            errStream << "inotify_rm_watch() error:  " << errno;
            throw std::runtime_error(errStream.str());
        } else {
            std::cout << this->prefix() << "Watch[" << it->first << "] removed." << std::endl;
        }
    }
    if (close(this->fdNotify) == -1) {
        std::stringstream errStream;
        errStream << "inotify close() error:  " << errno;
        throw std::runtime_error(errStream.str());
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
        std::stringstream errStream;
        errStream << "inotify_add_watch() error:  " << errno;
        throw std::runtime_error(errStream.str());
    }

    // Add watch to map and reverse map

    this->watchMap.insert({watch, pathStr});
    this->revWatchMap.insert({pathStr, watch});

    std::cout << this->prefix() << "Directory add [" << pathStr << "] watch = [" << this->revWatchMap[pathStr] << "]" << std::endl;

}

//
// Initialize inotify and add watches for any existing directory structure.
//

void FPE_Task::createWatchTable(void) {

    // Initialize inotify 

    if ((this->fdNotify = inotify_init()) == -1) {
        std::stringstream errStream;
        errStream << "inotify_init() error:  " << errno;
        throw std::runtime_error(errStream.str());
    }

    this->addWatchPath(this->watchFolder);

    for (fs::recursive_directory_iterator i(this->watchFolder), end; i != end; ++i) {
        if (fs::is_directory(i->path())) {
            std::string pathStr = i->path().string() + "/";
            if (fs::exists(fs::path(pathStr))) {
                this->addWatchPath(pathStr);
            }
        }
    }

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

            std::cout << this->prefix() << "Directory remove [" << pathStr << "] watch = [" << watch << "] File [" << filename << "]" << std::endl;

            this->watchMap.erase(watch);
            this->revWatchMap.erase(pathStr);

            if (inotify_rm_watch(this->fdNotify, watch) == -1) {
                std::stringstream errStream;
                errStream << "inotify_rm_watch() error:  " << errno;
                throw std::runtime_error(errStream.str());
            }

        } else {
            std::cerr << this->prefix() << "Directory remove failed [" << pathStr << "] File [" << filename << "]" << std::endl;
        }

        if (this->watchMap.size() == 1) {
            std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;
        }

    } catch (std::runtime_error &e) {
        // Report error 22 and carry on. From the documentation on this error the kernel has removed the watch for us.
        if (errno == EINVAL) {
            std::cerr << this->prefix() << "Caught a runtime_error exception: " << e.what() << std::endl;
            if (this->watchMap.size() == 1) {
                std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;
            }
        } else {
            throw; // Throw exception back up the chain.
        }
    }

}

//
// Worker thread. Remove path/filename from queue and process.
// Access to fileNames queue controlled by lockguard(mutex) and 
// loop is controlled by atomic bool doWork flag.
//

void FPE_Task::worker(void) {

    bool bFilesToProcess;
    std::string filenamePathStr;
    std::string filenameStr;

    std::cout << this->prefix() << "Worker thread started... " << std::endl;

    while (this->bDoWork.load()) {

        try {

            do {
                do {
                    std::lock_guard<std::mutex> guard(this->fileNamesMutex);
                    bFilesToProcess = !this->fileNames.empty();
                    if (bFilesToProcess) {
                        filenamePathStr = this->fileNames.front();
                        this->fileNames.pop();
                        filenameStr = this->fileNames.front();
                        this->fileNames.pop();
                    }
                } while (false); // lock guard out of scope so mutex off
                if (bFilesToProcess) {
                    this->taskActFcn(filenamePathStr, filenameStr, this->fnData);
                    if ((this->killCount != 0) && (--(this->killCount) == 0)) {
                        std::cout << this->prefix() << "FPE_Task Kill Count reached." << std::endl;
                        this->stop();
                    }

                }
            } while (bFilesToProcess);

            std::this_thread::sleep_for(std::chrono::seconds(1)); // This works better than yield

        } catch (const fs::filesystem_error& e) {
            std::cerr << this->prefix() << "BOOST file system exception occured: " << e.what() << std::endl;
        } catch (std::runtime_error &e) {
            std::cerr << this->prefix() << "Caught a runtime_error exception: " << e.what() << std::endl;
        } catch (std::exception &e) {
            std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << this->prefix() << "unknown exception occured" << std::endl;
        }

    }

    std::cout << this->prefix() << "Worker thread stopped. " << std::endl;

}

//
// PUBLIC METHODS
//

//
// Task object constructor. 
//

FPE_Task::FPE_Task(std::string taskNameStr, std::string watchFolder,
        TaskActionFcn taskActFcn, std::shared_ptr<void> fnData, int maxWatchDepth, int killCount) :
taskName{taskNameStr}, taskActFcn{taskActFcn}, fnData{fnData}, watchFolder{watchFolder}, killCount{killCount}
{

    // ASSERT if passed parameters invalid

    assert(taskNameStr.length() != 0); // Length == 0
    assert(watchFolder.length() != 0); // Length == 0
    assert(maxWatchDepth >= -1);       // < -1
    assert(taskActFcn != nullptr);     // nullptr
    assert(fnData != nullptr);         // nulptr
    assert(killCount >=0);             // < 0

    std::cout << this->prefix() << "Watch Folder [" << watchFolder << "]" << std::endl;

    // Create watch directory.

    if (!fs::exists(watchFolder)) {
        std::cout << this->prefix() << "Watch Folder [" << watchFolder << "] DOES NOT EXIST." << std::endl;
        if (fs::create_directory(watchFolder)) {
            std::cout << this->prefix() << "Creating Watch Folder [" << watchFolder << "]" << std::endl;
        }
    }

    std::cout << this->prefix() << "Watch Depth [" << maxWatchDepth << "]" << std::endl;

    // Save away max watch depth and modify with watch folder depth value if not all (-1).

    this->maxWatchDepth = maxWatchDepth;
    if (maxWatchDepth != -1) {
        this->maxWatchDepth += pathDepth(watchFolder);
    }

    // All threads start working

    this->bDoWork = true;

}

//
// Flag thread loops to stop. Folder watcher needs a push because of wait for read().
// Also clean up any resources.
//

void FPE_Task::stop(void) {

    std::cout << this->prefix() << "Stop task threads." << std::endl;

    this->bDoWork = false;
    if (fs::is_empty(this->watchFolder) || fs::exists(this->watchFolder)) {
        std::cout << this->prefix() << "Close down folder watcher thread" << std::endl;
    }
    this->destroyWatchTable();

}

//
// Create watch table and loop forever adding/removing watches for directory structure changes
// and also activate task processing for any non-directory file added.
//

void FPE_Task::monitor(void) {

    std::uint8_t buffer[FPE_Task::kInotifyEventBuffLen];

    std::thread::id this_id = std::this_thread::get_id();

    std::cout << this->prefix() << "FPE_Task Monitor on Thread started [" << this_id << "]" << std::endl;

    try {

        this->createWatchTable();

        this->bDoWork = true;
        this->workerThread.reset(new std::thread(&FPE_Task::worker, this));

        while (this->bDoWork.load()) {

            int readLen, currentPos = 0;

            if ((readLen = read(this->fdNotify, buffer, FPE_Task::kInotifyEventBuffLen)) == -1) {
                std::stringstream errStream;
                errStream << "inotify read() error: " << errno;
                throw std::runtime_error(errStream.str());
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
                        std::lock_guard<std::mutex> guard(this->fileNamesMutex);
                        this->fileNames.push(this->watchMap[event->wd]);
                        this->fileNames.push(event->name);
                        break;

                }

                currentPos += FPE_Task::kInotifyEventSize + event->len;

            }
        }

        // Wait for worker thread to exit
        
        this->workerThread->join();
        
    } catch (const fs::filesystem_error& e) {
        std::cerr << this->prefix() << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::runtime_error &e) {
        std::cerr << this->prefix() << "Caught a runtime_error exception: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << this->prefix() << "unknown exception occured" << std::endl;
    }

    std::cout << this->prefix() << "FPE_Task Monitor on Thread stopped." << std::endl;

}