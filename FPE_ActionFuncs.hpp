/*
 * File:   FPE_ActionFuncs.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

#ifndef FPE_ACTIONFUNCS_HPP
#define FPE_ACTIONFUNCS_HPP

//
// C++ STL
//

#include <string>
#include <memory>
#include <unordered_map>

//
// Antikythera Classes
//

#include "CTask.hpp"
#include "CLogger.hpp"

// =========
// NAMESPACE
// =========

namespace FPE_ActionFuncs {
    
    // =======
    // IMPORTS
    // =======

    using namespace Antik::Util;
    using namespace Antik::File;

    //
    // Task Action Function Names
    //
    
    constexpr const char *kTaskCopyFile          {"Copy File"};
    constexpr const char *kTaskVideoConversion   {"Video Conversion"};
    constexpr const char *kTaskEmailFile         {"Email Attachment"};
    constexpr const char *kTaskZipFile           {"ZIP Archive"};
    constexpr const char *kTaskRunCommand        {"Run Command"};
    constexpr const char *kTaskImportCSVFile     {"Import CSV File"};
    
    //
    // Create task action object
    //
    
    struct ActFnData {
        std::unordered_map<std::string, std::string> optionsMap;    // Option map
    };

    //
    //  Get task details from taskList table
    //

    std::shared_ptr<CTask::Action> createTaskAction(int taskNumber);
    
    //
    // Action classes
    //
    
    class ActionCopyFile : public CTask::Action {
    public:
        ActionCopyFile(const std::string &taskName) : Action(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionCopyFile() {} ;
    };

    class ActionVideoConversion : public CTask::Action {
    public:
        ActionVideoConversion(const std::string &taskName) : Action(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionVideoConversion() {} ;
    };
    
  class ActionEmailFile : public CTask::Action {
    public:
        ActionEmailFile(const std::string &taskName) : Action(taskName) {}
        virtual void init(void);
        virtual void term(void);
        virtual bool process(const std::string &file);
        virtual ~ActionEmailFile() {} ;
    };
    
  class ActionZIPFile : public CTask::Action {
    public:
        ActionZIPFile(const std::string &taskName) : Action(taskName) {}
        virtual void init(void);
        virtual void term(void);
        virtual bool process(const std::string &file);
        virtual ~ActionZIPFile() {} ;
    };
    
  class ActionRunCommand : public CTask::Action {
    public:
        ActionRunCommand(const std::string &taskName) : Action(taskName) {}
        virtual void init(void);
        virtual void term(void);
        virtual bool process(const std::string &file);
        virtual ~ActionRunCommand() {} ;
    };
    
  class ActionImportCSVFile : public CTask::Action {
    public:
        ActionImportCSVFile(const std::string &taskName) : Action(taskName) {}
        virtual void init(void);
        virtual void term(void);
        virtual bool process(const std::string &file);
        virtual ~ActionImportCSVFile() {} ;
    };

      
} // namespace FPE_ActionFuncs

#endif /* FPE_ACTIONFUNCS_HPP */

