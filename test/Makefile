###########################
# requires "libgtest"
# #########################



TEST_DIR = .
GTEST_DIR = /usr/local/lib/gtest-1.7.0
CPPFLAGS = -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) -I$(TEST_DIR)/fixtures
CXXFLAGS = -g -Wall -Wextra -pthread $(GTEST_DIR)/gtest.a

CPPFLAGS += -I../

test_strutils :
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c test_strutils.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ../strutils.c

TESTS = strutils

all : $(patsubst %,test_%,$(TESTS))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o test $(patsubst %,%.o,$(TESTS)) $(patsubst %,test_%.o,$(TESTS))
	
clean :
	rm -f test *.o
	
run :
	./test