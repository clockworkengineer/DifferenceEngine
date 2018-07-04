#include "HOST.hpp"
/*
 * File:   CopyFile.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

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
// Program components.
//

#include "FPE.hpp"
#include "FPE_Actions.hpp"

//
// Antik Classes
//

#include "CFile.hpp"
#include "CPath.hpp"

namespace FPE_TaskActions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

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

    bool CopyFile::process(const string &file) {

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
                    cout << "Created :" << destinationFile.parentPath().toString() << endl;
                } else {
                    cerr << "Created failed for :" << destinationFile.toString() << endl;
                }
            }

            // Currently only copy file if it doesn't already exist.

            if (!CFile::exists(destinationFile)) {
                cout << "COPY FROM [" << sourceFile.toString() << "] TO [" << destinationFile.toString() << "]" << endl;
                CFile::copy(sourceFile, destinationFile);
                bSuccess = true;
                if (!this->m_actionData[kDeleteOption].empty()) {
                    cout << "Deleting Source [" + sourceFile.toString() + "]" << endl;
                    CFile::remove(sourceFile);
                }

            } else {
                cout << "Destination already exists : " << destinationFile.toString() << endl;
            }

        } catch (const CFile::Exception& e) {
            cerr << this->getName() << " Error: " << e.what() << endl;
        }

        return (bSuccess);

    }

} // namespace FPE_TaskActions