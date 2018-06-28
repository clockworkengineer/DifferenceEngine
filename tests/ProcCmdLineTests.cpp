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

// Boost file system and format libraries

#include <boost/filesystem.hpp> 
#include <boost/format.hpp>

namespace fs = boost::filesystem;

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

        // Create watch folder.

        if (!fs::exists(ProcCmdLineTests::kWatchFolder)) {
            fs::create_directory(ProcCmdLineTests::kWatchFolder);
        }

        // Create destination folder.

        if (!fs::exists(ProcCmdLineTests::kDestinationFolder)) {
            fs::create_directory(ProcCmdLineTests::kDestinationFolder);
        }

    }

    virtual void TearDown() {

        // Remove watch folder.

        if (fs::exists(ProcCmdLineTests::kWatchFolder)) {
            fs::remove(ProcCmdLineTests::kWatchFolder);
        }

        // Remove destination folder.

        if (fs::exists(ProcCmdLineTests::kDestinationFolder)) {
            fs::remove(ProcCmdLineTests::kDestinationFolder);
        }

    }

    int argvLen(char *argv[]);

    static const std::string kWatchFolder; // Test Watch Folder
    static const std::string kDestinationFolder; // Test Destination folder

};

// =================
// FIXTURE CONSTANTS
// =================

const std::string ProcCmdLineTests::kWatchFolder("/tmp/watch");
const std::string ProcCmdLineTests::kDestinationFolder("/tmp/destination");

// ===============
// FIXTURE METHODS
// ===============

//
// Count length of argv to pass to fetchCommandLineOptionData.
//

int ProcCmdLineTests::argvLen(char *argv[]) {
    int argc = 0;
    while (argv[++argc] != NULL);
    return (argc);
}

// ==================================
// COMMAND LINE PROCESSING UNIT TESTS
// ==================================

//
// Process command line with no options
//

TEST_F(ProcCmdLineTests, NoOptions) {

    FPEOptions optionData;

    char *argv[] = {(char *) "fpe", nullptr};

    EXPECT_EXIT(optionData = fetchCommandLineOptions(this->argvLen(argv), argv),
            ::testing::ExitedWithCode(1), "FPE Error: the option '--task' is required but missing");

}

//
// Command fpe --task 0 --watch /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileNoDelete) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "0",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Copy File", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());
    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());


}

//
// Command fpe --delete --task 0 --watch /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, TaskCopyFileDelete) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--task",
        (char *) "0",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Copy File", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());
    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());


}

//
// Command fpe --task 1 --watch /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionNoDelete) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "1",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Video Conversion", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());
    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());



}

//
// Command fpe --delete --task 1 --watch /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionDelete) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--delete",
        (char *) "--task",
        (char *) "1",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Video Conversion", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());
    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());


}

//
// Command fpe --task 4 "echo %1% -------> %2%" --watch /tmp/watch/ --destination /tmp/destination/
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
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Run Command", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());
    ASSERT_STREQ("echo %1% -------> %2%", optionData.map[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());

}

//
// Command fpe --delete --task 4 "echo %1% -------> %2%" /tmp/watch/ --destination /tmp/destination/
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
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_TRUE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Run Command", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());
    ASSERT_STREQ("echo %1% -------> %2%", optionData.map[kCommandOption].c_str());

    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());

}

//
// Command fpe --task 0 --maxdepth 3 /tmp/watch/ --destination /tmp/destination/
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
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Copy File", optionData.action->getName().c_str());
    EXPECT_EQ(3, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());
    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());


}

//
// Command fpe --task 9 /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, InvalidTaskNumber) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "9",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    EXPECT_EXIT(optionData = fetchCommandLineOptions(this->argvLen(argv), argv),
            ::testing::ExitedWithCode(1), "FPE Error: Error invalid task number.");

}

//
// Command fpe --task e --delete /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, MultipleTasksSpecified) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "e",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    EXPECT_EXIT(optionData = fetchCommandLineOptions(this->argvLen(argv), argv),
            ::testing::ExitedWithCode(1), "FPE Error: task is not a valid integer.");

}

//
// Command fpe --delete --task 1 --extension ".mkv" --watch /tmp/watch/ --destination /tmp/destination/
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
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_FALSE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Video Conversion", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());
    ASSERT_STREQ(".mkv", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());

}

//
// Command fpe --task 1 --quiet  --watch /tmp/watch/ --destination /tmp/destination/
//

TEST_F(ProcCmdLineTests, TaskVideoFileConversionQuiet) {

    FPEOptions optionData;

    char *argv[] = {
        (char *) "fpe",
        (char *) "--task",
        (char *) "1",
        (char *) "--quiet",
        (char *) "--watch",
        (char *) ProcCmdLineTests::kWatchFolder.c_str(),
        (char *) "--destination",
        (char *) ProcCmdLineTests::kDestinationFolder.c_str(),
        nullptr
    };

    optionData = fetchCommandLineOptions(this->argvLen(argv), argv);

    EXPECT_FALSE(getOption<bool>(optionData, kDeleteOption));
    EXPECT_FALSE(getOption<bool>(optionData, kListOption));
    EXPECT_TRUE(getOption<bool>(optionData, kQuietOption));
    EXPECT_FALSE(getOption<bool>(optionData, kSingleOption));

    ASSERT_STREQ("Video Conversion", optionData.action->getName().c_str());
    EXPECT_EQ(-1, getOption<int>(optionData, kMaxDepthOption));
    EXPECT_EQ(0, getOption<int>(optionData, kKillCountOption));
    ASSERT_STREQ(ProcCmdLineTests::kWatchFolder.c_str(), optionData.map[kWatchOption].c_str());
    ASSERT_STREQ(ProcCmdLineTests::kDestinationFolder.c_str(), optionData.map[kDestinationOption].c_str());

    ASSERT_STREQ("", optionData.map[kCommandOption].c_str());
    ASSERT_STREQ("", optionData.map[kExtensionOption].c_str());
    ASSERT_STREQ("", optionData.map[kServerOption].c_str());
    ASSERT_STREQ("", optionData.map[kUserOption].c_str());
    ASSERT_STREQ("", optionData.map[kPasswordOption].c_str());
    ASSERT_STREQ("", optionData.map[kRecipientOption].c_str());
    ASSERT_STREQ("", optionData.map[kLogOption].c_str());

}

// =====================
// RUN GOOGLE UNIT TESTS
// =====================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}