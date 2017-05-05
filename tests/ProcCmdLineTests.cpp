#include "HOST.hpp"
/*
 * File:   ProcCmdLineTests.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 * 
 * Description: Google unit tests for FPE command line processing.
 *
 * Copyright 2016.
 *
 */

// =============
// INCLUDE FILES
// =============

//
// Google test definitions
//

#include "gtest/gtest.h"

//
// FPE Components
//
#include "FPE.hpp"
#include "FPE_ProcCmdLine.hpp"

using namespace FPE;
using namespace FPE_ProcCmdLine;

// =======================
// UNIT TEST FIXTURE CLASS
// =======================

class ProcCmdLineTests : public ::testing::Test {
    
protected:

    // Empty constructor
    
    ProcCmdLineTests() {
    }

    // Empty destructor
    
    virtual ~ProcCmdLineTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {

    }
    
    int argvLen (char *argv[]);
    

};

// ===============
// FIXTURE METHODS
// ===============

//
// Count length of argv to pass to fetchCommandLineOptionData.
//

int ProcCmdLineTests::argvLen (char *argv[]) {
    int argc = 0; 
    while(argv[++argc] != NULL);
    return(argc);
}

// ==================================
// COMMAND LINE PROCESSING UNIT TESTS
// ==================================

//
// Process command line with no options
//

TEST_F(ProcCmdLineTests, NoParameters) { 
    
    FPEOptions argData;
    
    char *argv[] = { (char *)"fpe" ,  nullptr};
     
    EXPECT_EXIT(argData = fetchCommandLineOptionData(this->argvLen(argv), argv), ::testing::ExitedWithCode(1), "FPE Error: the option '--destination' is required but missing");

}

//
// Command fpe --task 0 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileNoDelete) {

    FPEOptions argData;
      
    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "0",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
    
    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);
    
    EXPECT_FALSE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Copy File", argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));
    
}

//
// Command fpe --delete --task 0 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileDelete) {

    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--task",
        (char *) "0",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Copy File", argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));
  
}

//
// Command fpe --task 1 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNoDelete) {
    
    FPEOptions argData;
    
    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "1",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);
  
    EXPECT_FALSE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Video Conversion" , argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));


}

//
// Command fpe --delete --task 1 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionDelete) {

    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--task",
        (char *) "1",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Video Conversion" , argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));

    
}

//
// Command fpe --task 4 "echo %1% -------> %2%" --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskRunCommandCommandNoDelete) {
    
    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "4",
        (char *) "--command",
        (char *) "echo %1% -------> %2%",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Run Command" , argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("echo %1% -------> %2%", argData.optionsMap[kCommandOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));


}

//
// Command fpe --delete --task 4 "echo %1% -------> %2%" /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskRunCommandCommandDelete) {
    
    FPEOptions argData;
    
    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--task",
        (char *) "4",
        (char *) "--command", 
        (char *) "echo %1% -------> %2%",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);
 
    EXPECT_TRUE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Run Command" , argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("echo %1% -------> %2%", argData.optionsMap[kCommandOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));


}

//
// Command fpe --task 0 --maxdepth 3 /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileMaxDepth3) {
    
    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "0",
        (char*) "--maxdepth",
        (char *) "3",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);
    
    EXPECT_FALSE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Copy File", argData.taskFunc.name.c_str());
    EXPECT_EQ(3, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));

}
#if 0
//
// Command fpe --delete /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, NoTaskDefaultCopyDelete) {
    
    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
    
    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);
 
    EXPECT_TRUE(getOption<bool>(argData, kDeleteOption));
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));

 
}

//
// Command fpe --task 0 --task 1 --delete /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, MultipleTasksSpecified) {
    
    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task 0",
        (char *) "--task 1",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    EXPECT_EXIT (argData = fetchCommandLineOptionData(this->argvLen(argv), argv), ::testing::ExitedWithCode(1), "FPE Error: More than one task specified");
 
}
#endif
//
// Command fpe --delete --task 1 --extension ".mkv" --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNewExtension) {

    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "1",
        (char *) "--extension",
        (char *) ".mkv",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Video Conversion" , argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ(".mkv", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_FALSE(getOption<bool>(argData, kQuietOption));

    
}

//
// Command fpe --task 1 --quiet  --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionQuiet) {

    FPEOptions argData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "1",
        (char *) "--quiet",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };

    argData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(argData, kDeleteOption));
    ASSERT_STREQ("Video Conversion" , argData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(argData, kMaxDepthOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("", argData.optionsMap[kExtensionOption].c_str());
    EXPECT_TRUE(getOption<bool>(argData, kQuietOption));

    
}

// =====================
// RUN GOOGLE UNIT TESTS
// =====================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}