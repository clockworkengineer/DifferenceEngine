/*
 * File:   ActionFuncsTests.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * The MIT License
 *
 * Copyright 2016.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "gtest/gtest.h"

#include "FPE_ActionFuncs.hpp"
#include "FPE_ProcCmdLine.hpp"

//
// ActionFuncsTests fixtures and constants
//

class ActionFuncsTests : public ::testing::Test {
protected:

    ActionFuncsTests() {

        // Create function data (wrap in void shared pointer for passing to task).
        fnData.reset(new ActFnData{kWatchFolder, kDestinationFolder, "", false});
        funcData = static_cast<ActFnData *> (fnData.get());

    }

    virtual ~ActionFuncsTests() {
        // Nothing here for present
    }

    virtual void SetUp() {

        // Create watch directory.

        if (!fs::exists(kWatchFolder)) {
            fs::create_directory(kWatchFolder);
        }

        // Create kDestinationFolder directory.

        if (!fs::exists(kDestinationFolder)) {
            fs::create_directory(kDestinationFolder);
        }

    }

    virtual void TearDown() {

        // Remove any files created
        
        if ((filePath.length() != 0) && (fileName.length() != 0)) {
            
            if (fs::exists(filePath + fileName)) {
                boost::filesystem::remove(filePath + fileName);
            }

            if (fs::exists(kDestinationFolder + fileName)) {
                boost::filesystem::remove(kDestinationFolder + fileName);
            }
            
            filePath = "";
            fileName = "";
            
        }

    }

    std::shared_ptr<void> fnData;                   // Action function data shared pointer wrapper

    ActFnData *funcData;                            // Action function data 

    std::string filePath = "";                      // Test file path
    std::string fileName = "";                      // Test file name

    static const std::string kWatchFolder;          // Test Watch Folder
    static const std::string kDestinationFolder;    // Test Destination folde

    static const std::string kParamAssertion1;      // Missing parameter 1 Assert REGEX
    static const std::string kParamAssertion2;      // Missing parameter 2 Assert REGEX
    static const std::string kParamAssertion3;      // Missing parameter 3 Assert REGEX

};

const std::string ActionFuncsTests::kWatchFolder("/tmp/watch/");
const std::string ActionFuncsTests::kDestinationFolder("/tmp/destination/");

const std::string ActionFuncsTests::kParamAssertion1("Assertion*"); // NEED TO MODIFY FOR SPECIFIC ASSERTS
const std::string ActionFuncsTests::kParamAssertion2("Assertion*");
const std::string ActionFuncsTests::kParamAssertion3("Assertion*");

//
// Create a file for test purposes.
//

void createFile(std::string fileName) {

    std::ofstream outfile(fileName);
    outfile << "Happy XMAS!!!" << std::endl;
    outfile.close();
    
}

//
// ====================
// TASK COPY FILE TESTS
// ====================
//

//
// Function data null Assert
//

TEST_F(ActionFuncsTests, TaskCopyFileAssertParam1) {

    EXPECT_DEATH(copyFile(filePath, fileName, nullptr), ActionFuncsTests::kParamAssertion1);

}

//
// filePath length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskCopyFileAssertParam2) {

    EXPECT_DEATH(copyFile(filePath, fileName, fnData), ActionFuncsTests::kParamAssertion2);

}

//
// fileName length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskCopyFileAssertParam3) {

    filePath = kWatchFolder;

    EXPECT_DEATH(copyFile(filePath, fileName, fnData), ActionFuncsTests::kParamAssertion3);

}

//
// Source file doesn't exist exception
//

TEST_F(ActionFuncsTests, TaskCopyFileSourceNotExist) {

    filePath = kWatchFolder;
    fileName = "temp1.txt";

    EXPECT_THROW(copyFile(filePath, fileName, fnData), fs::filesystem_error);

}

//
// File copied correctly
//

TEST_F(ActionFuncsTests, TaskCopyFileSourceExists) {

    filePath = kWatchFolder;
    fileName = "temp1.txt";

    createFile(filePath + fileName);

    EXPECT_TRUE(copyFile(filePath, fileName, fnData));


}

//
// File copied with destination existing.
//

TEST_F(ActionFuncsTests, TaskCopyFileDestinationExists) {

    filePath = kWatchFolder;
    fileName = "temp1.txt";

    createFile(filePath + fileName);
    createFile(kDestinationFolder + fileName);

    EXPECT_FALSE(copyFile(filePath, fileName, fnData));

}

//
// ======================
// TASK RUN COMMAND TESTS
// ======================
//

//
// Function data null Assert
//

TEST_F(ActionFuncsTests, TaskRunCommandAssertParam1) {

    EXPECT_DEATH(runCommand(filePath, fileName, nullptr), ActionFuncsTests::kParamAssertion1);

}

//
// filePath length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskRunCommandAssertParam2) {

    EXPECT_DEATH(runCommand(filePath, fileName, fnData), ActionFuncsTests::kParamAssertion2);

}

//
// fileName length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskRunCommandAssertParam3) {

    filePath = kWatchFolder;

    EXPECT_DEATH(runCommand(filePath, fileName, fnData), ActionFuncsTests::kParamAssertion3);

}

//
// run command no file exists. 
//

TEST_F(ActionFuncsTests, TaskRunCommandSourceNotExist) {

    filePath = kWatchFolder;
    fileName = "temp1.txt";

    funcData->commandToRun = "echo %1%";    // Doesn't matter file doesn't exist so TRUE.
    EXPECT_TRUE(runCommand(filePath, fileName, fnData));

    funcData->commandToRun = "ls %1%";      // Does matter file doesn't exist so FALSE.
    EXPECT_FALSE(runCommand(filePath, fileName, fnData));


}

TEST_F(ActionFuncsTests, TaskRunCommandSourceExists) {

    filePath = kWatchFolder;
    fileName = "temp1.txt";

    createFile(filePath + fileName);

    EXPECT_TRUE(fs::exists(filePath + fileName));               // File should exist
    EXPECT_FALSE(fs::exists(kDestinationFolder + fileName));    // Destination shouldn't

    funcData->commandToRun = "cp %1% %2%";                      // Copy file using cp
    EXPECT_TRUE(runCommand(filePath, fileName, fnData));

    EXPECT_TRUE(fs::exists(kDestinationFolder + fileName));     // Destination should now

}

//
// ====================
// TASK HANDBRAKE TESTS
// ====================
//

//
// Function data null Assert
//

TEST_F(ActionFuncsTests, TaskHandBrakeAssertParam1) {

    EXPECT_DEATH(handBrake(filePath, fileName, nullptr), ActionFuncsTests::kParamAssertion1);

}

//
// filePath length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskHandBrakeAssertParam2) {

    EXPECT_DEATH(handBrake(filePath, fileName, fnData), ActionFuncsTests::kParamAssertion2);

}

//
// fileName length == 0 ASSERT
//

TEST_F(ActionFuncsTests, TaskHandBrakeAssertParam3) {

    filePath = kWatchFolder;

    EXPECT_DEATH(handBrake(filePath, fileName, fnData), ActionFuncsTests::kParamAssertion3);

}

// ADD MORE LATER ....................

//
// RUN GOOGLE TEST
//

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}