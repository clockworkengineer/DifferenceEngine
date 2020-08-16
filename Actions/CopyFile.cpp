//
// Module: CopyFile
//
// Description: Copy passed file to destination directory keeping the 
// sources directory structure.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CFile, CPath.
// Linux              : Target platform
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>

//
// Antik Classes
//

#include "CFile.hpp"
#include "CPath.hpp"
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
    using namespace Antik::File;

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
    // Copy file task action.
    //

    bool CopyFile::process(const std::string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        try {

            // Form source and destination file paths

            CPath sourceFile(file);

            // Destination file path += ("filename path" - "watch folder path")

            CPath destinationFile { this->m_actionData[kDestinationOption] };
            destinationFile.join(file.substr((this->m_actionData[kWatchOption]).length()));

            // Construct full destination path if needed

            if (!CFile::exists(destinationFile.parentPath())) {
                if (CFile::createDirectory(destinationFile.parentPath())) {
                    std::cout << "Created :" << destinationFile.parentPath().toString() << std::endl;
                } else {
                    std::cerr << "Created failed for :" << destinationFile.toString() << std::endl;
                }
            }

            // Currently only copy file if it doesn't already exist.

            if (!CFile::exists(destinationFile)) {
                std::cout << "COPY FROM [" << sourceFile.toString() << "] TO [" << destinationFile.toString() << "]" << std::endl;
                CFile::copy(sourceFile, destinationFile);
                bSuccess = true;
                if (!this->m_actionData[kDeleteOption].empty()) {
                    std::cout << "Deleting Source [" + sourceFile.toString() + "]" << std::endl;
                    CFile::remove(sourceFile);
                }

            } else {
                std::cout << "Destination already exists : " << destinationFile.toString() << std::endl;
            }

        } catch (const CFile::Exception& e) {
            std::cerr << this->getName() << " Error: " << e.what() << std::endl;
        }

        return (bSuccess);

    }

} // namespace FPE_TaskActions