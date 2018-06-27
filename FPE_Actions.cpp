#include "HOST.hpp"
/*
 * File:   FPE_Actions.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: FPE_Actions
//
// Description: Generate FPE Task object for task number passed in
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Linux              : Target platform
// Boost              : File system.
//

// =============
// INCLUDE FILES
// =============

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_Actions.hpp"

namespace FPE_Actions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;

    // ===============
    // LOCAL VARIABLES
    // ===============

    // ===============
    // LOCAL FUNCTIONS
    // ===============
    
    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Task action factory
    //

    shared_ptr<TaskAction> createTaskAction(int taskNumber) {

        switch (taskNumber) {
            case 0:
                return (shared_ptr<TaskAction> (new CopyFile()));
                break;
            case 1:
                return (shared_ptr<TaskAction> (new VideoConversion()));
                break;
            case 2:
                return (shared_ptr<TaskAction> (new EmailFile()));
                break;
            case 3:
                return (shared_ptr<TaskAction> (new ZIPFile()));
                break;
            case 4:
                return (shared_ptr<TaskAction> (new RunCommand()));
                break;
            case 5:
                return (shared_ptr<TaskAction> (new ImportCSVFile()));
                break;
        }
        
        return(nullptr);

    }

} // namespace FPE_Actions