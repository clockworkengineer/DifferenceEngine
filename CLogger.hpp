/* 
 * File:   CLogger.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on January 6, 2017, 6:37 PM
 * 
 * Copyright 2016.
 * 
 */

#ifndef CLOGGER_HPP
#define CLOGGER_HPP

//
// C++ STL definitions
//

#include <vector>
#include <functional>
#include <mutex>

// ================
// CLASS DEFINITION
// ================

class CLogger {
    
public:
    
    // ==========================
    // PUBLIC TYPES AND CONSTANTS
    // ==========================

    typedef  std::function<void (const std::vector<std::string>& )> LogStringsFn;
    
    static LogStringsFn noOp;
    
    // ============
    // CONSTRUCTORS
    // ============
  
    CLogger();
    
    // ==========
    // DESTRUCTOR
    // ==========
   
    virtual ~CLogger();
    
    // ==============
    // PUBLIC METHODS
    // ==============

    static void coutstr(const std::vector<std::string>& outstr);
    static void cerrstr(const std::vector<std::string>& errstr);
    static void coutstrTimeStamped(const std::vector<std::string>& outstr);
    static void cerrstrTimeStamped(const std::vector<std::string>& errstr);
 
    // ===========================
    // PRIVATE TYPES AND CONSTANTS
    // ===========================
  
    // =====================
    // DISABLED CONSTRUCTORS
    // =====================
 
    CLogger(const CLogger& orig) = delete;
    CLogger(const CLogger&& orig) = delete;
    
private:
    
    // ===============
    // PRIVATE METHODS
    // ===============

    static const std::string currentDateAndTime(void);
 
};
#endif /* CLOGGER_HPP */

