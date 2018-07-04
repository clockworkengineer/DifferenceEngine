#include "HOST.hpp"
/*
 * File:   VideoConversion.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: VideoConversion
//
// Description: Take passed in file and convert it to a new video format 
// (default MPEG4) using handbrake.
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

//
//
// Process wait
//

#include <sys/wait.h>

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

    using namespace Antik::File;

    // ===============
    // LOCAL VARIABLES
    // ===============

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Fork and execute Shell command
    //

    static int forkCommand(char *const argv[]) {

        pid_t pid; // Process id
        int status; // wait status
        int exitStatus = 0; // child exit status

        if ((pid = fork()) < 0) { /* fork a child process           */

            throw system_error(error_code(errno, system_category()), "Error: forking child process failed:");

        } else if (pid == 0) { /* for the child process: */

            // Redirect stdout/stderr

            freopen("/dev/null", "w", stdout);
            freopen("/dev/null", "w", stderr);

            if (execvp(*argv, argv) < 0) { /* execute the command  */
                exit(1);
            }

        } else { // for the parent:

            while (wait(&status) != pid) { /* wait for completion  */
            }

            if (WIFEXITED(status)) { // Set any exit status
                exitStatus = WEXITSTATUS(status);
            }

        }

        return (exitStatus);

    }

    //
    // Run shell command. Split command into argv components before passing onto exec.
    // All heap memory cleaned up when function returns due to unique_pointers.
    //

    static int runShellCommand(const string& shellCommand) {

        int exitStatus = 0;
        int argc = 0;

        vector<char *> argvs;
        unique_ptr<char*> argv;
        unique_ptr<char> command{ new char[shellCommand.length() + 1]};

        // Take a 'C' string copy

        shellCommand.copy(command.get(), shellCommand.length());
        command.get()[shellCommand.length()] = 0; // null terminate

        // Loop through command splitting into substrings for argv

        char *p2 = strtok(command.get(), " ");

        while (p2) {
            argvs.push_back(p2);
            p2 = strtok(0, " ");
        }

        // Allocate argv array and copy vector over

        argv.reset(new char*[argvs.size() + 1]);

        for (char *arg : argvs) {
            argv.get()[argc++] = arg;
        }

        argv.get()[argc] = 0; // Last element null

        // Fork command

        exitStatus = forkCommand(argv.get());

        return (exitStatus);

    }
    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Video file conversion action function. Convert passed in file to MP4 using Handbrake.
    //

    bool VideoConversion::process(const string& file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        try {

            // Form source and destination file paths

            CPath sourceFile(file);
            CPath destinationFile(this->m_actionData[kDestinationOption]);

            destinationFile.join(sourceFile.baseName());

            if (this->m_actionData["extension"].length() > 0) {
                destinationFile.replaceExtension(this->m_actionData["extension"]);
            } else {
                destinationFile.replaceExtension(".mp4");
            }

            // Convert file

            string command = (boost::format(this->m_actionData[kCommandOption]) % sourceFile.toString() % destinationFile.toString()).str();

            cout << "Converting file [" << sourceFile.toString() << "] To [" << destinationFile.toString() << "]" << endl;

            auto result = 0;
            if ((result = runShellCommand(command)) == 0) {
                bSuccess = true;
                cout << "File conversion success." << endl;
                if (!this->m_actionData[kDeleteOption].empty()) {
                    cout << "Deleting Source [" << sourceFile.toString() << "]" << endl;
                    CFile::remove(sourceFile);
                }

            } else {
                cout << "File conversion error: " << to_string(result) << endl;
            }

        } catch (const exception & e) {
           cerr << this->getName() << " Error: " << e.what() << endl;
        }

        return (bSuccess);

    }


} // namespace FPE_TaskActions