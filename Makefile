#Compiler and Linker
CC          := clang++

#The Target Binary Program
TARGET      := garbage_collect
TESTTARGET  := garbage_collect_test

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
TESTDIR     := test
INCDIR      := inc
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res
SRCEXT      := cpp
DEPEXT      := d
OBJEXT      := o

current_dir = $(shell pwd)

#Flags, Libraries and Includes
CXXSTD      := -std=c++11 -Wno-deprecated-register
CFLAGS      := $(CXXSTD) -fopenmp -Wall -O3 -g
#LIB         := -fopenmp -lm -larmadillo
INC         := -I$(INCDIR) -Isrc -Isrc/test -I../ -I/usr/local/opt/flex/include
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))
MAINOBJS    := $(shell echo $(OBJECTS) | sed 's/[^ ]*test[^ ]* *//g')
TESTOBJS    := $(filter-out $(MAINOBJS), $(OBJECTS))

#Defauilt Make
all: directories $(TARGET) $(TESTTARGET)

#Remake
remake: cleaner all

#Copy Resources from Resources Directory to Target Directory
resources: directories
	@cp $(RESDIR)/* $(TARGETDIR)/

#Make the Directories
directories:
	mkdir -p $(TARGETDIR)
	mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
	$(RM) -rf $(BUILDDIR)
	$(RM) -rf $(TARGETDIR)

#Clean only Objecst
watch: $(SRCDIR)
	@fswatch -o $^/*.cpp | xargs -I{} make 


#Full Clean, Objects and Binaries
cleaner: clean
	$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(filter-out $(TESTOBJS),$(OBJECTS))
	$(CC) -o $(TARGETDIR)/$(TARGET) $^

$(TESTTARGET): $(filter-out $(BUILDDIR)/$(TARGET).$(OBJEXT),$(OBJECTS))
	$(CC) -o $(TARGETDIR)/$(TESTTARGET) $^

#Compile src
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	$(CC) $(CFLAGS) $(INC) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT) 
	cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all remake clean cleaner