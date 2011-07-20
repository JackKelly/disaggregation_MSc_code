#################################################
#            Variables                          #
#################################################

# Directories
SRC = ./src/
TEST = ./tests/

# SET DEFAULTS (which can be overriden)
ifeq ($(origin CXX), undefined)
	CXX = g++
endif

ifeq ($(origin LDFLAGS), undefined)
	LDFLAGS = -g -Wall -lglog -O0 -std=c++0x # -lserial
endif

ifeq ($(origin CXXFLAGS), undefined)
	CXXFLAGS = $(LDFLAGS) -DDEBUG=1 -MD # MD is required for auto dependency
endif


#################################################
#                    modules                    #
#################################################

# COMMON OBJECT FILES
COMMONOBJS = $(SRC)Disaggregate.o $(SRC)Signature.o $(SRC)Utils.o $(SRC)Device.o $(SRC)GNUplot.o

#####################
# COMPILATION RULES #
#####################

disaggregate: $(COMMONOBJS)
	$(CXX) -o $@ $(COMMONOBJS) $(INC) $(LDFLAGS) -lm

# GENERIIC COMPILATION RULE
.C.o:
	$(CXX) $< -c $(CXXFLAGS) $(INC)
	
# TESTING
TESTCXXFLAGS = -g -Wall -std=c++0x -lboost_unit_test_framework

testAll: ArrayTest GNUplotTest

ArrayTest: $(TEST)ArrayTest.cpp $(SRC)Array.h $(SRC)Utils.o
	g++ $(TESTCXXFLAGS) -o $(TEST)ArrayTest $(TEST)ArrayTest.cpp $(SRC)Utils.o && $(TEST)ArrayTest 

GNUplotTest: $(TEST)GNUplotTest.cpp $(SRC)GNUplot.o
	g++ $(TESTCXXFLAGS) -o $(TEST)GNUplotTest $(TEST)GNUplotTest.cpp $(SRC)GNUplot.o && $(TEST)GNUplotTest

# AUTOMATIC DEPENDENCY DETECTION
# http://www.wlug.org.nz/MakefileHowto
# also take a look at http://lear.inrialpes.fr/people/klaeser/software_makefile_link_dependencies

DEPS := $(patsubst %.o,%.d,$(COMMONOBJS))

-include $(DEPS)


#################################################
#                  Clean                        #
#################################################

clean:
	rm -f $(SRC)*.o $(SRC)*.d
	
linecount:
	wc -l $(SRC)*.cpp $(SRC)*.h $(TEST)*.cpp data/diagrams/hist_and_states_and_RAgradient/*.gnu