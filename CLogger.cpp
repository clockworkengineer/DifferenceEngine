/* 
 * File:   CLogger.cpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on January 6, 2017, 6:37 PM
 * 
 * Copyright 2016.
 * 
 */

// =================
// CLASS DEFINITIONS
// =================

#include "CLogger.hpp"

// Boost date and time libraries definitions

#include <boost/date_time.hpp>

namespace pt = boost::posix_time;

// ====================
// CLASS IMPLEMENTATION
// ====================

// ===========================
// PRIVATE TYPES AND CONSTANTS
// ===========================

CLogger::LogStringsFn CLogger::noOp  = [] (const std::vector<std::string>& outstr) { };

// ===============
// PRIVATE METHODS
// ===============

//
// Get string for current date time
//

const std::string  CLogger::currentDateAndTime(void) {

    return(pt::to_simple_string(pt::second_clock::local_time()));

}

// ==============
// PUBLIC METHODS
// ==============
//

//
// CLogger object constructor. 
//

CLogger::CLogger() {
}

//
// CLogger Destructor
//

CLogger::~CLogger() {
}

//
// Standard cout for string of vectors. All calls to this function from different
// threads are guarded by mutex mOutput (this is static but local to the function).
//

void CLogger::coutstr(const std::vector<std::string>& outstr) {

    static std::mutex mOutput;
    std::lock_guard<std::mutex> locker(mOutput);

    if (!outstr.empty()) {

        for (auto str : outstr) {
            std::cout << str;
        }

        std::cout << std::endl;

    }

}

//
// Standard cerr for string of vectors. All calls to this function from different
// threads are guarded by mutex mError (this is static but local to the function).
//

void  CLogger::cerrstr(const std::vector<std::string>& errstr) {

    static std::mutex mError;
    std::lock_guard<std::mutex> locker(mError);

    if (!errstr.empty()) {

        for (auto str : errstr) {
            std::cerr << str;
        }

        std::cerr << std::endl;

    }

}

//
// Add timestamp to coutstr output
//

void  CLogger::coutstrTimeStamped(const std::vector<std::string>& outstr) {

    if (!outstr.empty()) {
        std::vector<std::string> newstr { "[" + currentDateAndTime() + "]" };
        newstr.insert(newstr.end(), outstr.begin(), outstr.end() );
        coutstr(newstr);
    }

}

//
// Add timestamp to cerrstr output
//

void  CLogger::cerrstrTimeStamped(const std::vector<std::string>& errstr) {

    if (!errstr.empty()) {
        std::vector<std::string> newstr { "[" + currentDateAndTime() + "]" };
        newstr.insert(newstr.end(), errstr.begin(), errstr.end() );
        cerrstr(errstr);
    }
}
