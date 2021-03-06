//
// Module: FPE_TaskAction
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

namespace FPE_TaskActions {

    // =======
    // IMPORTS
    // =======

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

    std::shared_ptr<TaskAction> TaskAction::create(int taskNumber) {

        switch (taskNumber) {
            case 0:
                return (std::shared_ptr<TaskAction> (new CopyFile()));
                break;
            case 1:
                return (std::shared_ptr<TaskAction> (new VideoConversion()));
                break;
            case 2:
                return (std::shared_ptr<TaskAction> (new EmailFile()));
                break;
            case 3:
                return (std::shared_ptr<TaskAction> (new ZIPFile()));
                break;
            case 4:
                return (std::shared_ptr<TaskAction> (new RunCommand()));
                break;
            case 5:
                return (std::shared_ptr<TaskAction> (new ImportCSVFile()));
                break;
        }
        
        return(nullptr);

    }

} // namespace FPE_TaskActions