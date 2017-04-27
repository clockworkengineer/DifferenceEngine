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

//
// C++ STL definitions
//

#include <system_error>
#include <memory>

//
// Antikythera Classes
//

#include "CZIP.hpp"
#include "CTask.hpp" 
#include "CRedirect.hpp" 
#include "CLogger.hpp"
#include "CSMTP.hpp"
#include "CIMAP.hpp"

using namespace Antik::IMAP;
using namespace Antik::File;
using namespace Antik::SMTP;
using namespace Antik::Util;
using namespace Antik::ZIP;
        
//
// Process command line arguments
//

#include "FPE_ProcCmdLine.hpp"

//
// Task Action functions
//

#include "FPE_ActionFuncs.hpp"


//
// Boost file system library definitions
//

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#endif /* FPE_HPP */

