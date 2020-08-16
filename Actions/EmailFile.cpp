//
// Module: EmailFile
//
// Description: Take passed in file  attach it to an email and send to recipient
// (SMTP) or append it to an email box (IMAP).
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CSMTP, CIMAP, CIMAPParse, CMIME, CFile, CPath
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

#include "CSMTP.hpp"
#include "CIMAP.hpp"
#include "CIMAPParse.hpp"
#include "CMIME.hpp"
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
    using namespace Antik::IMAP;
    using namespace Antik::File;
    using namespace Antik::SMTP;

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
    };

    void EmailFile::term(void) {
        CSMTP::closedown();
    };

    bool EmailFile::process(const std::string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        CSMTP smtp;

        // Form source file path

        CPath sourceFile(file);

        try {

            smtp.setServer(this->m_actionData[kServerOption]);
            smtp.setUserAndPassword(this->m_actionData[kUserOption], this->m_actionData[kPasswordOption]);
            smtp.setFromAddress("<" + this->m_actionData[kUserOption] + ">");
            smtp.setToAddress("<" + this->m_actionData[kRecipientOption] + ">");

            smtp.setMailSubject("FPE Attached File");
            smtp.addFileAttachment(file, CMIME::getFileMIMEType(file), "base64");

            if (this->m_actionData[kServerOption].find(std::string("smtp")) == 0) {

                smtp.postMail();
                std::cout << "Emailing file [" << file << "] to [" << this->m_actionData[kRecipientOption] << "]" << std::endl;
                bSuccess = true;

            } else if (this->m_actionData[kServerOption].find(std::string("imap")) == 0) {

                CIMAP imap;
                std::string mailMessage;
                std::string commandLine;

                commandLine = "Append " + this->m_actionData[kMailBoxOption] + " (\\Seen) {";
                mailMessage = smtp.getMailMessage();
                commandLine += std::to_string(mailMessage.length() - 2) + "}" + mailMessage;

                imap.setServer(this->m_actionData[kServerOption]);
                imap.setUserAndPassword(this->m_actionData[kUserOption], this->m_actionData[kPasswordOption]);

                imap.connect();

                std::string response(imap.sendCommand(commandLine));

                CIMAPParse::COMMANDRESPONSE commandResponse(CIMAPParse::parseResponse(response));
                if (commandResponse->status == CIMAPParse::RespCode::BAD) {
                    std::cout << commandResponse->errorMessage << std::endl;
                } else {
                    std::cout << "Added file [" << file << "] to [" << this->m_actionData[kMailBoxOption] << "]" << std::endl;
                    bSuccess = true;
                }

                imap.disconnect();

            }

        } catch (const CSMTP::Exception &e) {
            std::cerr << this->getName() << " Error: " << e.what() << std::endl;
        } catch (const CIMAP::Exception &e) {
            std::cerr << this->getName() << " Error: " << e.what() << std::endl;
        } catch (const std::exception & e) {
            std::cerr << this->getName() << " Error: " << e.what() << std::endl;
        }

        return (bSuccess);

    }

} // namespace FPE_TaskActions
