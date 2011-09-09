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
	LDFLAGS = -g -Wall -std=c++0x -O3 -lboost_program_options # -lserial -lglog -L/usr/local/lib
endif

ifeq ($(origin CXXFLAGS), undefined)
	CXXFLAGS = $(LDFLAGS) -DDEBUG=1 -MD -Wno-deprecated -Wno-unused-result # -O2 PUT this back after testing 
	# -MD is required for auto dependency, 
	# -O2 and -Wno-deprecated are required for Boost Graph Library.
	# -Wno-unused-result is required to stop warnings from GNUplot about ignoring
	#    the returned int from system().
endif


#################################################
#                    modules                    #
#################################################

# COMMON OBJECT FILES
COMMONOBJS = $(SRC)Main.o $(SRC)Signature.o $(SRC)Utils.o $(SRC)Device.o \
 $(SRC)GNUplot.o $(SRC)PowerStateSequence.o $(SRC)AggregateData.o $(SRC)PowerStateGraph.o

#####################
# COMPILATION RULES #
#####################

disaggregate: $(COMMONOBJS)
	$(CXX) -o $@ $(COMMONOBJS) $(INC) $(LDFLAGS) -lm

# GENERIIC COMPILATION RULE
.C.o:
	$(CXX) $< -c $(CXXFLAGS) $(INC)
	
# AUTOMATIC DEPENDENCY DETECTION
# http://www.wlug.org.nz/MakefileHowto
# also take a look at http://lear.inrialpes.fr/people/klaeser/software_makefile_link_dependencies

DEPS := $(patsubst %.o,%.d,$(COMMONOBJS))

-include $(DEPS)
	
# TESTING (it's best to do a 'make clean' when switching between testing and normal compiling because object files are compiled with different options)
TESTCXXFLAGS = -g -Wall -std=c++0x -lboost_unit_test_framework -MD # -DGOOGLE_STRIP_LOG=4 

testAll: ArrayTest GNUplotTest UtilsTest StatisticTest PowerStateGraphTest

ATOBJFILES = $(SRC)Utils.o $(SRC)GNUplot.o
ArrayTest: CXXFLAGS = $(TESTCXXFLAGS)
ArrayTest: $(TEST)ArrayTest.cpp $(SRC)Array.h $(ATOBJFILES)  
	g++ $(TESTCXXFLAGS) -o $(TEST)ArrayTest $(TEST)ArrayTest.cpp $(ATOBJFILES) && $(TEST)ArrayTest 

GPTOBJFILES = $(SRC)GNUplot.o $(SRC)Utils.o
GNUplotTest: CXXFLAGS = $(TESTCXXFLAGS) -Wno-unused-result
GNUplotTest: $(TEST)GNUplotTest.cpp $(GPTOBJFILES) 
	g++ $(CXXFLAGS) -o $(TEST)GNUplotTest $(TEST)GNUplotTest.cpp $(GPTOBJFILES) && $(TEST)GNUplotTest

UTOBJFILES = $(SRC)Utils.o
UtilsTest: CXXFLAGS = $(TESTCXXFLAGS)
UtilsTest: $(TEST)UtilsTest.cpp $(UTOBJFILES)
	g++ $(CXXFLAGS) -o $(TEST)UtilsTest $(TEST)UtilsTest.cpp $(UTOBJFILES) && $(TEST)UtilsTest

STOBJFILES = $(SRC)Utils.o $(SRC)GNUplot.o
StatisticTest: CXXFLAGS = $(TESTCXXFLAGS)
StatisticTest: $(TEST)StatisticTest.cpp $(SRC)Statistic.h $(SRC)Array.h $(STOBJFILES)
	g++ $(CXXFLAGS) -o $(TEST)StatisticTest $(TEST)StatisticTest.cpp $(STOBJFILES)  && $(TEST)StatisticTest

PSGTOBJFILES = $(SRC)PowerStateGraph.o $(SRC)Signature.o $(SRC)GNUplot.o $(SRC)Utils.o $(SRC)PowerStateSequence.o $(SRC)AggregateData.o
PowerStateGraphTest: CXXFLAGS = $(TESTCXXFLAGS) -Wno-deprecated -Wno-unused-result -O3
PowerStateGraphTest: $(TEST)PowerStateGraphTest.cpp $(SRC)Array.h $(PSGTOBJFILES)
	g++ $(CXXFLAGS) -o $(TEST)PowerStateGraphTest $(PSGTOBJFILES) $(TEST)PowerStateGraphTest.cpp && $(TEST)PowerStateGraphTest

ADTOBJFILES = $(SRC)AggregateData.o $(SRC)GNUplot.o $(SRC)Utils.o
AggregateDataTest: CXXFLAGS = $(TESTCXXFLAGS) -Wno-deprecated -Wno-unused-result
AggregateDataTest: $(TEST)AggregateDataTest.cpp $(SRC)Array.h $(ADTOBJFILES)
	g++ $(CXXFLAGS) -o $(TEST)AggregateDataTest $(ADTOBJFILES) $(TEST)AggregateDataTest.cpp && $(TEST)AggregateDataTest


#################################################
#                  Clean                        #
#################################################

clean:
	rm -f $(SRC)*.o $(SRC)*.d $(TEST)*.o $(TEST)*.d
	
linecount:
	wc -l $(SRC)*.cpp $(SRC)*.h $(TEST)*.cpp config/*.* Makefile

	

