#################################################
#            Variables                          #
#################################################

# Directories
SRC = ./

# SET DEFAULTS (which can be overriden)
ifeq ($(origin CXX), undefined)
	CXX = g++
endif

ifeq ($(origin LDFLAGS), undefined)
	LDFLAGS = -g -Wall -lglog -O0 # -lserial
endif

ifeq ($(origin CXXFLAGS), undefined)
	CXXFLAGS = $(LDFLAGS) -DDEBUG=1 -MD # MD is required for auto dependency
endif


#################################################
#                    modules                    #
#################################################

# COMMON OBJECT FILES
COMMONOBJS = $(SRC)Disaggregate.o $(SRC)Signature.o

# COMPILATION RULES
###################

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


#################################################
#                  Clean                        #
#################################################

clean:
	rm -f *.o *.d