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

#ifndef FPE_ACTIONS_HPP
#define FPE_ACTIONS_HPP

//
// C++ STL
//

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

//
// Antik Classes
//

#include "CTask.hpp"
#include "FPE_TaskAction.hpp"

// =========
// NAMESPACE
// =========

namespace FPE_TaskActions {

    //
    // FPE Action classes
    //

    class CopyFile : public TaskAction {
    public:

        CopyFile() : TaskAction("Copy File") {
        }

        void init(void) override {
        };

        void term(void) override {
        };
        
        bool process(const std::string &file) override;

        std::vector<std::string> getParameters() override {
            return (std::vector<std::string>({FPE::kDestinationOption}));
        }

        ~CopyFile() override {
        };
    };

    class VideoConversion : public TaskAction {
    public:

        VideoConversion() : TaskAction("Video Conversion") {
        }

        void init(void) override {
            // Allows the  of built-in command
            if (m_actionData.find(FPE::kCommandOption)==m_actionData.end()) {
                m_actionData[FPE::kCommandOption] =
                    "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";
            }
        };

        void term(void) override {
        };
        
        bool process(const std::string &file) override;

        std::vector<std::string> getParameters() override {
            return (std::vector<std::string>({FPE::kDestinationOption}));
        }

        ~VideoConversion() override {
        };
    };

    class EmailFile : public TaskAction {
    public:

        EmailFile() : TaskAction("Email Attachment") {
        }
        
        void init(void) override;
        void term(void) override;
        
        bool process(const std::string &file) override;

        std::vector<std::string> getParameters() override {
            return (std::vector<std::string>({FPE::kServerOption, FPE::kUserOption,
                FPE::kPasswordOption, FPE::kRecipientOption, FPE::kMailBoxOption}));
        }

        ~EmailFile() override {
        };
    };

    class ZIPFile : public TaskAction {
    public:

        ZIPFile() : TaskAction("ZIP Archive") {
        }

        void init(void) override {
        };

        void term(void) override {
        };
        
        bool process(const std::string &file) override;

        std::vector<std::string> getParameters() override {
            return (std::vector<std::string>({FPE::kArchiveOption}));
        }

        ~ZIPFile() override {
        };
    };

    class RunCommand : public TaskAction {
    public:

        RunCommand() : TaskAction("Run Command") {
        }

        void init(void) override {
        };

        void term(void) override {
        };
        
        bool process(const std::string &file) override;

        std::vector<std::string> getParameters() override {
            return (std::vector<std::string>({FPE::kCommandOption}));
        }

        ~RunCommand() override {
        };
    };

    class ImportCSVFile : public TaskAction {
    public:

        ImportCSVFile() : TaskAction("Import CSV File") {
        }

        void init(void) override {
        };

        void term(void) override {
        };
        
        bool process(const std::string &file) override;

        std::vector<std::string> getParameters() override {
            return (std::vector<std::string>({FPE::kServerOption, FPE::kUserOption,
                FPE::kPasswordOption, FPE::kDatabaseOption, FPE::kCollectionOption}));
        }

        ~ImportCSVFile() override {
        };
    };

} // namespace FPE_TaskActions
#endif /* FPE_ACTIONS_HPP */


