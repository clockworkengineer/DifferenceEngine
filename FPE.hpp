/*
 * File:   FPE.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 * 
 * Copyright 2016.
 *
 */

#ifndef FPE_HPP
#define FPE_HPP

// C++ STL definitions

#include <iostream>
#include <mutex>
#include <system_error>
#include <memory>
#include <fstream>

// Task class

#include "CFileTask.hpp" 

// Process command line arguments

#include "FPE_ProcCmdLine.hpp"

// Task Action functions

#include "FPE_ActionFuncs.hpp"

// Redirect Class

#include "Redirect.hpp" 

// Boost file system and date and time libraries definitions

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;

#endif /* FPE_HPP */

