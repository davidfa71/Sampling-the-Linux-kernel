_OBJ=Logic2BDD.o Transform.o simplify.o FastSifter.o synExp.o mytime.o bddBuilder.o humanNums.o eval.o cuddAdapter.o constraintSorter.o minspanSorter.o RememberSorter.o maxOccSorter.o expSorter.o MultiComponents.o OneComponent.o compSorter.o DynamicScorer.o  synExpParser.o nullSorter.o VariableEntry.o VariableTable.o smartspanSorter.o

OBJ = $(patsubst %,../obj/%,$(_OBJ))
DEP = $(OBJ:.o=.d)  # one dependency file for each source


INCLUDEFLAGS=-I../include -I../tools/src -Isrc -I/opt/local/include -I/usr/local/include 

LDFLAGS=-lcudd -L../lib -L/usr/local/lib -L/opt/local/lib 

include ../Makefile.os

../bin/Logic2BDD: ../obj/synExpScanner.o ../obj/synExpDriver.o ${OBJ} 
	@echo \\t"[CXX]  " \\t$@
	@$(CC) $(CXXFLAGS) -o $@ $^ $(INCLUDEFLAGS) $(LDFLAGS)


.PHONY: clean
clean:
	-@rm -f ${OBJ} ${DEP}  ../bin/Logic2BDD src/synExpParser.hpp src/synExpScanner.cpp src/synExpParser.output src/synExpParser.cpp src/location.hh src/stack.hh src/position.hh

include ../Makefile.rules
