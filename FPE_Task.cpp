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
// Prefix string for any task logging.
//

std::string FPE_Task::prefix(void) {

    return ("TASK [" + this->taskName + "] ");

}

//
// PUBLIC METHODS
//

//
// Task object constructor. 
//

FPE_Task::FPE_Task(std::string taskName, std::string watchFolder, TaskActionFcn taskActFcn, std::shared_ptr<void> fnData,
        int maxWatchDepth, std::shared_ptr<TaskOptions> taskOptions) :
        taskName{taskName}, taskActFcn{taskActFcn}, fnData{fnData}, taskOptions{taskOptions}
{

    // ASSERT if passed parameters invalid

    assert(taskName.length() != 0); // Length == 0
    assert(watchFolder.length() != 0); // Length == 0
    assert(maxWatchDepth >= -1); // < -1
    assert(taskActFcn != nullptr); // nullptr
    assert(fnData != nullptr); // nullptr

    // No task option passed in so setup default. THIS NEEDS TO BE SETUP FIRST FOR COUTSTR/CERRSTR.

    if (this->taskOptions == nullptr) {
        this->taskOptions.reset(new TaskOptions{0, nullptr, nullptr});
    }
    
    // Create IApprise watcher object. Use same stdout/stderr functions as Task.

    this->watchOpt.reset(new IAppriseOptions{nullptr, this->taskOptions->coutstr, this->taskOptions->cerrstr});

    this->watcher.reset(new IApprise{watchFolder, maxWatchDepth, watchOpt});

    // Create IApprise object thread and start to watch

    this->watcherThread.reset(new std::thread(&IApprise::watch, this->watcher));

}

//
// Flag watcher and task loops to stop.
//

void FPE_Task::stop(void) {

    coutstr({this->prefix(), "Stop task."});
    
    this->watcher->stop();

}

//
// Loop calling the task action function for each add file event.
//

void FPE_Task::monitor(void) {

    coutstr({this->prefix(), "FPE_Task monitor started."});

    // Loop until watcher stopped
    
    while (this->watcher->stillWatching()) {

        IAppriseEvent evt;

        this->watcher->getEvent(evt);

        if ((evt.id == Event_add) && !evt.message.empty()) {

            this->taskActFcn(evt.message, this->fnData);

            if ((this->taskOptions->killCount != 0) && (--(this->taskOptions->killCount) == 0)) {
                coutstr({this->prefix(), "FPE_Task kill count reached."});
                this->watcher->stop();
            }

        } else if ((evt.id == Event_error) && !evt.message.empty()) {
            coutstr({evt.message});
        }

    }

    this->watcherThread->join(); 

    coutstr({this->prefix(), "FPE_Task monitor on stopped."});

}