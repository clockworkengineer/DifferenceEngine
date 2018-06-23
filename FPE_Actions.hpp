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

// =========
// NAMESPACE
// =========

namespace FPE_Actions {
    
    // =======
    // IMPORTS
    // =======

    using namespace FPE;
    using namespace Antik::File;

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
            m_actionData.insert(actionData.begin(),actionData.end());
        }

        std::string getName() const {
            return (name);
        }
        
        virtual std::vector<std::string> getParameters() {}

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
    
    class CopyFile : public TaskAction {
    public:
        CopyFile() : TaskAction("Copy File") {}
        void init(void) final  { };
        void term(void) final { };
        bool process(const std::string &file) final ;
        std::vector<std::string> getParameters() final { 
            return (std::vector<std::string>({kDestinationOption}));
        }
        ~CopyFile() {};
    };

    class VideoConversion : public TaskAction {
    public:
        VideoConversion() : TaskAction("Video Conversion") {}
        virtual void init(void) { m_actionData[kCommandOption] = kHandbrakeCommand; };
        virtual void term(void) { };
        virtual bool process(const std::string &file);
        virtual std::vector<std::string> getParameters() { 
            return (std::vector<std::string>({kDestinationOption}));
        }
        virtual ~VideoConversion() {};
    };
    
  class EmailFile : public TaskAction {
    public:
        EmailFile() : TaskAction("Email Attachment") {}
        void init(void) final;
        void term(void) final;
        bool process(const std::string &file) final ;
        std::vector<std::string> getParameters() final { 
            return (std::vector<std::string>({kServerOption, kUserOption, 
                    kPasswordOption, kRecipientOption, kMailBoxOption}));
        }
        ~EmailFile() {};
    };
    
  class ZIPFile : public TaskAction {
    public:
        ZIPFile() : TaskAction("ZIP Archive") {}
        void init(void) final { };
        void term(void) final { };
        bool process(const std::string &file) final;
        std::vector<std::string> getParameters() { 
            return (std::vector<std::string>({kArchiveOption}));
        }
        ~ZIPFile() {};
    };
    
  class RunCommand : public TaskAction {
    public:
        RunCommand() : TaskAction("Run Command") {}
        void init(void) final { };
        void term(void) final { };
        bool process(const std::string &file) final ;
        std::vector<std::string> getParameters() { 
            return (std::vector<std::string>({kCommandOption}));
        }
        ~RunCommand() {};
    };
    
  class ImportCSVFile : public TaskAction {
    public:
        ImportCSVFile() : TaskAction("Import CSV File") {}
        void init(void) final  { };
        void term(void) final { };
        bool process(const std::string &file) final;
        std::vector<std::string> getParameters() final { 
            return (std::vector<std::string>({kServerOption, kUserOption, 
                    kPasswordOption, kDatabaseOption, kCollectionOption}));
        }
        ~ImportCSVFile() {};
    };

} // namespace FPE_Actions
#endif /* FPE_ACTIONFUNCS_HPP */
