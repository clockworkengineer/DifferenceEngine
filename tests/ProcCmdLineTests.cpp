/*
 * File:   ProcCmdLineTests.cpp
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
#include "FPE_ProcCmdLine.hpp"

// Use if tracing wanted to test/create new tests

void coutstr(const std::vector<std::string>& outstr);
void cerrstr(const std::vector<std::string>& errstr);

//
// ProcCmdLineTests fixtures and constants
//

class ProcCmdLineTests : public ::testing::Test {
    
protected:

    ProcCmdLineTests() {

    }

    virtual ~ProcCmdLineTests() {

    }

    virtual void SetUp() {

    }

    virtual void TearDown() {

    }
    
    int argvLen (char *argv[]);
    

};

//
// Count length of argv to pass to procCmdLine.
//

int ProcCmdLineTests::argvLen (char *argv[]) {
    int argc = 0; 
    while(argv[++argc] != NULL);
    return(argc);
}

//
// Process command line with no options
//

TEST_F(ProcCmdLineTests, NoParameters) { 
    
    ParamArgData argData;
    
    char *argv[] = { (char *)"fpe" ,  nullptr};
     
    EXPECT_EXIT(procCmdLine(this->argvLen(argv), argv, argData), ::testing::ExitedWithCode(1), "FPE Error: the option '--destination' is required but missing");

}

//
// Command fpe --copy --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileNoDelete) {

    ParamArgData argData;
      
    char *argv[] = {
        (char *) "fpe",
        (char *) "--copy",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
    
    procCmdLine(this->argvLen(argv), argv, argData);
    
    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);
    
}

//
// Command fpe --delete --copy --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileDelete) {

    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--copy",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    procCmdLine(this->argvLen(argv), argv, argData);

    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);
  
}

//
// Command fpe --video --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNoDelete) {
    
    ParamArgData argData;
    
    char *argv[] = {
        (char *) "fpe",
        (char *) "--video",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    procCmdLine(this->argvLen(argv), argv, argData);
  
    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_TRUE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);


}

//
// Command fpe --delete --video --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionDelete) {

    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--video",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    procCmdLine(this->argvLen(argv), argv, argData);

    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_TRUE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);

    
}

//
// Command fpe --command "echo %1% -------> %2%" --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskRunCommandCommandNoDelete) {
    
    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--command",
        (char *) "echo %1% -------> %2%",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    procCmdLine(this->argvLen(argv), argv, argData);

    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_TRUE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("echo %1% -------> %2%", argData.commandToRun.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);


}

//
// Command fpe --delete --command "echo %1% -------> %2%" /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskRunCommandCommandDelete) {
    
    ParamArgData argData;
    
    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--command",
        (char *) "echo %1% -------> %2%",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    procCmdLine(this->argvLen(argv), argv, argData);
 
    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_TRUE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("echo %1% -------> %2%", argData.commandToRun.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);


}

//
// Command fpe --copy --maxdepth 3 /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileMaxDepth3) {
    
    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--copy",
        (char*) "--maxdepth",
        (char *) "3",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    procCmdLine(this->argvLen(argv), argv, argData);
    
    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(3, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);

}

//
// Command fpe --delete /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, NoTaskDefaultCopyDelete) {
    
    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
    
    procCmdLine(this->argvLen(argv), argv, argData);
 
    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);

 
}

//
// Command fpe --copy --video --delete /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, MultipleTasksSpecified) {
    
    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--copy",
        (char *) "--video",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    EXPECT_EXIT (procCmdLine(this->argvLen(argv), argv, argData), ::testing::ExitedWithCode(1), "FPE Error: More than one task specified");
 
}

//
// Command fpe --delete --video --extension ".mkv" --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNewExtension) {

    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--video",
        (char *) "--extension",
        (char *) ".mkv",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    procCmdLine(this->argvLen(argv), argv, argData);

    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_TRUE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ(".mkv", argData.extension.c_str());
    EXPECT_FALSE(argData.bQuiet);

    
}

//
// Command fpe --video --quiet  --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionQuiet) {

    ParamArgData argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--video",
        (char *) "--quiet",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    procCmdLine(this->argvLen(argv), argv, argData);

    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_TRUE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("", argData.extension.c_str());
    EXPECT_TRUE(argData.bQuiet);

    
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}