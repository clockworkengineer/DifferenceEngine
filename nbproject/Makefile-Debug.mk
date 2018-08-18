#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Actions/CopyFile.o \
	${OBJECTDIR}/Actions/EmailFile.o \
	${OBJECTDIR}/Actions/ImportCSVFile.o \
	${OBJECTDIR}/Actions/RunCommand.o \
	${OBJECTDIR}/Actions/VideoConversion.o \
	${OBJECTDIR}/Actions/ZipFile.o \
	${OBJECTDIR}/FPE.o \
	${OBJECTDIR}/FPE_ProcCmdLine.o \
	${OBJECTDIR}/FPE_TaskActions.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f1

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/tests/ProcCmdLineTests.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../Antik/dist/Debug/GNU-Linux/libantik.a -lboost_system -lboost_filesystem -lboost_program_options -lpthread `pkg-config --libs libcurl` `pkg-config --libs zlib` `pkg-config --libs openssl`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/differenceengine

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/differenceengine: ../Antik/dist/Debug/GNU-Linux/libantik.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/differenceengine: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/differenceengine ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Actions/CopyFile.o: Actions/CopyFile.cpp
	${MKDIR} -p ${OBJECTDIR}/Actions
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/CopyFile.o Actions/CopyFile.cpp

${OBJECTDIR}/Actions/EmailFile.o: Actions/EmailFile.cpp
	${MKDIR} -p ${OBJECTDIR}/Actions
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/EmailFile.o Actions/EmailFile.cpp

${OBJECTDIR}/Actions/ImportCSVFile.o: Actions/ImportCSVFile.cpp
	${MKDIR} -p ${OBJECTDIR}/Actions
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/mongocxx/v_noabi -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/ImportCSVFile.o Actions/ImportCSVFile.cpp

${OBJECTDIR}/Actions/RunCommand.o: Actions/RunCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/Actions
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/RunCommand.o Actions/RunCommand.cpp

${OBJECTDIR}/Actions/VideoConversion.o: Actions/VideoConversion.cpp
	${MKDIR} -p ${OBJECTDIR}/Actions
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/VideoConversion.o Actions/VideoConversion.cpp

${OBJECTDIR}/Actions/ZipFile.o: Actions/ZipFile.cpp
	${MKDIR} -p ${OBJECTDIR}/Actions
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/ZipFile.o Actions/ZipFile.cpp

${OBJECTDIR}/FPE.o: FPE.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FPE.o FPE.cpp

${OBJECTDIR}/FPE_ProcCmdLine.o: FPE_ProcCmdLine.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FPE_ProcCmdLine.o FPE_ProcCmdLine.cpp

${OBJECTDIR}/FPE_TaskActions.o: FPE_TaskActions.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FPE_TaskActions.o FPE_TaskActions.cpp

# Subprojects
.build-subprojects:
	cd ../Antik && ${MAKE}  -f Makefile CONF=Debug

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/ProcCmdLineTests.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS}   -lgtest 


${TESTDIR}/tests/ProcCmdLineTests.o: tests/ProcCmdLineTests.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/ProcCmdLineTests.o tests/ProcCmdLineTests.cpp


${OBJECTDIR}/Actions/CopyFile_nomain.o: ${OBJECTDIR}/Actions/CopyFile.o Actions/CopyFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Actions
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Actions/CopyFile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/CopyFile_nomain.o Actions/CopyFile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Actions/CopyFile.o ${OBJECTDIR}/Actions/CopyFile_nomain.o;\
	fi

${OBJECTDIR}/Actions/EmailFile_nomain.o: ${OBJECTDIR}/Actions/EmailFile.o Actions/EmailFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Actions
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Actions/EmailFile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/EmailFile_nomain.o Actions/EmailFile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Actions/EmailFile.o ${OBJECTDIR}/Actions/EmailFile_nomain.o;\
	fi

${OBJECTDIR}/Actions/ImportCSVFile_nomain.o: ${OBJECTDIR}/Actions/ImportCSVFile.o Actions/ImportCSVFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Actions
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Actions/ImportCSVFile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/mongocxx/v_noabi -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/ImportCSVFile_nomain.o Actions/ImportCSVFile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Actions/ImportCSVFile.o ${OBJECTDIR}/Actions/ImportCSVFile_nomain.o;\
	fi

${OBJECTDIR}/Actions/RunCommand_nomain.o: ${OBJECTDIR}/Actions/RunCommand.o Actions/RunCommand.cpp 
	${MKDIR} -p ${OBJECTDIR}/Actions
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Actions/RunCommand.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/RunCommand_nomain.o Actions/RunCommand.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Actions/RunCommand.o ${OBJECTDIR}/Actions/RunCommand_nomain.o;\
	fi

${OBJECTDIR}/Actions/VideoConversion_nomain.o: ${OBJECTDIR}/Actions/VideoConversion.o Actions/VideoConversion.cpp 
	${MKDIR} -p ${OBJECTDIR}/Actions
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Actions/VideoConversion.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/VideoConversion_nomain.o Actions/VideoConversion.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Actions/VideoConversion.o ${OBJECTDIR}/Actions/VideoConversion_nomain.o;\
	fi

${OBJECTDIR}/Actions/ZipFile_nomain.o: ${OBJECTDIR}/Actions/ZipFile.o Actions/ZipFile.cpp 
	${MKDIR} -p ${OBJECTDIR}/Actions
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Actions/ZipFile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -I. -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Actions/ZipFile_nomain.o Actions/ZipFile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Actions/ZipFile.o ${OBJECTDIR}/Actions/ZipFile_nomain.o;\
	fi

${OBJECTDIR}/FPE_nomain.o: ${OBJECTDIR}/FPE.o FPE.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/FPE.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FPE_nomain.o FPE.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/FPE.o ${OBJECTDIR}/FPE_nomain.o;\
	fi

${OBJECTDIR}/FPE_ProcCmdLine_nomain.o: ${OBJECTDIR}/FPE_ProcCmdLine.o FPE_ProcCmdLine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/FPE_ProcCmdLine.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FPE_ProcCmdLine_nomain.o FPE_ProcCmdLine.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/FPE_ProcCmdLine.o ${OBJECTDIR}/FPE_ProcCmdLine_nomain.o;\
	fi

${OBJECTDIR}/FPE_TaskActions_nomain.o: ${OBJECTDIR}/FPE_TaskActions.o FPE_TaskActions.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/FPE_TaskActions.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -I. -I../Antik/include -I../Antik/classes/implementation -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FPE_TaskActions_nomain.o FPE_TaskActions.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/FPE_TaskActions.o ${OBJECTDIR}/FPE_TaskActions_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f1 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../Antik && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
