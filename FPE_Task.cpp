#include "HOST.hpp"
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
// PRIVATE METHODS
//

//
// Prefix string for any task logging.
//

std::string FPE_Task::prefix(void) {

    return ("[TASK " + this->taskName + "] ");

}

//
// PUBLIC METHODS
//

//
// Task object constructor. 
//

FPE_Task::FPE_Task(const std::string& taskName,
        const std::string& watchFolder,
        TaskActionFcn taskActFcn,
        std::shared_ptr<void> fnData,
        int watchDepth,
        std::shared_ptr<TaskOptions> options) : taskName{taskName}, taskActFcn{taskActFcn}, fnData{fnData}
{

    // ASSERT if passed parameters invalid

    assert(taskName.length() != 0); // Length == 0
    assert(watchFolder.length() != 0); // Length == 0
    assert(watchDepth >= -1); // < -1
    assert(taskActFcn != nullptr); // nullptr
    assert(fnData != nullptr); // nullptr

    // If options passed then setup trace functions and  kill count

    if (options) {
        if (options->coutstr) {
            this->coutstr = options->coutstr;
        }
        if (options->cerrstr) {
            this->cerrstr = options->cerrstr;
        }
        this->killCount = options->killCount;
    }

    // Create IApprise watcher object. Use same cout/cerr functions as Task.

    this->watcherOptions.reset(new IAppriseOptions{nullptr, this->coutstr, this->cerrstr});
    this->watcher.reset(new IApprise{watchFolder, watchDepth, watcherOptions});

    // Create IApprise object thread and start to watch

    this->watcherThread.reset(new std::thread(&IApprise::watch, this->watcher));

}

//
// Destructor
//

FPE_Task::~FPE_Task() {

    this->coutstr({this->prefix(), "FPE_Task DESTRUCTOR CALLED."});

}

//
// Check whether termination of FPE_Task was the result of any thrown exception
//

std::exception_ptr FPE_Task::getThrownException(void) {

    return (this->thrownException);

}

//
// Flag watcher and task loops to stop.
//

void FPE_Task::stop(void) {

    this->coutstr({this->prefix(), "Stop task."});
    this->watcher->stop();

}

//
// Loop calling the task action function for each add file event.
//

void FPE_Task::monitor(void) {

    try {

        this->coutstr({this->prefix(), "FPE_Task monitor started."});

        // Loop until watcher stopped

        while (this->watcher->stillWatching()) {

            IAppriseEvent evt;

            this->watcher->getEvent(evt);

            if ((evt.id == Event_add) && !evt.message.empty()) {

                this->taskActFcn(evt.message, this->fnData);

                if ((this->killCount != 0) && (--(this->killCount) == 0)) {
                    this->coutstr({this->prefix(), "FPE_Task kill count reached."});
                    break;
                }

            } else if ((evt.id == Event_error) && !evt.message.empty()) {
                this->coutstr({evt.message});
            }

        }

        // Pass any IApprise exceptions up chain

        if (this->watcher->getThrownException()) {
            this->thrownException = this->watcher->getThrownException();
        }

    } catch (...) {
        // Pass any FPE_Task thrown exceptions up chain
        this->thrownException = std::current_exception();
    }

    // IApprise still flagged as running so close down 

    if (this->watcher->stillWatching()) {
        this->watcher->stop();
    }

    // Wait for IApprise thread
    
    this->watcherThread->join();

    this->coutstr({this->prefix(), "FPE_Task monitor on stopped."});

}