#include "HOST.hpp"
/*
 * File:   ActionFuncsTests.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 * 
 * Description: Google unit tests for FPE*  task action functions.
 *
 * Copyright 2016.
 *
 */

// =============
// INCLUDE FILES
// =============

// Google test definitions

#include "gtest/gtest.h"

// CLogger class definitions

#include "CLogger.hpp"

// Task Action function and process command line definitions

#include "FPE_ActionFuncs.hpp"
#include "FPE_ProcCmdLine.hpp"

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// Used from FPE.cpp if tracing wanted to test/create new tests

//void coutstr(const std::vector<std::string>& outstr);
//void cerrstr(const std::vector<std::string>& errstr);

// =======================
// UNIT TEST FIXTURE CLASS
// =======================

class ActionFuncsTests : public ::testing::Test {
    
protected:

     // Empty constructor
    
    ActionFuncsTests() {
    }

    // Empty destructor
    
    virtual ~ActionFuncsTests() {
    }

    // Keep initialization and cleanup code to SetUp() and TearDown() methods
    
    virtual void SetUp() {
        
       // Create function data (wrap in void shared pointer for passing to task).
        
        this->fnData.reset(new ActFnData{ ActionFuncsTests::kWatchFolder,  ActionFuncsTests::kDestinationFolder, "", false, "", CLogger::noOp, CLogger::noOp});
        this->funcData = static_cast<ActFnData *> (this->fnData.get());
        
        // Create watch folder.

        if (!fs::exists( ActionFuncsTests::kWatchFolder)) {
            fs::create_directory( ActionFuncsTests::kWatchFolder);
        }

        // Create destination folder.

        if (!fs::exists( ActionFuncsTests::kDestinationFolder)) {
            fs::create_directory( ActionFuncsTests::kDestinationFolder);
        }
   

    }

    virtual void TearDown() {


        // Remove any files created

        if (fs::exists(this->filePath + this->fileName)) {
            boost::filesystem::remove(this->filePath + this->fileName);
        }

        if (fs::exists(ActionFuncsTests::kDestinationFolder + this->fileName)) {
            boost::filesystem::remove(ActionFuncsTests::kDestinationFolder + this->fileName);
        }
        
        // Remove watch folder.

        if (fs::exists( ActionFuncsTests::kWatchFolder)) {
            fs::remove( ActionFuncsTests::kWatchFolder);
        }

        // Remove destination folder.

        if (fs::exists( ActionFuncsTests::kDestinationFolder)) {
            fs::remove( ActionFuncsTests::kDestinationFolder);
        }


    }

    void createFile(std::string fileName);      // Create a temporary file for test
    
    std::shared_ptr<void> fnData; // Action function data shared pointer wrapper

    ActFnData *funcData; // Action function data 

    std::string filePath = ""; // Test file path
    std::string fileName = ""; // Test file name

    static const std::string kWatchFolder; // Test Watch Folder
    static const std::string kDestinationFolder; // Test Destination folder

    static const std::string kParamAssertion1; // Missing parameter 1 Assert REGEX
    static const std::string kParamAssertion2; // Missing parameter 2 Assert REGEX
    static const std::string kParamAssertion3; // Missing parameter 3 Assert REGEX

};

// =================
// FIXTURE CONSTANTS
// =================

const std::string ActionFuncsTests:: kWatchFolder("/tmp/watch/");
const std::string ActionFuncsTests:: kDestinationFolder("/tmp/destination/");

const std::string ActionFuncsTests::kParamAssertion1("Assertion*"); // NEED TO MODIFY FOR SPECIFIC ASSERTS
const std::string ActionFuncsTests::kParamAssertion2("Assertion*");

// ===============
// FIXTURE METHODS
// ===============

//
// Create a file for test purposes.
//

void ActionFuncsTests::createFile(std::string fileName) {

    std::ofstream outfile(fileName);
    outfile << "TEST TEXT" << std::endl;
    outfile.close();

}

// ====================
// TASK COPY FILE TESTS
// ====================

//
// Function data null pointer Assert.
//

TEST_F(ActionFuncsTests, TaskCopyFileAssertParam1) {

    EXPECT_DEATH(copyFile(this->filePath+this->fileName, nullptr), ActionFuncsTests::kParamAssertion1);

}

//
// FilePath length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskCopyFileAssertParam2) {

    EXPECT_DEATH(copyFile(this->filePath+this->fileName, this->fnData), ActionFuncsTests::kParamAssertion2);

}

//
// Source file doesn't exist exception
//

TEST_F(ActionFuncsTests, TaskCopyFileSourceNotExist) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    EXPECT_THROW(copyFile(this->filePath+this->fileName, this->fnData), fs::filesystem_error);

}

//
// File copied correctly
//

TEST_F(ActionFuncsTests, TaskCopyFileSourceExists) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);

    EXPECT_TRUE(copyFile(this->filePath+this->fileName, this->fnData));
    
    EXPECT_TRUE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName));


}

//
// File copied with destination existing. File is not copied.
//

TEST_F(ActionFuncsTests, TaskCopyFileDestinationExists) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);
    this->createFile( ActionFuncsTests::kDestinationFolder + this->fileName);
    
    EXPECT_TRUE(fs::exists(this->filePath + this->fileName));
    EXPECT_TRUE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName));

    EXPECT_FALSE(copyFile(this->filePath+this->fileName, this->fnData));

}

//
// File copied correctly and delete source set. On sucessful copy (ie. command) delete source.
//

TEST_F(ActionFuncsTests, TaskCopyFileSourceExistsDeleteSource) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName));
    EXPECT_FALSE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName));

    this->funcData->bDeleteSource = true;
    EXPECT_TRUE(copyFile(this->filePath+this->fileName, this->fnData));
    
    EXPECT_TRUE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName));
    EXPECT_FALSE(fs::exists(this->filePath + this->fileName));


}

