#include "HOST.hpp"
/*
 * File:   ImportCSVFile.cpp
 *
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

//
// Module: ImportCSVFile
//
// Description: Take passed in CSV and import it into a MongoDB.
//
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CFile, CPath.
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
// Boost tokenizer library
//

#include <boost/tokenizer.hpp>

//
// MongoDB C++ Driver
// Note: C++ Driver not easy to install so add define
//

#if defined(MONGO_DRIVER_INSTALLED)
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#endif // MONGO_DRIVER_INSTALLED

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

    vector<string> getCSVTokens(const std::string& csvLineStr) {

        boost::tokenizer< boost::escaped_list_separator<char> > csvTokenizer(csvLineStr);
        vector<string> csvTokens;

        csvTokens.assign(csvTokenizer.begin(), csvTokenizer.end());

        return (csvTokens);

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Import CSV File to MongoDB
    //

    bool ImportCSVFile::process(const string &file) {

        // ASSERT for any invalid options.

        assert(file.length() != 0);

        bool bSuccess = false;

        // Form source file path

#if defined(MONGO_DRIVER_INSTALLED)

        try {

            CPath sourceFile(file);

            ifstream csvFileStream(sourceFile.toString());
            if (!csvFileStream.is_open()) {
                cout << "Error opening file " << sourceFile.toString() << endl;
                return (false);
            }

            cout << "Importing CSV file [" << sourceFile.fileName() << "] To MongoDB." << endl;

            mongocxx::instance driverInstance{};
            mongocxx::client mongoConnection{mongocxx::uri{this->m_actionData[kServerOption]}};
            auto csvCollection = mongoConnection[this->m_actionData[kDatabaseOption]][this->m_actionData[kCollectionOption]];
            vector<string> fieldNames;
            string csvLine;

            getline(csvFileStream, csvLine);
            if (csvLine.back() == '\r')csvLine.pop_back();

            fieldNames = getCSVTokens(csvLine);

            while (getline(csvFileStream, csvLine)) {
                vector< string > fieldValues;
                bsoncxx::builder::stream::document document{};
                if (csvLine.back() == '\r')csvLine.pop_back();
                fieldValues = getCSVTokens(csvLine);
                int i = 0;
                for (auto& field : fieldValues) {
                    document << fieldNames[i++] << field;
                }
                csvCollection.insert_one(document.view());
            }

        } catch (const exception & e) {
            cerr << this->getName() << " Error: " << e.what() << endl;
        }
#endif // MONGO_DRIVER_INSTALLED

        return (bSuccess);

    }

} // namespace FPE_TaskActions