/*
 * File:   TaskClassTests.cpp
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
#include "FPE_Task.hpp" 

// Boost file system and format libraries definitions

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fs = boost::filesystem;

// Test Action function data

struct TestActFnData {
    int fnCalledCount; // How many times action function called
};

//
// TaskClassTests fixtures and constants
//

class TaskClassTests : public ::testing::Test {
protected:

    TaskClassTests() {

        // Create function data (wrap in void shared pointer for passing to task).
        
        fnData.reset(new TestActFnData{0});
        funcData = static_cast<TestActFnData *> (fnData.get());
        
        this->taskOptions.reset(new TaskOptions { 0 , nullptr, nullptr });

    }

    virtual ~TaskClassTests() {
    }

    virtual void SetUp() {
        
        // Create watch folder.

        if (!fs::exists( TaskClassTests::kWatchFolder)) {
            fs::create_directory( TaskClassTests::kWatchFolder);
        }

        // Create destination folder.

        if (!fs::exists( TaskClassTests::kDestinationFolder)) {
            fs::create_directory( TaskClassTests::kDestinationFolder);
        }

    }

    virtual void TearDown() {
        
        // Remove watch folder.

        if (fs::exists( TaskClassTests::kWatchFolder)) {
            fs::remove( TaskClassTests::kWatchFolder);
        }

        // Remove destination folder.

        if (fs::exists( TaskClassTests::kDestinationFolder)) {
            fs::remove( TaskClassTests::kDestinationFolder);
        }

    }

    void createFile(std::string fileName); // Create a test file.
    void createFiles(int fileCount); // Create fileCount files and check action function call count

    std::shared_ptr<void> fnData; // Action function data shared pointer wrapper
    TestActFnData *funcData; // Action function data 

    std::string filePath = "";      // Test file path
    std::string fileName = "";      // Test file name
    int watchDepth = -1;            // Folder Watch depth
    std::string taskName = "";      // Task Name
    std::string watchFolder = "";   // Watch Folder

    TaskActionFcn taskActFcn;                   // Task Action Function Data
    std::shared_ptr<TaskOptions> taskOptions;   // Task options

    static const std::string kWatchFolder; // Test Watch Folder
    static const std::string kDestinationFolder; // Test Destination folder

    static const std::string kParamAssertion1; // Missing parameter 1 Assert REGEX
    static const std::string kParamAssertion2; // Missing parameter 2 Assert REGEX
    static const std::string kParamAssertion3; // Missing parameter 3 Assert REGEX
    static const std::string kParamAssertion4; // Missing parameter 4 Assert REGEX
    static const std::string kParamAssertion5; // Missing parameter 5 Assert REGEX

};

const std::string TaskClassTests::kWatchFolder("/tmp/watch/");
const std::string TaskClassTests::kDestinationFolder("/tmp/destination/");

const std::string TaskClassTests::kParamAssertion1("Assertion*"); // NEED TO MODIFY FOR SPECIFIC ASSERTS
const std::string TaskClassTests::kParamAssertion2("Assertion*");
const std::string TaskClassTests::kParamAssertion3("Assertion*");
const std::string TaskClassTests::kParamAssertion4("Assertion*");
const std::string TaskClassTests::kParamAssertion5("Assertion*");

//
// Create a file for test purposes.
//

void TaskClassTests::createFile(std::string fileName) {

    std::ofstream outfile(fileName);
    outfile << "TEST TEXT" << std::endl;
    outfile.close();

}

//
// Create fileCount files and check that action function called for each
//

void TaskClassTests::createFiles(int fileCount) {

    this->taskName = "Test";
    this->watchFolder = kWatchFolder;
    this->watchDepth = -1;

    this->taskActFcn = [] (auto filenamePathStr, auto fnData) -> bool {
        TestActFnData *funcData = static_cast<TestActFnData *> (fnData.get());
        funcData->fnCalledCount++;
        return true;
    };

    // Set any task options required by test
    
    (this->taskOptions)->killCount=fileCount;
    
    FPE_Task task{this->taskName, this->watchFolder, this->taskActFcn, this->fnData, this->watchDepth, this->taskOptions};

    // Create task object thread and start to watch

    std::unique_ptr<std::thread> taskThread;

    taskThread.reset(new std::thread(&FPE_Task::monitor, &task));

    this->filePath = TaskClassTests::kWatchFolder;

    for (auto cnt01 = 0; cnt01 < fileCount; cnt01++) {
        std::string file = (boost::format("temp%1%.txt") % cnt01).str();
        this->createFile(this->filePath + file);
    }

    taskThread->join();

    EXPECT_EQ(fileCount, funcData->fnCalledCount);

    for (auto cnt01 = 0; cnt01 < fileCount; cnt01++) {
        std::string file = (boost::format("temp%1%.txt") % cnt01).str();
        fs::remove(this->filePath + file);
    }

}

//
// Task Name lengh == 0 ASSERT
//

TEST_F(TaskClassTests, TaskClassAssertParam1) {

    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->taskActFcn, this->fnData, this->watchDepth), TaskClassTests::kParamAssertion1);

}

//
// Watch Folder Name lengh == 0 ASSERT
//

TEST_F(TaskClassTests, TaskClassAssertParam2) {

    this->taskName = "Test";

    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->taskActFcn, this->fnData, this->watchDepth), TaskClassTests::kParamAssertion2);

}

//
// Action Function Pointer == NULL ASSERT
//

TEST_F(TaskClassTests, TaskClassAssertParam3) {

    this->taskName = "Test";
    this->watchFolder = kWatchFolder;
    this->watchDepth = -1;

    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, nullptr, this->fnData, this->watchDepth), TaskClassTests::kParamAssertion3);

}

//
// Action Function Data Pointer == NULL ASSERT
//

TEST_F(TaskClassTests, TaskClassAssertParam4) {

    this->taskName = "Test";
    this->watchFolder = kWatchFolder;
    this->watchDepth = -1;

    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->taskActFcn, nullptr, this->watchDepth), TaskClassTests::kParamAssertion4);

}

//
// Watch Depth < -1 ASSERT
//

TEST_F(TaskClassTests, TaskClassAssertParam5) {

    this->taskName = "Test";
    this->watchFolder = kWatchFolder;
    this->watchDepth = -99;

    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->taskActFcn, this->fnData, this->watchDepth), TaskClassTests::kParamAssertion5);

}

//
// Create 1 file in watcher folder
//

TEST_F(TaskClassTests, TaskClassCreateFile1) {

    this->createFiles(1);

}

//
// Create 10 files in watcher folder
//

TEST_F(TaskClassTests, TaskClassCreateFile10) {

    this->createFiles(10);

}

//
// Create 50 files in watcher folder
//

TEST_F(TaskClassTests, TaskClassCreateFile50) {

    this->createFiles(50);

}

//
// Create 100 files in watcher folder
//

TEST_F(TaskClassTests, TaskClassCreateFile100) {

    this->createFiles(100);

}

//
// Create 250 files in watcher folder
//

TEST_F(TaskClassTests, TaskClassCreateFile250) {

    this->createFiles(250);

}

//
// Create 500 files in watcher folder
//

TEST_F(TaskClassTests, TaskClassCreateFile500) {

    this->createFiles(500);

}

//
//
// RUN GOOGLE TEST
//

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}