//
// Module: ZipFile
//
// Description: Take passed in file and add it to a ZIP archive.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CZIP, CFile, CPath.
// Linux              : Target platform.
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

#include "CZIP.hpp"
#include "CFile.hpp"
#include "CPath.hpp"

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_Actions.hpp"

//
// Boost format library
//

#include <boost/format.hpp>

namespace FPE_TaskActions {

    // =======
    // IMPORTS
    // =======

    using namespace FPE;
    using namespace Antik::ZIP;
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
    // Add file to ZIP archive.
    //

    bool ZIPFile::process(const std::string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        try {

            // Form source and zips file paths

            CPath sourceFile(file);
            CPath zipFilePath(this->m_actionData[kArchiveOption]);

            // Create path for ZIP archive if needed.

            if (!CFile::exists(zipFilePath.parentPath())) {
                if (CFile::createDirectory(zipFilePath.parentPath())) {
                    std::cout << "Created : " << zipFilePath.parentPath().toString() << std::endl;
                } else {
                    std::cerr << "Created failed for :" << zipFilePath.parentPath().toString() << std::endl;
                }
            }

            // Create archive if doesn't exist

            CZIP zipFile(zipFilePath.toString());

            if (!CFile::exists(zipFilePath)) {
                std::cout << "Creating archive " << zipFilePath.toString() << std::endl;
                zipFile.create();
            }

            // Append file to archive

            zipFile.open();

            bSuccess = zipFile.add(sourceFile.toString(), sourceFile.fileName());
            if (bSuccess) {
                std::cout << "Appended [" << sourceFile.fileName() << "] to archive [" << zipFilePath.toString() << "]" << std::endl;
            }

            zipFile.close();

        } catch (const std::exception & e) {
           std::cerr << this->getName() << " Error: " << e.what() << std::endl;
        }

        return (bSuccess);

    }

} // namespace FPE_TaskActions

