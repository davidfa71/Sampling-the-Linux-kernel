_OBJ=Tseitin.o synExp.o synExpParser.o 

OBJ = $(patsubst %,../obj/%,$(_OBJ))
DEP = $(OBJ:.o=.d)  # one dependency file for each source


INCLUDEFLAGS=-I../include  -I../tools/src -Isrc -I/opt/local/include -I/usr/local/include 

LDFLAGS=-lgmp -lgmpxx -lcudd -L../lib -L/usr/local/lib -L/opt/local/lib 

include ../Makefile.os

../bin/Tseitin: ../obj/synExpScanner.o ../obj/synExpDriver.o ${OBJ} 
	@echo \\t"[CXX]  " \\t$@
	@$(CC) $(CXXFLAGS) -o $@ $^ $(INCLUDEFLAGS) $(LDFLAGS)
	@rm src/location.hh src/position.hh src/stack.hh src/synExpParser.cpp src/synExpParser.hpp src/synExpParser.output src/synExpScanner.cpp


.PHONY: clean
clean:
	-@rm -f ${OBJ} ${DEP}  ../bin/Tseitin src/synExpParser.hpp src/synExpScanner.cpp src/synExpParser.output src/synExpParser.cpp src/location.hh src/stack.hh src/position.hh ../obj/synExpScanner.o ../obj/synExpDriver.o synExpParser.output

include ../Makefile.rules
