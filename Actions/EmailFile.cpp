#include "HOST.hpp"
/*
 * File:   EmailFile.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: EmailFile
//
// Description: Take passed in file  attach it to an email and send to recipient
// (SMTP) or append it to an email box (IMAP).
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CSMTP, CIMAP, CIMAPParse, CMIME
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

#include "CSMTP.hpp"
#include "CIMAP.hpp"
#include "CIMAPParse.hpp"
#include "CMIME.hpp"

//
// Boost file system
//

#include <boost/filesystem.hpp>

namespace FPE_TaskActions {

    // =======
    // IMPORTS
    // =======

    using namespace std;

    using namespace FPE;

    using namespace Antik::IMAP;
    using namespace Antik::File;
    using namespace Antik::SMTP;
    
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
    // Email file task action.
    //

    void EmailFile::init(void) {
        CSMTP::init();
        CIMAP::init();
    };

    void EmailFile::term(void) {
        CSMTP::closedown();
        CIMAP::closedown();
    };

    bool EmailFile::process(const string &file) {

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
                cout << "Emailing file [" << file << "] to [" << this->m_actionData[kRecipientOption] << "]" << endl;
                bSuccess = true;

            } else if (this->m_actionData[kServerOption].find(string("imap")) == 0) {

                CIMAP imap;
                string mailMessage;
                string commandLine;

                commandLine = "Append " + this->m_actionData[kMailBoxOption] + " (\\Seen) {";
                mailMessage = smtp.getMailMessage();
                commandLine += to_string(mailMessage.length() - 2) + "}" + mailMessage;

                imap.setServer(this->m_actionData[kServerOption]);
                imap.setUserAndPassword(this->m_actionData[kUserOption], this->m_actionData[kPasswordOption]);

                imap.connect();

                string response(imap.sendCommand(commandLine));

                CIMAPParse::COMMANDRESPONSE commandResponse(CIMAPParse::parseResponse(commandLine));
                if (commandResponse->status == CIMAPParse::RespCode::BAD) {
                    cout << commandResponse->errorMessage << endl;
                } else {
                    cout << "Added file [" << file << "] to [" << this->m_actionData[kMailBoxOption]  << "]" << endl;
                    bSuccess = true;
                }

                imap.disconnect();

            }

        } catch (const CSMTP::Exception &e) {
            cerr << e.what() << endl;
        } catch (const CIMAP::Exception &e) {
            cerr << e.what() << endl;
        } catch (const exception & e) {
             cerr << "Standard exception occured: [" << e.what() << "]" << endl;
        }

        return (bSuccess);

    }

} // namespace FPE_Actions
