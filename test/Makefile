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

TESTS = test_all
UNDERTEST = strutils minstdlib minstring minstdio

SOURCES = $(patsubst %,$(SRC_DIR)/%.c,$(UNDERTEST)) $(patsubst %,$(TEST_DIR)/%.cc,$(TESTS))
OBJECTS = $(patsubst %,%.o,$(UNDERTEST) $(TESTS))

all :
	g++ $(CPPFLAGS) $(CXXFLAGS) $(TEST_DIR)/*.cc $(SRC_DIR)/*.c $(GTEST_LIBS) -o test
	
#objs: 
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(SOURCES)
#	
#all : objs
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJECTS) $(GTEST_LIBS) -o test 
	
clean :
	rm -f test *.o
	
run :
	./test