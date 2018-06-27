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

        void init(void) final {
        };

        void term(void) final {
        };
        
        bool process(const std::string &file) final;

        std::vector<std::string> getParameters() final {
            return (std::vector<std::string>({FPE::kDestinationOption}));
        }

        ~CopyFile() {
        };
    };

    class VideoConversion : public TaskAction {
    public:

        VideoConversion() : TaskAction("Video Conversion") {
        }

        virtual void init(void) {
            // Allows the override of built-in command
            if (m_actionData.find(FPE::kCommandOption)==m_actionData.end()) {
                m_actionData[FPE::kCommandOption] =
                    "/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"";
            }
        };

        virtual void term(void) {
        };
        
        virtual bool process(const std::string &file);

        virtual std::vector<std::string> getParameters() {
            return (std::vector<std::string>({FPE::kDestinationOption}));
        }

        ~VideoConversion() {
        };
    };

    class EmailFile : public TaskAction {
    public:

        EmailFile() : TaskAction("Email Attachment") {
        }
        
        void init(void) final;
        void term(void) final;
        
        bool process(const std::string &file) final;

        std::vector<std::string> getParameters() final {
            return (std::vector<std::string>({FPE::kServerOption, FPE::kUserOption,
                FPE::kPasswordOption, FPE::kRecipientOption, FPE::kMailBoxOption}));
        }

        ~EmailFile() {
        };
    };

    class ZIPFile : public TaskAction {
    public:

        ZIPFile() : TaskAction("ZIP Archive") {
        }

        void init(void) final {
        };

        void term(void) final {
        };
        
        bool process(const std::string &file) final;

        std::vector<std::string> getParameters() {
            return (std::vector<std::string>({FPE::kArchiveOption}));
        }

        ~ZIPFile() {
        };
    };

    class RunCommand : public TaskAction {
    public:

        RunCommand() : TaskAction("Run Command") {
        }

        void init(void) final {
        };

        void term(void) final {
        };
        
        bool process(const std::string &file) final;

        std::vector<std::string> getParameters() {
            return (std::vector<std::string>({FPE::kCommandOption}));
        }

        ~RunCommand() {
        };
    };

    class ImportCSVFile : public TaskAction {
    public:

        ImportCSVFile() : TaskAction("Import CSV File") {
        }

        void init(void) final {
        };

        void term(void) final {
        };
        
        bool process(const std::string &file) final;

        std::vector<std::string> getParameters() final {
            return (std::vector<std::string>({FPE::kServerOption, FPE::kUserOption,
                FPE::kPasswordOption, FPE::kDatabaseOption, FPE::kCollectionOption}));
        }

        ~ImportCSVFile() {
        };
    };

} // namespace FPE_Actions
#endif /* FPE_ACTIONS_HPP */


