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

#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <boost/filesystem.hpp>
#include "FPETask.hpp"

using namespace boost::filesystem;

const long FPETask::InofityEvents = IN_ISDIR | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO;

FPETask::FPETask(std::string taskNameStr, std::string watchFolderStr, std::string destinationFolderStr,
        void (*taskFcn)(std::string srcPathStr, std::string destPathStr, std::string filenameStr)) : taskProcessFcn(taskFcn), taskName(taskNameStr), watchFolder(watchFolderStr), destinationFolder(destinationFolderStr) {

    std::cout << this->prefix() << "Watch Folder " << watchFolderStr << std::endl;
    std::cout << this->prefix() << "Destination Folder " << destinationFolderStr << std::endl;

    try {

        if (!exists(watchFolderStr)) {
            std::cout << this->prefix() << "Watch Folder " << watchFolderStr << " DOES NOT EXIST." << std::endl;
            if (create_directory(watchFolderStr)) {
                std::cout << this->prefix() << "Creating Watch Folder " << watchFolderStr << std::endl;
            }
        }

        if (!exists(destinationFolderStr)) {
            std::cout << this->prefix() << "Destination Folder " << destinationFolderStr << " DOES NOT EXIST." << std::endl;
            if (create_directory(destinationFolderStr)) {
                std::cout << this->prefix() << "Creating Destination Folder " << destinationFolderStr << std::endl;
            }
        }

    } catch (const boost::filesystem::filesystem_error& e) {
        std::cerr << this->prefix() << "BOOST file system exception occured: " << e.what() << std::endl;
    }

}

FPETask::FPETask(const FPETask& orig) {

}

FPETask::~FPETask() {

    this->destroyWatchTable();

}

std::string FPETask::prefix (void) {
    
    return ("TASK [" + this->taskName + "] ");
    
}

void FPETask::destroyWatchTable(void) {

    this->notify->Close();
    this->watchMap.clear();
}

void FPETask::createWatchTable(void) {

    this->notify = new Inotify();

    this->watch = new InotifyWatch(this->watchFolder, FPETask::InofityEvents);
    this->notify->Add(*(this->watch));
    this->watchMap.insert({this->watch, this->watchFolder});

    for (recursive_directory_iterator i(this->watchFolder), end; i != end; ++i) {
        if (is_directory(i->path())) {
            std::string pathStr = i->path().string() + "/";
            if (exists(path(pathStr))) {
                InotifyWatch *watch = new InotifyWatch(pathStr, FPETask::InofityEvents);
                this->notify->Add(watch);
                this->watchMap.insert({watch, pathStr});
            }
            std::cout << this->prefix() << "Directory added [" << pathStr << "] watch = [" << watch << "]" << std::endl;
        }
    }

}

void FPETask::addWatch(InotifyEvent event) {

    std::string filename = event.GetName();
    std::string pathStr = this->watchMap[event.GetWatch()] + filename + "/";

    InotifyWatch *watch = new InotifyWatch(pathStr, FPETask::InofityEvents);

    std::cout << this->prefix() << "Directory add [" << pathStr << "] watch = [" << watch << "] File [" << filename << "]" << std::endl;

    this->notify->Add(watch);
    this->watchMap.insert({watch, pathStr});


}

void FPETask::removeWatch(InotifyEvent event) {

    try {

        std::string filename = event.GetName();
        std::string pathStr = this->watchMap[event.GetWatch()];
        InotifyWatch *watch = event.GetWatch();

        if (filename.compare("") != 0) {
            pathStr += filename + "/";
        }

        for (auto& x : this->watchMap) {
            if (pathStr.compare(x.second) == 0) {
                watch = x.first;
                break;
            }
        }

        if (watch) {
            std::cout << this->prefix() << "Directory remove [" << pathStr << "] watch = [" << watch << "] File [" << filename << "]" << std::endl;
            this->watchMap.erase(watch);
            this->notify->Remove(watch);
        } else {
            std::cerr << this->prefix() << "Directory remove failed [" << pathStr << "] File [" << filename << "]" << std::endl;
        }

        if (this->watchMap.size()==1) { 
            std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;
        }
        
    } catch (InotifyException &e) {
        if (this->watchMap.size()==1) { 
            std::cout << this->prefix() << "WATCH TABLE CLEARED." << std::endl;
        }
        std::cerr << this->prefix() << "Inotify exception occured: " << e.GetMessage() << " errno: " << e.GetErrorNumber() << std::endl;
    }

}

void FPETask::monitor(void) {

    std::thread::id this_id = std::this_thread::get_id();

    std::cout << this->prefix() << "FPETask Monitor on Thread " << this_id << std::endl;

    this->createWatchTable();

    try {

        std::cout << this->prefix() << "Watching directory " << this->watchFolder << std::endl << std::endl;

        for (;;) {

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
                            this->taskProcessFcn(this->watchMap[event.GetWatch()], this->destinationFolder, event.GetName());
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

}