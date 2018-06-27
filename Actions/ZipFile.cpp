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
// Description:
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CZIP
// Linux              : Target platform
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

//
// Boost file system, format library
//

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace FPE_Actions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;
    
    using namespace Antik::ZIP;
    
    namespace fs = boost::filesystem;

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

        // Form source and zips file paths

        fs::path sourceFile(file);
        fs::path zipFilePath(this->m_actionData[kArchiveOption]);

        // Create path for ZIP archive if needed.

        if (!fs::exists(zipFilePath.parent_path())) {
            if (fs::create_directories(zipFilePath.parent_path())) {
                 std::cout << "Created : "<< zipFilePath.parent_path().string() << std::endl;
            } else {
                 std::cerr << "Created failed for :" << zipFilePath.parent_path().string() << std::endl;
            }
        }

        // Create archive if doesn't exist

        CZIP zipFile(zipFilePath.string());

        if (!fs::exists(zipFilePath)) {
            std::cout << "Creating archive " << zipFilePath.string() << std::endl;
            zipFile.create();
        }

        // Append file to archive

        zipFile.open();

        bSuccess = zipFile.add(sourceFile.string(), sourceFile.filename().string());
        if (bSuccess) {
             std::cout << "Appended [" << sourceFile.filename().string() << "] to archive [" << zipFilePath.string() << "]" << std::endl;
        }

        zipFile.close();

        return (bSuccess);

    }

} // namespace FPE_Actions

