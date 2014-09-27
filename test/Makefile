###########################
# requires "libgtest"
# #########################



#TEST_DIR = .
#GTEST_DIR = /usr/lib/
#CPPFLAGS = 
#CXXFLAGS = -g -Wall -Wextra -pthread 
#GTEST_LIBS = $(GTEST_DIR)/libgtest_main.a $(GTEST_DIR)/libgtest.a
##GTEST_LIBS = -lgtest_main -lgtest 
#
#CPPFLAGS += -I../
#
#test_strutils :
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c test_minstdio.cc test_strutils.cc ../strutils.c ../minstdio.c ../minstring.c ../minstdlib.c
#
#TESTS =  minstdio strutils
#UNDERTEST =  minstdio minstring minstdlib strutils
#
#all : test_strutils
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(GTEST_LIBS) -o test $(patsubst %,%.o,$(UNDERTEST)) $(patsubst %,test_%.o,$(TESTS))
#	
#clean :
#	rm -f test *.o
#	
#run :
#	./test

TEST_DIR = .
SRC_DIR = ..
GTEST_DIR = /usr/lib
CPPFLAGS = -I../
CXXFLAGS = -g -Wall -Wextra -pthread 
GTEST_LIBS = $(GTEST_DIR)/libgtest_main.a $(GTEST_DIR)/libgtest.a
#GTEST_LIBS = -lgtest_main -lgtest 

TESTS = test_strutils 
UNDERTEST = strutils minstdlib minstring

SOURCES = $(patsubst %,$(SRC_DIR)/%.c,$(UNDERTEST)) $(patsubst %,$(TEST_DIR)/%.cc,$(TESTS))
OBJECTS = $(patsubst %,%.o,$(UNDERTEST) $(TESTS))

objs: 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(SOURCES)
	
all : objs
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(GTEST_LIBS) $(OBJECTS) -o test 
	
clean :
	rm -f test *.o
	
run :
	./test