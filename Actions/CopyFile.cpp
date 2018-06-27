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
// BOOST filesystem
//

#include <boost/filesystem.hpp>

namespace FPE_Actions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;
    
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
    // Copy file task action.
    //

    bool CopyFile::process(const string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(file);

        // Destination file path += ("filename path" - "watch folder path")

        fs::path destinationFile(this->m_actionData[kDestinationOption] +
                file.substr((this->m_actionData[kWatchOption]).length()));

        // Construct full destination path if needed

        if (!fs::exists(destinationFile.parent_path())) {
            if (fs::create_directories(destinationFile.parent_path())) {
                 std::cout << "Created :" << destinationFile.parent_path().string() << std::endl;
            } else {
                 std::cerr << "Created failed for :" << destinationFile.parent_path().string() << std::endl;
            }
        }

        // Currently only copy file if it doesn't already exist.

        if (!fs::exists(destinationFile)) {
            std::cout << "COPY FROM [" << sourceFile.string() << "] TO [" << destinationFile.string() << "]" << std::endl;
            fs::copy_file(sourceFile, destinationFile, fs::copy_option::none);
            bSuccess = true;
            if (!this->m_actionData[kDeleteOption].empty()) {
                 std::cout << "Deleting Source ["+sourceFile.string()+"]" << std::endl;
                fs::remove(sourceFile);
            }

        } else {
             std::cout << "Destination already exists : " << destinationFile.string() << std::endl;
        }

        return (bSuccess);

    }
    
} // namespace FPE_Actions