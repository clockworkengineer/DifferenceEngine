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

//
// TaskClassTests fixtures and constants
//

class TaskClassTests : public ::testing::Test {
protected:

    TaskClassTests() {

    }

    virtual ~TaskClassTests() {
        // Create function data (wrap in void shared pointer for passing to task).
        fnData.reset(new ActFnData{kWatchFolder, kDestinationFolder, "", false});
        funcData = static_cast<ActFnData *> (fnData.get());
    }

    virtual void SetUp() {

        // Save and redirect stdout/stderr for tests

        cout_sbuf = std::cout.rdbuf();      // save original sbuf
        std::cout.rdbuf(fileout.rdbuf());   // redirect 'cout'

        cerr_sbuf = std::cerr.rdbuf();      // save original sbuf
        std::cerr.rdbuf(fileerr.rdbuf());   // redirect 'cerr'

    }

    virtual void TearDown() {

        // Restore stdout/stderr stream buffers

        std::cout.rdbuf(cout_sbuf);
        std::cout.rdbuf(cerr_sbuf);
        
        filePath = ""; // Test file path
        fileName = ""; // Test file name
    
        watchDepth = -1;
        taskName = "";
        watchFolder = "";

    }
    
    // stdout/stderr redirect for tests

    std::streambuf* cout_sbuf = std::cout.rdbuf(); // save original sbuf
    std::streambuf* cerr_sbuf = std::cerr.rdbuf(); // save original sbuf
    std::ofstream fileout{ "/dev/null"};
    std::ofstream fileerr{ "/dev/null"};
    
    std::shared_ptr<void> fnData; // Action function data shared pointer wrapper

    ActFnData *funcData; // Action function data 

    std::string filePath = ""; // Test file path
    std::string fileName = ""; // Test file name
    
    int watchDepth = -1;
    std::string taskName = "";
    std::string watchFolder = "";
    
    TaskActionFcn taskActFcn;

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

TEST_F(TaskClassTests, TaskClassAssertParam1) {

   EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->watchDepth, this->taskActFcn, this->fnData), TaskClassTests::kParamAssertion1);

}

TEST_F(TaskClassTests, TaskClassAssertParam2) {

   this->taskName="Test";
   this->watchFolder= kWatchFolder;
   
    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->watchDepth, this->taskActFcn, this->fnData), TaskClassTests::kParamAssertion2);

}

TEST_F(TaskClassTests, TaskClassAssertParam3) {

   this->taskName="Test";
   this->watchFolder= kWatchFolder;
   this->watchDepth = -99;
   
    EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->watchDepth, this->taskActFcn, this->fnData), TaskClassTests::kParamAssertion3);

}

TEST_F(TaskClassTests, TaskClassAssertParam4) {

   this->taskName="Test";
   this->watchFolder= kWatchFolder;
   this->watchDepth = -1;
   
   EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->watchDepth, nullptr, this->fnData), TaskClassTests::kParamAssertion4);

}

TEST_F(TaskClassTests, TaskClassAssertParam5) {

   this->taskName="Test";
   this->watchFolder= kWatchFolder;
   this->watchDepth = -1;
   this->taskActFcn = [] (auto filenamePathStr, auto filenameStr, auto fnData) -> bool 
                   { std::cout << "[" << filenamePathStr+filenameStr << "]" << std::endl; return true; };
   
   EXPECT_DEATH(FPE_Task task(this->taskName, this->watchFolder, this->watchDepth, this->taskActFcn, nullptr),  TaskClassTests::kParamAssertion5);

}


//
// RUN GOOGLE TEST
//

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}