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
 * File:   FPECPP.cpp
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include <boost/version.hpp>
#include "boost/filesystem.hpp"
#include "FPETask.hpp"

using namespace boost:: filesystem;

std::string watchFolder("/home/pi/watchstuff/tmp/");
std::string destinationFolder("/home/pi/watchstuff/destination/");

void copyFile(std::string srcPathStr,
              std::string destPathStr,
              std::string filenameStr)
{
    path sourcePath(srcPathStr + filenameStr);
    path destinationPath(destPathStr + srcPathStr.substr((watchFolder).length()));

    try
    {
        if (!exists(destinationPath))
        {
            if (create_directories(destinationPath))
            {
                std::cout << "CREATED :" + destinationPath.string() << std::endl;
            }
            else
            {
                std::cerr << "CREATED FAILED FOR :" + destinationPath.string() << std::endl;
            }
        }

        destinationPath /= filenameStr;

        if (!exists(destinationPath))
        {
            std::cout << "COPY FROM [" << sourcePath.string() << "] TO [" << destinationPath.string() << "]" << std::endl;

            copy_file(sourcePath, destinationPath, copy_option::none);
        }
        else
        {
            std::cout << "DESTINATION ALREADY EXISTS : " + destinationPath.string() << std::endl;
        }
    }
    catch (const boost::filesystem::filesystem_error& e) 
    {
        std::cerr << "BOOST file system exception occured: " << e.what() << std::endl;
    }
    catch (std::exception & e)
    {
        std::cerr << "STL exception occured: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "unknown exception occured" << std::endl;
    }
}

int main(void)
{
    std::cout << "FPE Running" << std::endl;
    
    std::cout << "Using Boost " << BOOST_VERSION / 100000 << "."    // major version
              << BOOST_VERSION / 100 % 1000 << "."                  // minor version
              << BOOST_VERSION % 100                                // patch level
              << std::endl;

    FPETask task(std::string("File Copy"), watchFolder, destinationFolder, copyFile);

    std::thread taskThread(&FPETask::monitor, &task);
    
    taskThread.join();
}

