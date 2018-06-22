#include "HOST.hpp"
/*
 * File:   FPE_ActionFuncs.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: FPE_ActionFuncs
//
// Description: The task action functions used by the FPE and their support functions.
// 
// Currently five actions are provided.
//
// 1) File copy
// 2) Video file conversion (using handbrake)
// 3) Run shell 
// 4) Email file as attachment (if the server is IMAP then mail message is appended to a mailbox)
// 5) Append file to a ZIP archive
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CTask, CSMTP, CIMAP, CIMAPParse, CZIP, CMIME
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
#include <system_error>
#include <algorithm>    // copy
#include <iterator>     // ostream_operator

//
// Program components.
//

#include "FPE.hpp"
#include "FPE_ActionFuncs.hpp"

//
// Antikythera Classes
//

#include "CZIP.hpp"
#include "CTask.hpp"  
#include "CSMTP.hpp"
#include "CIMAP.hpp"
#include "CIMAPParse.hpp"
#include "CMIME.hpp"

//
//
// Process wait
//

#include <sys/wait.h>

//
// Boost file system, format and tokenizer library
//

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

//
// MongoDB C++ Driver
// Note: C++ Driver not easy to install so add define
//

#ifdef MONGO_DRIVER_INSTALLED
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#endif // MONGO_DRIVER_INSTALLED

namespace fs = boost::filesystem;

namespace FPE_ActionFuncs {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;

    using namespace Antik::IMAP;
    using namespace Antik::File;
    using namespace Antik::SMTP;
    using namespace Antik::Util;
    using namespace Antik::ZIP;

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
                continue;
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

    static int runShellCommand(const string& shellCommandStr) {

        int exitStatus = 0;
        int argc = 0;

        vector<char *> argvs;
        unique_ptr<char*> argv;
        unique_ptr<char> commandStr{ new char[shellCommandStr.length() + 1]};

        // Take a 'C' string copy

        shellCommandStr.copy(commandStr.get(), shellCommandStr.length());
        commandStr.get()[shellCommandStr.length()] = 0; // null terminate

        // Loop through command splitting into substrings for argv

        char *p2 = strtok(commandStr.get(), " ");

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
    //  Create task action object
    //

    std::shared_ptr<CTask::Action> createTaskAction(int taskNumber) {

        switch (taskNumber) {
            case 0:
                return (std::shared_ptr<CTask::Action> (new ActionCopyFile(kTaskCopyFile)));
                break;
            case 1:
                return (std::shared_ptr<CTask::Action> (new ActionCopyFile(kTaskVideoConversion)));
                break;
            case 2:
                return (std::shared_ptr<CTask::Action> (new ActionCopyFile(kTaskEmailFile)));
                break;
            case 3:
                return (std::shared_ptr<CTask::Action> (new ActionCopyFile(kTaskZipFile)));
                break;
            case 4:
                return (std::shared_ptr<CTask::Action> (new ActionCopyFile(kTaskRunCommand)));
                break;
             case 5:
                return (std::shared_ptr<CTask::Action> (new ActionCopyFile(kTaskImportCSVFile)));
                break;      
        }

    }

    //
    // Run a specified command on the file (%1% source, %2% destination)
    //

    bool ActionRunCommand::process(const string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(file);
        fs::path destinationFile(this->m_actionData[kDestinationOption] + sourceFile.filename().string());

        // Create correct command for whether source and destination specified or just source or none

        bool srcFound = (this->m_actionData[kCommandOption].find("%1%") != string::npos);
        bool dstFound = (this->m_actionData[kCommandOption].find("%2%") != string::npos);

        string commandStr;
        if (srcFound && dstFound) {
            commandStr = (boost::format(this->m_actionData[kCommandOption]) % sourceFile.string() % destinationFile.string()).str();
        } else if (srcFound) {
            commandStr = (boost::format(this->m_actionData[kCommandOption]) % sourceFile.string()).str();
        } else {
            commandStr = this->m_actionData[kCommandOption];
        }

    }

    //
    // Video file conversion action function. Convert passed in file to MP4 using Handbrake.
    //

    bool ActionVideoConversion::process(const string& file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        // Form source and destination file paths

        fs::path sourceFile(file);
        fs::path destinationFile(this->m_actionData[kDestinationOption]);

        destinationFile /= sourceFile.stem().string();

        if (this->m_actionData["extension"].length() > 0) {
            destinationFile.replace_extension(this->m_actionData["extension"]);
        } else {
            destinationFile.replace_extension(".mp4");
        }

        // Convert file

        string commandStr = (boost::format(this->m_actionData[kCommandOption]) % sourceFile.string() % destinationFile.string()).str();

        std::cout << "Converting file [" << sourceFile.string() << "] To [" << destinationFile.string() << "]" << std::endl;

        auto result = 0;
        if ((result = runShellCommand(commandStr)) == 0) {
            bSuccess = true;
            std::cout << "File conversion success." << std::endl;
            if (!this->m_actionData[kDeleteOption].empty()) {
                std::cout << "Deleting Source [" << sourceFile.string() << "]" << std::endl;
                fs::remove(sourceFile);
            }

        } else {
             std::cout << "File conversion error: " << to_string(result) << std::endl;
        }

        return (bSuccess);

    }

    //
    // Copy file action function. Copy passed file to destination folder/directory 
    // keeping the sources directory structure.
    //

    bool ActionCopyFile::process(const string &file) {

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

    //
    // Email file action function.
    //

    void ActionEmailFile::init(void) {
        CSMTP::init();
        CIMAP::init();
    };

    void ActionEmailFile::term(void) {
        CSMTP::closedown();
        CIMAP::closedown();
    };

    bool ActionEmailFile::process(const string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        CSMTP smtp;

        // Form source file path

        fs::path sourceFile(file);

        try {

            smtp.setServer(this->m_actionData[kServerOption]);
            smtp.setUserAndPassword(this->m_actionData[kUserOption], this->m_actionData[kPasswordOption]);
            smtp.setFromAddress("<" + this->m_actionData[kUserOption] + ">");
            smtp.setToAddress("<" + this->m_actionData[kRecipientOption] + ">");

            smtp.setMailSubject("FPE Attached File");
            smtp.addFileAttachment(file, CMIME::getFileMIMEType(file), "base64");

            if (this->m_actionData[kServerOption].find(string("smtp")) == 0) {

                smtp.postMail();
                std::cout << "Emailing file [" << file << "] to [" << this->m_actionData[kRecipientOption] << "]" << std::endl;
                bSuccess = true;

            } else if (this->m_actionData[kServerOption].find(string("imap")) == 0) {

                CIMAP imap;
                string mailMessageStr;
                string commandLineStr;

                commandLineStr = "Append " + this->m_actionData[kMailBoxOption] + " (\\Seen) {";
                mailMessageStr = smtp.getMailMessage();
                commandLineStr += to_string(mailMessageStr.length() - 2) + "}" + mailMessageStr;

                imap.setServer(this->m_actionData[kServerOption]);
                imap.setUserAndPassword(this->m_actionData[kUserOption], this->m_actionData[kPasswordOption]);

                imap.connect();

                string commandResponseStr(imap.sendCommand(commandLineStr));

                CIMAPParse::COMMANDRESPONSE commandResponse(CIMAPParse::parseResponse(commandLineStr));
                if (commandResponse->status == CIMAPParse::RespCode::BAD) {
                    std::cout << commandResponse->errorMessage << std::endl;
                } else {
                    std::cout << "Added file [" << file << "] to [" << this->m_actionData[kMailBoxOption]  << "]" << std::endl;
                    bSuccess = true;
                }

                imap.disconnect();

            }

        } catch (const CSMTP::Exception &e) {
            std::cerr << e.what() << std::endl;
        } catch (const CIMAP::Exception &e) {
            std::cerr << e.what() << std::endl;
        } catch (const exception & e) {
             std::cerr << "Standard exception occured: [" << e.what() << "]" << std::endl;
        }

        return (bSuccess);

    }

    //
    // Add file to ZIP archive.
    //

    bool ActionZIPFile::process(const string &file) {

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

    //
    // Import CSV File to MongoDB
    //

    bool ActionImportCSVFile::process(const string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        // Form source file path

#ifdef MONGO_DRIVER_INSTALLED

        fs::path sourceFile(file);

        ifstream csvFileStream(sourceFile.string());
        if (!csvFileStream.is_open()) {
            std::cout << "Error opening file " << sourceFile.string() << std::endl;
            return (false);
        }

        std::cout << "Importing CSV file [" << sourceFile.filename().string() << "] To MongoDB." << std::endl;

        mongocxx::instance driverInstance{};
        mongocxx::client mongoConnection{mongocxx::uri{this->m_actionData[kServerOption]}};
        auto csvCollection = mongoConnection[this->m_actionData[kDatabaseOption]][this->m_actionData[kCollectionOption]];
        vector<string> fieldNames;
        string csvLineStr;

        getline(csvFileStream, csvLineStr);
        if (csvLineStr.back() == '\r')csvLineStr.pop_back();

        fieldNames = getCSVTokens(csvLineStr);

        while (getline(csvFileStream, csvLineStr)) {
            vector< string > fieldValues;
            bsoncxx::builder::stream::document document{};
            if (csvLineStr.back() == '\r')csvLineStr.pop_back();
            fieldValues = getCSVTokens(csvLineStr);
            int i = 0;
            for (auto& field : fieldValues) {
                document << fieldNames[i++] << field;
            }
            csvCollection.insert_one(document.view());
        }
#endif // MONGO_DRIVER_INSTALLED

        return (bSuccess);

    }

} // namespace FPE_ActionFuncs