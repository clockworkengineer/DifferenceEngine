/*
 * The MIT License
 *
 * Copyright 2016 Robert Tizzard.
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
 * File:   FPETask.cpp
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 */

#include "FPETask.hpp"

// inotify events to recieve

const uint32_t FPETask::kInofityEvents = IN_ACCESS | IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM |
        IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO;

const uint32_t FPETask::kInotifyEventSize = (sizeof (struct inotify_event));

const uint32_t FPETask::kInotifyEventBuffLen = (1024 * (FPETask::kInotifyEventSize + 16));

// Task object constructor. Create watch directory and initialize variables

FPETask::FPETask(std::string taskNameStr, std::string watchFolder,
        void (*taskFcn)(std::string watchFolder, std::string filenameStr)) :
taskName(taskNameStr), watchFolder(watchFolder), taskProcessFcn(taskFcn) {

    std::cout << this->prefix() << "Watch Folder " << watchFolder << std::endl;

    try {

        if (!fs::exists(watchFolder)) {
            std::cout << this->prefix() << "Watch Folder " << watchFolder << " DOES NOT EXIST." << std::endl;
            if (fs::create_directory(watchFolder)) {
                std::cout << this->prefix() << "Creating Watch Folder " << watchFolder << std::endl;
            }
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << this->prefix() << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::runtime_error &e) {
        std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << this->prefix() << "unknown exception occured" << std::endl;
    }

    // All threads start working

    this->doWork = true;

}

// Copy constructor is private

FPETask::FPETask(const FPETask& orig) {

}

// Move constructor is private

FPETask::FPETask(const FPETask&& orig) {

}

// Destructor is private

FPETask::~FPETask() {
   
}

// Flag thread loops to stop. Folder watcher needs a push because of wait for read().
// Also clean up any resources

void FPETask::stop(void) {

    std::cout << this->prefix() << "Stop task threads." << std::endl;

    try {
        this->doWork = false;
        if (fs::is_empty(watchFolder) || fs::exists(watchFolder)) {
            std::cout << this->prefix() << "Close down folder watcher thread" << std::endl;
        }
        this->destroyWatchTable();
    } catch (const fs::filesystem_error& e) {
        std::cerr << this->prefix() << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::runtime_error &e) {
        std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << this->prefix() << "unknown exception occured" << std::endl;
    }


}

// Prefix string for any task logging.

std::string FPETask::prefix(void) {

    return ("TASK [" + this->taskName + "] ");

}

// Clean up inotifier and its watch variables and clear watch maps.

void FPETask::destroyWatchTable(void) {

    for (auto it = this->watchMap.begin(); it != this->watchMap.end(); ++it) {
        if (inotify_rm_watch(this->fdNotify, it->first) == -1) {
            std::stringstream errStream;
            errStream << "inotify_rm_watch() error:  " << errno;
            throw std::runtime_error(errStream.str());
        } else {
            std::cout << this->prefix() << "Watch[" << it->first << "] removed." <<std::endl;
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

// create a watch for the path.

void FPETask::addWatchPath(std::string pathStr) {

    int watch;

    if ((watch = inotify_add_watch(this->fdNotify, pathStr.c_str(), FPETask::kInofityEvents)) == -1) {
        std::stringstream errStream;
        errStream << "inotify_add_watch() error:  " << errno;
        throw std::runtime_error(errStream.str());
    }

    this->watchMap.insert({watch, pathStr});
    this->revWatchMap.insert({pathStr, watch});

}

// Initialize inotify and  add watches for any existing directory structure.

void FPETask::createWatchTable(void) {

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
            std::cout << this->prefix() << "Directory added [" << pathStr << "] watch = [" << this->revWatchMap[pathStr] << "]" << std::endl;
        }
    }


}

// Add watch for newly added directory

void FPETask::addWatch(struct inotify_event *event) {

    std::string filename = event->name;
    std::string pathStr = this->watchMap[event->wd] + filename + "/";

    this->addWatchPath(pathStr);

    std::cout << this->prefix() << "Directory add [" << pathStr << "] watch = [" << this->revWatchMap[pathStr] << "] File [" << filename << "]" << std::endl;

}

//  Remove watch for deleted or moved directory

void FPETask::removeWatch(struct inotify_event *event) {

    try {

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
        std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
        if (this->watchMap.size() == 1) {
            std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;
        }

    }

}

// Worker thread. Remove path/filename from queue and process.
// Access to fileNames queue controlled by lockguard(mutex) and 
// loop is controlled by atomic bool doWork flag.

void FPETask::worker(void) {

    bool filesToProcess;
    std::string filenamePathStr;
    std::string filenameStr;

    std::cout << this->prefix() << "Worker thread started... " << std::endl;

    while (this->doWork.load()) {

        do {
            do {
                std::lock_guard<std::mutex> guard(this->fileNamesMutex);
                filesToProcess = !this->fileNames.empty();
                if (filesToProcess) {
                    filenamePathStr = this->fileNames.front();
                    this->fileNames.pop();
                    filenameStr = this->fileNames.front();
                    this->fileNames.pop();

                }
            } while (false); // lock guard out of scope so mutex off
            if (filesToProcess) {
                this->taskProcessFcn(filenamePathStr, filenameStr);
            }
        } while (filesToProcess);

        std::this_thread::sleep_for(std::chrono::seconds(1)); // This works better than yield

    }

    std::cout << this->prefix() << "Worker thread stopped. " << std::endl;

}

// Create watch table and loop forever adding/removing watches for directory structure changes
// and also activate task processing for any non-directory file added.

void FPETask::monitor(void) {

    std::uint8_t buffer[FPETask::kInotifyEventBuffLen];

    std::thread::id this_id = std::this_thread::get_id();

    std::cout << this->prefix() << "FPETask Monitor on Thread started ... " << this_id << std::endl;

    this->createWatchTable();

    this->doWork = true;
    this->workerThread.reset(new std::thread(&FPETask::worker, this));

    try {

        while (this->doWork.load()) {

            int readLen, currentPos = 0;

            if ((readLen = read(this->fdNotify, buffer, FPETask::kInotifyEventBuffLen)) == -1) {
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

                currentPos += FPETask::kInotifyEventSize + event->len;

            }
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << this->prefix() << "BOOST file system exception occured: " << e.what() << std::endl;
    } catch (std::runtime_error &e) {
        std::cout << "Caught a runtime_error exception: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << this->prefix() << "unknown exception occured" << std::endl;
    }

    std::cout << this->prefix() << "FPETask Monitor on Thread stopped." << std::endl;

}