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

// Inotify events to recieve

const uint32_t FPETask::InofityEvents = IN_ACCESS | IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM |
        IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO;

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

// Destructor just releases watch table related resources /  flag threads to stop working.

FPETask::~FPETask() {

    this->doWork = false;
    this->destroyWatchTable();

}

// Flag thread loops to stop. Folder watcher needs a push because of wait for event.

void FPETask::stop(void) {


    std::cout << this->prefix() << "Stop task threads." << std::endl;

    this->doWork = false;
    if (fs::is_empty(watchFolder) || fs::exists(watchFolder)) {
        std::cout << this->prefix() << "Close down folder watcher thread" << std::endl;
    }

}

// Prefix string for any task logging.

std::string FPETask::prefix(void) {

    return ("TASK [" + this->taskName + "] ");

}

// Clean up inotifier and its watch variables and clear watch maps.

void FPETask::destroyWatchTable(void) {

    this->notify->Close();
    this->watchMap.clear();
    this->revWatchMap.clear();

}

// Create inotifer and  add watches for any existing directory structure.

void FPETask::createWatchTable(void) {

    InotifyWatch *watch;

    this->notify.reset(new Inotify());

    watch = new InotifyWatch(this->watchFolder, FPETask::InofityEvents);

    this->notify->Add(watch);
    this->watchMap.insert({watch, this->watchFolder});
    this->revWatchMap.insert({this->watchFolder, watch});

    for (fs::recursive_directory_iterator i(this->watchFolder), end; i != end; ++i) {
        if (fs::is_directory(i->path())) {
            std::string pathStr = i->path().string() + "/";
            if (fs::exists(fs::path(pathStr))) {
                watch = new InotifyWatch(pathStr, FPETask::InofityEvents);
                this->notify->Add(watch);
                this->watchMap.insert({watch, pathStr});
                this->revWatchMap.insert({pathStr, watch});
            }
            std::cout << this->prefix() << "Directory added [" << pathStr << "] watch = [" << watch << "]" << std::endl;
        }
    }

}

// Add watch for newly added directory

void FPETask::addWatch(InotifyEvent event) {

    std::string filename = event.GetName();
    std::string pathStr = this->watchMap[event.GetWatch()] + filename + "/";

    InotifyWatch *watch = new InotifyWatch(pathStr, FPETask::InofityEvents);

    std::cout << this->prefix() << "Directory add [" << pathStr << "] watch = [" << watch << "] File [" << filename << "]" << std::endl;

    this->notify->Add(watch);
    this->watchMap.insert({watch, pathStr});
    this->revWatchMap.insert({pathStr, watch});


}

//  Remove watch for deleted or moved directory

void FPETask::removeWatch(InotifyEvent event) {

    try {

        std::string filename = event.GetName();
        std::string pathStr = this->watchMap[event.GetWatch()];
        InotifyWatch *watch = nullptr;

        if (filename.compare("") != 0) {
            pathStr += filename + "/";
        }

        watch = this->revWatchMap[pathStr];
        if (watch) {
            std::cout << this->prefix() << "Directory remove [" << pathStr << "] watch = [" << watch << "] File [" << filename << "]" << std::endl;
            this->watchMap.erase(watch);
            this->revWatchMap.erase(pathStr);
            this->notify->Remove(watch);
        } else {
            std::cerr << this->prefix() << "Directory remove failed [" << pathStr << "] File [" << filename << "]" << std::endl;
        }

        if (this->watchMap.size() == 1) {
            std::cout << this->prefix() << "Watch Count = " << this->notify->GetWatchCount() << std::endl;
            std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;
        }

    } catch (InotifyException &e) {
        if (this->watchMap.size() == 1) {
            std::cout << this->prefix() << "Watch Count = " << this->notify->GetWatchCount() << std::endl;
            std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;

        }

        std::cerr << this->prefix() << "Inotify exception occured: " << e.GetMessage() << " errno: " << e.GetErrorNumber() << std::endl;
    }

}

// Worker thread. Remove path/filename from queue and process.
// Access to fileNames queue conrolled by mutex and loop is controlled
// by atomic bool doWork flag.

void FPETask::worker(void) {

    try {

        std::cout << this->prefix() << "Worker thread started... " << std::endl;

        while (this->doWork.load()) {

            this->fileNamesMutex.lock();
            while (!this->fileNames.empty()) {
                std::string filenamePathStr(this->fileNames.front());
                this->fileNames.pop();
                std::string filenameStr(this->fileNames.front());
                this->fileNames.pop();
                this->fileNamesMutex.unlock();
                this->taskProcessFcn(filenamePathStr, filenameStr);
                this->fileNamesMutex.lock();
            }
            this->fileNamesMutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (std::exception &e) {
        this->fileNamesMutex.unlock();
        std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        this->fileNamesMutex.unlock();
        std::cerr << this->prefix() << "unknown exception occured" << std::endl;
    }

    std::cout << this->prefix() << "Worker thread stopped. " << std::endl;

}

// Create watch table and loop forever adding/removing watches for directory structure changes
// and also activate task processing for any non-directory file added.

void FPETask::monitor(void) {

    std::thread::id this_id = std::this_thread::get_id();

    std::cout << this->prefix() << "FPETask Monitor on Thread started ... " << this_id << std::endl;

    this->createWatchTable();

    this->doWork = true;
    this->workerThread.reset(new std::thread(&FPETask::worker, this));

    try {

        std::cout << this->prefix() << "Watching directory " << this->watchFolder << std::endl;

        while (this->doWork.load()) {

            this->notify->WaitForEvents();

            size_t count = this->notify->GetEventCount();

            while (count > 0) {

                InotifyEvent event;

                if (this->notify->GetEvent(&event)) {

                    switch (event.GetMask()) {

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
                            this->fileNames.push(this->watchMap[event.GetWatch()]);
                            this->fileNames.push(event.GetName());
                            break;

                    }

                }

                count--;
            }
        }
    } catch (InotifyException &e) {
        std::cerr << this->prefix() << "Inotify exception occured: " << e.GetMessage() << std::endl;
    } catch (std::exception &e) {
        std::cerr << this->prefix() << "STL exception occured: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << this->prefix() << "unknown exception occured" << std::endl;
    }

    std::cout << this->prefix() << "FPETask Monitor on Thread stopped." << std::endl;

}