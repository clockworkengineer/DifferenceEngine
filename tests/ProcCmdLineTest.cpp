/* 
 * File:   ProcCmdLineTest.cpp
 * Author: Robert Tizzard
 *
 * Created on December 2, 2016, 3:30 PM
 */

#include "gtest/gtest.h"
#include "FPE_ProcCmdLine.hpp"

// Count length of argv to pass to procCmdLine.

int argvLen (char *argv[]) {
    int argc = 0; 
    while(argv[++argc] != NULL);
    return(argc);
}

TEST(ProcCmdLineTest, NoParameters) { 
    ParamArgData argData;
    ASSERT_EXIT (procCmdLine(0, nullptr, argData), ::testing::ExitedWithCode(1), "FPE Error: the option '--destination' is required but missing");
}

TEST(ProcCmdLineTest, TaskCopyFileNoDelete) {

    ParamArgData argData;
      
    char *argv[] = { (char *)"fpe", (char *)"--copy", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};
   
    procCmdLine(argvLen(argv), argv, argData);
    
    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    
}

TEST(ProcCmdLineTest, TaskCopyFileDelete) {

    ParamArgData argData;

    char *argv[] = { (char *)"fpe", (char *) "--delete", (char *)"--copy", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};

    procCmdLine(argvLen(argv), argv, argData);

    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    
}

TEST(ProcCmdLineTest, TaskVideoFileConversionNoDelete) {
    
    ParamArgData argData;
    
    char *argv[] = { (char *)"fpe", (char *)"--video", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};
 
    procCmdLine(argvLen(argv), argv, argData);
  
    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_TRUE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());

}

TEST(ProcCmdLineTest, TaskVideoFileConversionDelete) {

    ParamArgData argData;

    char *argv[] = { (char *)"fpe", (char *) "--delete", (char *)"--video", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};

    procCmdLine(argvLen(argv), argv, argData);

    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_TRUE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    
}

TEST(ProcCmdLineTest, TaskRunCommandCommandNoDelete) {
    
    ParamArgData argData;

    char *argv[] = { (char *)"fpe", (char *)"--command", (char *) "echo %1% -------> %2%", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};

    procCmdLine(argvLen(argv), argv, argData);

    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_TRUE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("echo %1% -------> %2%", argData.commandToRun.c_str());

}

TEST(ProcCmdLineTest, TaskRunCommandCommandDelete) {
    
    ParamArgData argData;
    
    char *argv[] = { (char *)"fpe", (char *)"--delete", (char *)"--command", (char *) "echo %1% -------> %2%", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};
 
    procCmdLine(argvLen(argv), argv, argData);
 
    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_FALSE(argData.bFileCopy);
    EXPECT_TRUE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
    ASSERT_STREQ("echo %1% -------> %2%", argData.commandToRun.c_str());

}

TEST(ProcCmdLineTest, TaskCopyFileMaxDepth3) {
    
    ParamArgData argData;

    char *argv[] = { (char *)"fpe", (char *)"--copy", (char*) "--maxdepth", (char *) "3", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};

    procCmdLine(argvLen(argv), argv, argData);
    
    EXPECT_FALSE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(3, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
}

TEST(ProcCmdLineTest, NoTaskDefaultCopy) {
    
    ParamArgData argData;
    
    char *argv[] = { (char *)"fpe", (char *) "--delete", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};
 
    procCmdLine(argvLen(argv), argv, argData);
 
    EXPECT_TRUE(argData.bDeleteSource);
    EXPECT_TRUE(argData.bFileCopy);
    EXPECT_FALSE(argData.bRunCommand);
    EXPECT_FALSE(argData.bVideoConversion);
    EXPECT_EQ(-1, argData.maxWatchDepth);
    ASSERT_STREQ("/home/pi/watchstuff/watch/", argData.watchFolder.c_str());
    ASSERT_STREQ("/home/pi/watchstuff/destination/", argData.destinationFolder.c_str());
 
}

TEST(ProcCmdLineTest, MultipleTasksSpecified) {
    
    ParamArgData argData;
    
    char *argv[] = { (char *)"fpe", (char *) "--copy", (char *) "--video", (char *) "--delete", (char *)"--watch", (char *)"/home/pi/watchstuff/watch/", (char *)"--destination", (char *)"/home/pi/watchstuff/destination/",  nullptr};
 
    ASSERT_EXIT (procCmdLine(argvLen(argv), argv, argData), ::testing::ExitedWithCode(1), "FPE Error: More than one task specified");
 
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}