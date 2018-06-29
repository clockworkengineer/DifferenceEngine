#include "HOST.hpp"
/*
 * File:   ZipFile.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

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
// Boost              : File system.
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

#include "CZIP.hpp"
#include "CFile.hpp"
#include "CPath.hpp"

//
// Boost format library
//

#include <boost/format.hpp>

namespace FPE_TaskActions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

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

    bool ZIPFile::process(const string &file) {

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
                    cout << "Created : " << zipFilePath.parentPath().toString() << endl;
                } else {
                    cerr << "Created failed for :" << zipFilePath.parentPath().toString() << endl;
                }
            }

            // Create archive if doesn't exist

            CZIP zipFile(zipFilePath.toString());

            if (!CFile::exists(zipFilePath)) {
                cout << "Creating archive " << zipFilePath.toString() << endl;
                zipFile.create();
            }

            // Append file to archive

            zipFile.open();

            bSuccess = zipFile.add(sourceFile.toString(), sourceFile.fileName());
            if (bSuccess) {
                cout << "Appended [" << sourceFile.fileName() << "] to archive [" << zipFilePath.toString() << "]" << endl;
            }

            zipFile.close();

        } catch (const exception & e) {
           cerr << this->getName() << " Error: " << e.what() << endl;
        }

        return (bSuccess);

    }

} // namespace FPE_Actions

