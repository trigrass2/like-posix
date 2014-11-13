###########################
# requires "libgtest"
###########################

TEST_DIR = .
SRC_DIR = ..
GTEST_DIR = /usr/lib
CPPFLAGS = -I../
CXXFLAGS = -g -Wall -Wextra -pthread
GTEST_LIBS = $(GTEST_DIR)/libgtest_main.a $(GTEST_DIR)/libgtest.a
#GTEST_LIBS = -lgtest_main -lgtest 

all :
	g++ $(CPPFLAGS) $(CXXFLAGS) $(TEST_DIR)/*.cc $(SRC_DIR)/*.c $(GTEST_LIBS) -o test
	
clean :
	rm -f test *.o
	
run :
	./test