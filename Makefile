#makefile 

CC   =   g++

#UCFLAGS = -O0 -g3 -Wall -gstabs+  
UCFLAGS = -O3 -Wall -gstabs+ -fpic 


RUCFLAGS := $(shell root-config --cflags) -I./include/ -I${CMSSW_BASE}/src/ -I${CMSSW_RELEASE_BASE}/src/ 
LIBS :=  $(shell root-config --libs) -lTreePlayer -L${CMSSW_BASE}/lib/slc6_amd64_gcc481/ -L${CMSSW_RELEASE_BASE}/lib/slc6_amd64_gcc481/ -lTreePlayer -lDataFormatsL1TCalorimeter -lDataFormatsL1Trigger -lCondFormatsL1TObjects -lL1TriggerL1TCalorimeter
GLIBS := $(shell root-config --glibs)

vpath %.cpp ./src

SRCPP = L1TStage2Wrapper.cpp


OBJCPP = $(patsubst %.cpp,lib/%.o,$(SRCPP))


all : lib/libL1TEmulatorWrapper.so

lib/%.o : %.cpp
	@echo "> compiling $*"
	@mkdir -p lib/
	@$(CC) -c $< $(UCFLAGS) $(RUCFLAGS) -o $@

lib/libL1TEmulatorWrapper.so : $(OBJCPP)
	@echo "> Creating library"
	@$(CC) -shared $^ $(ACLIBS) $(LIBS) $(GLIBS)  -o $@

clean:
	@echo "> Cleaning object files"
	@rm  -f lib/*.o
        
cleanall: clean
	@echo "> Cleaning library"
	@rm -f lib/libL1TEmulatorWrapper.so

