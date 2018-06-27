/*
 * File:   FPE_TaskAction.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

#ifndef FPE_TASKACTION_HPP
#define FPE_TASKACTION_HPP

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

// =========
// NAMESPACE
// =========

namespace FPE_TaskActions {

    //
    // TaskAction class
    //

    class TaskAction : public Antik::File::CTask::IAction {
    public:
        
        static std::shared_ptr<TaskAction> create(int taskNumber);

        TaskAction(const std::string &taskName) : name{taskName}
        {
        }

        virtual ~TaskAction() {
        };

        // Data used by action is a unordered_map<string, string>.

        void setActionData(std::unordered_map<std::string, std::string> &actionData) {
            m_actionData.insert(actionData.begin(), actionData.end());
        }

        std::string getName() const {
            return (name);
        }

        virtual std::vector<std::string> getParameters() = 0;

    protected:
        std::string name; // Action name
        std::unordered_map<std::string, std::string> m_actionData; // Map to store action data

    };


} // namespace FPE_Actions
#endif /* FPE_TASKACTION_HPP */
