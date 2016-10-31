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
 * File:   FPETask.hpp
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:33 PM
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include <exception>
#include <unordered_map>
#include <thread>

// #include "boost/filesystem.hpp"
#define INOTIFY_THREAD_SAFE

#include "inotify-cxx.h"

#ifndef FPETASK_HPP
#define FPETASK_HPP
class FPETask
{
    public:
        FPETask(std::string taskNameStr,
                std::string watchFolderStr,
                std::string destinationFolderStr,
                void (*taskFcn)(std::string srcPathStr, std::string destPathStr, std::string filenameStr));

 
        virtual ~FPETask();

        void monitor(void);
        
        static const long InofityEvents;

    private:
        
        FPETask();
 
        FPETask(const FPETask & orig);

        std::string prefix (void);
        
        void addWatch(InotifyEvent event);

        void removeWatch(InotifyEvent event);

        void createWatchTable(void);

        void destroyWatchTable(void);

        std::string                                     taskName;
        std::string                                     watchFolder;
        std::string                                     destinationFolder;
        Inotify *                                       notify;
        InotifyWatch *                                  watch;
        std::unordered_map<InotifyWatch *, std::string> watchMap;
        void (*taskProcessFcn)(std::string srcPathStr, std::string destPathStr, std::string filenameStr);
        
};
#endif /* FPETASK_HPP */

