/*
 * File:   FPE_Actions.hpp
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

namespace FPE_Actions {
    
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
    // TaskAction class
    //
    
    class TaskAction : public CTask::IAction {
        
    public:

        TaskAction(const std::string &taskName) : name{taskName}
        {
        }

        // Data used by action is a map of string, string.

        void setActionData(std::unordered_map<std::string, std::string> &actionData) {
            m_actionData = actionData;
        }

        std::string getName() const {
            return (name);
        }

        virtual ~TaskAction() {
            
        };

    protected:
        std::string name; // Action name
        std::unordered_map<std::string, std::string> m_actionData; // Map to store action data

    };
           
    //
    //  Get task details from taskList table
    //

    std::shared_ptr<TaskAction> createTaskAction(int taskNumber);
    
    //
    // Action classes
    //
    
    class ActionCopyFile : public TaskAction {
    public:
        ActionCopyFile(const std::string &taskName) : TaskAction(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionCopyFile() {} ;
    };

    class ActionVideoConversion : public TaskAction {
    public:
        ActionVideoConversion(const std::string &taskName) : TaskAction(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionVideoConversion() {} ;
    };
    
  class ActionEmailFile : public TaskAction {
    public:
        ActionEmailFile(const std::string &taskName) : TaskAction(taskName) {}
        virtual void init(void);
        virtual void term(void);
        virtual bool process(const std::string &file);
        virtual ~ActionEmailFile() {} ;
    };
    
  class ActionZIPFile : public TaskAction {
    public:
        ActionZIPFile(const std::string &taskName) : TaskAction(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionZIPFile() {} ;
    };
    
  class ActionRunCommand : public TaskAction {
    public:
        ActionRunCommand(const std::string &taskName) : TaskAction(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionRunCommand() {} ;
    };
    
  class ActionImportCSVFile : public TaskAction {
    public:
        ActionImportCSVFile(const std::string &taskName) : TaskAction(taskName) {}
        virtual void init(void) { };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual ~ActionImportCSVFile() {} ;
    };

} // namespace FPE_Actions

#endif /* FPE_ACTIONFUNCS_HPP */