//
// File copied with destination existing and delete source set. File not copied so
// do not delete source.
//

TEST_F(ActionFuncsTests, TaskCopyFileDestinationExistsDeleteSource) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);
    this->createFile( ActionFuncsTests::kDestinationFolder + this->fileName);
    
    EXPECT_TRUE(fs::exists(this->filePath + this->fileName));
    EXPECT_TRUE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName));

    this->funcData->bDeleteSource = true;
    EXPECT_FALSE(copyFile(this->filePath+this->fileName, this->fnData));
    EXPECT_TRUE(fs::exists(this->filePath + this->fileName));

}

// ======================
// TASK RUN COMMAND TESTS
// ======================

//
// Function data null pointer Assert
//

TEST_F(ActionFuncsTests, TaskRunCommandAssertParam1) {

    EXPECT_DEATH(runCommand(this->filePath+this->fileName, nullptr), ActionFuncsTests::kParamAssertion1);

}

//
// FilePath length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskRunCommandAssertParam2) {

    EXPECT_DEATH(runCommand(this->filePath+this->fileName, this->fnData), ActionFuncsTests::kParamAssertion2);

}

//
// Run command no file exists. 
//

TEST_F(ActionFuncsTests, TaskRunCommandSourceNotExist) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";
    
    EXPECT_FALSE(fs::exists(this->filePath + this->fileName));

    this->funcData->commandToRun = "echo %1%"; // Doesn't matter file doesn't exist so TRUE.
    EXPECT_TRUE(runCommand(this->filePath+this->fileName, this->fnData));

    this->funcData->commandToRun = "ls %1%"; // Does matter file doesn't exist so FALSE.
    EXPECT_FALSE(runCommand(this->filePath+this->fileName, this->fnData));


}

//
// Run command source file exists.
//

TEST_F(ActionFuncsTests, TaskRunCommandSourceExists) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath+this->fileName);

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should exist
    EXPECT_FALSE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName)); // Destination shouldn't

    this->funcData->commandToRun = "cp %1% %2%"; // Copy file using cp
    EXPECT_TRUE(runCommand(this->filePath+this->fileName, this->fnData));

    EXPECT_TRUE(fs::exists( ActionFuncsTests::kDestinationFolder + this->fileName)); // Destination should now

}

//
// Run invalid command.
//

TEST_F(ActionFuncsTests, TaskRunCommandInvalidCommand) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->funcData->commandToRun = "echo %1%"; // Run a valid command
    EXPECT_TRUE(runCommand(this->filePath+this->fileName, this->fnData));

    this->funcData->commandToRun = "foobar %1%"; // Now run foobar which is not a command
    EXPECT_FALSE(runCommand(this->filePath+this->fileName, this->fnData));


}

//
// Run command, no source file and specify delete source
//

TEST_F(ActionFuncsTests, TaskRunCommandNoSourceTryToDelete) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    EXPECT_FALSE(fs::exists(this->filePath + this->fileName)); // File should not exist

    this->funcData->bDeleteSource = true;
    this->funcData->commandToRun = "echo %1%"; // Run a valid command
    EXPECT_TRUE(runCommand(this->filePath+this->fileName, this->fnData));

}

//
// Run command, source exists and specify delete source
//

TEST_F(ActionFuncsTests, TaskRunCommandSourceExistsTryToDelete) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should exist

    this->funcData->bDeleteSource = true;
    this->funcData->commandToRun = "echo %1%"; // Run a valid command
    EXPECT_TRUE(runCommand(this->filePath+this->fileName, this->fnData));

    EXPECT_FALSE(fs::exists(this->filePath + this->fileName)); // File should have been deleted

}

//
// Run invalid command with source file and specify delete source
//

TEST_F(ActionFuncsTests, TaskRunCommandInvalidCommandSourceExistsTryToDelete) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should exist

    this->funcData->bDeleteSource = true;
    this->funcData->commandToRun = "foobar %1%"; // Run a invalid command
    EXPECT_FALSE(runCommand(this->filePath+this->fileName, this->fnData));

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should have not been deleted

}

// ====================
// TASK HANDBRAKE TESTS
// ====================

//
// Function data null pointer ASSERT
//

TEST_F(ActionFuncsTests, TaskHandBrakeAssertParam1) {

    EXPECT_DEATH(handBrake(this->filePath+this->fileName, nullptr), ActionFuncsTests::kParamAssertion1);

}

//
// FilePath length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskHandBrakeAssertParam2) {

    EXPECT_DEATH(handBrake(this->filePath+this->fileName, this->fnData), ActionFuncsTests::kParamAssertion2);

}

//
// Convert file with invalid format/extension 
//

TEST_F(ActionFuncsTests, TaskHandBrakeConvertInvalidFile) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should exist now 

    this->funcData->commandToRun = kHandbrakeCommand;
    EXPECT_FALSE(handBrake(this->filePath+this->fileName, this->fnData));

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should have not been deleted

}

//
// Convert file with invalid format/extension and delete source
//

TEST_F(ActionFuncsTests, TaskHandBrakeConvertInvalidFileDeleteSource) {

    this->filePath =  ActionFuncsTests::kWatchFolder;
    this->fileName = "temp1.txt";

    this->createFile(this->filePath + this->fileName);

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should exist now 

    this->funcData->bDeleteSource = true;
    this->funcData->commandToRun = kHandbrakeCommand;
    EXPECT_FALSE(handBrake(this->filePath+this->fileName, this->fnData));

    EXPECT_TRUE(fs::exists(this->filePath + this->fileName)); // File should have not been deleted

}

// =====================
// RUN GOOGLE UNIT TESTS
// =====================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}