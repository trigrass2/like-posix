###########################
# requires "libgtest"
###########################

TEST_DIR = .
SRC_DIR = ../wav
GTEST_DIR = /usr/lib
CPPFLAGS = -I../wav
CXXFLAGS = -g -Wall -Wextra -pthread
GTEST_LIBS = $(GTEST_DIR)/libgtest_main.a $(GTEST_DIR)/libgtest.a
#GTEST_LIBS = -lgtest_main -lgtest 

all :
	g++ $(CPPFLAGS) $(CXXFLAGS) $(TEST_DIR)/*.cc $(SRC_DIR)/*.c $(GTEST_LIBS) -o test
	
clean :
	rm -f test *.o *.xml
	
run :
	./test --gtest_output=xml:xunit.xml