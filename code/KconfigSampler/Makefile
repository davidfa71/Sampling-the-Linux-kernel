_OBJ=KconfigSampler.o cuddAdapter.o humanNums.o mytime.o synExp.o randomAlgo.o TravInfo.o OneComponent.o MultiComponents.o  Traverser.o Dictionary.o VarData.o KeyValue.o
OBJ = $(patsubst %,../obj/%,$(_OBJ))
DEP = $(OBJ:.o=.d)  # one dependency file for each source


INCLUDEFLAGS=-I../include -I../tools/src -Isrc -I/opt/local/include -I/usr/local/include 

LDFLAGS=-lgmp -lgmpxx -lcudd -L../lib -L/usr/local/lib -L/opt/local/lib 

include ../Makefile.os

../bin/KconfigSampler: ${OBJ} 
	@echo \\t"[CXX]  " \\t$@
	@$(CC) $(CXXFLAGS) -o $@ ${OBJ}  $(INCLUDEFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	-@rm -f ${OBJ} ${DEP}  ../bin/KconfigSampler

include ../Makefile.rules
