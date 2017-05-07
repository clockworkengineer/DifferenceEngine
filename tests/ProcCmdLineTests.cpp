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

TEST_F(ProcCmdLineTests, NoOptions) { 
    
    FPEOptions optionData;
    
    char *argv[] = { (char *)"fpe" ,  nullptr};
     
    EXPECT_EXIT(optionData = fetchCommandLineOptionData(this->argvLen(argv), argv), ::testing::ExitedWithCode(1), "FPE Error: the option '--destination' is required but missing.");

}

//
// Command fpe --task 0 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileNoDelete) {

    FPEOptions optionData;
      
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
    
    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);
    
    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
 
    ASSERT_STREQ("Copy File", optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    
    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kCommandOption].c_str());
   
    
}

//
// Command fpe --delete --task 0 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileDelete) {

    FPEOptions optionData;

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

    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
 
    ASSERT_STREQ("Copy File", optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kCommandOption].c_str());

  
}

//
// Command fpe --task 1 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNoDelete) {
    
    FPEOptions optionData;
    
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
 
    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);
  
    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
 
    ASSERT_STREQ("Video Conversion" , optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"", optionData.optionsMap[kCommandOption].c_str());
 
    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());
 


}

//
// Command fpe --delete --task 1 --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionDelete) {

    FPEOptions optionData;

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

    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
   
    ASSERT_STREQ("Video Conversion" , optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"", optionData.optionsMap[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());

    
}

//
// Command fpe --task 4 "echo %1% -------> %2%" --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskRunCommandCommandNoDelete) {
    
    FPEOptions optionData;

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

    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
 
    ASSERT_STREQ("Run Command" , optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("echo %1% -------> %2%", optionData.optionsMap[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());

}

//
// Command fpe --delete --task 4 "echo %1% -------> %2%" /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskRunCommandCommandDelete) {
    
    FPEOptions optionData;
    
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
 
    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);
 
    EXPECT_TRUE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
    
    ASSERT_STREQ("Run Command" , optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("echo %1% -------> %2%", optionData.optionsMap[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());

}

//
// Command fpe --task 0 --maxdepth 3 /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileMaxDepth3) {
    
    FPEOptions optionData;

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

    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);
    
    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
    
    ASSERT_STREQ("Copy File", optionData.taskFunc.name.c_str());
    EXPECT_EQ(3, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kCommandOption].c_str());


}

//
// Command fpe --task 9 /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, InvalidTaskNumber) {
    
    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "9",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
    
     EXPECT_EXIT(optionData = fetchCommandLineOptionData(this->argvLen(argv), argv), ::testing::ExitedWithCode(1), "FPE Error: Invalid Task Number.");

}

//
// Command fpe --task e --delete /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, MultipleTasksSpecified) {
    
    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "e",
        (char *) "--watch",
        (char *) "/home/pi/watchstuff/watch/",
        (char *) "--destination",
        (char *) "/home/pi/watchstuff/destination/",
        nullptr
    };
 
    EXPECT_EXIT (optionData = fetchCommandLineOptionData(this->argvLen(argv), argv), ::testing::ExitedWithCode(1), "FPE Error: task is not a valid integer.");
 
}

//
// Command fpe --delete --task 1 --extension ".mkv" --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNewExtension) {

    FPEOptions optionData;

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

    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
 
    ASSERT_STREQ("Video Conversion" , optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"", optionData.optionsMap[kCommandOption].c_str());
    
    ASSERT_STREQ(".mkv", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());
   
}

//
// Command fpe --task 1 --quiet  --watch /home/pi/watchstuff/watch/ --destination /home/pi/watchstuff/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionQuiet) {

    FPEOptions optionData;

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

    optionData = fetchCommandLineOptionData(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_TRUE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));
    
    ASSERT_STREQ("Video Conversion" , optionData.taskFunc.name.c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ("/home/pi/watchstuff/watch/", optionData.optionsMap[kWatchOption].c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", optionData.optionsMap[kDestinationOption].c_str());
    ASSERT_STREQ("/usr/local/bin/HandBrakeCLI -i %1% -o %2% --preset=\"Normal\"", optionData.optionsMap[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.optionsMap[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kServerOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kUserOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.optionsMap[kLogOption].c_str());
    
}

// =====================
// RUN GOOGLE UNIT TESTS
// =====================

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}