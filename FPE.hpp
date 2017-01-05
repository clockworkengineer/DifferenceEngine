/*
 * File:   FPE.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
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

#ifndef FPE_HPP
#define FPE_HPP

// C++ STL definitions

#include <iostream>
#include <mutex>
#include <system_error>
#include <memory>
#include <fstream>

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Task class

#include "FPE_Task.hpp" 

// Redirect Class

#include "Redirect.hpp" 

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

// Boost file system and date and time libraries definitions

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

#endif /* FPE_HPP */

