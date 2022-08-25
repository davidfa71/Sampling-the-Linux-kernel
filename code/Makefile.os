ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
       CXXFLAGS=-std=c++11 -O2 -g -I. -I/usr/local/include
       LDFLAGS+= -lpthread
    endif
    ifeq ($(UNAME_S),Darwin)
        CXXFLAGS=-fcolor-diagnostics -O2 -std=c++11 -stdlib=libc++ -g
        # More stack size to avoid freetz translation from failing
        LDFLAGS+= -Wl,-stack_size,0x40000000,-stack_addr,0xf0000000
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif

CC=c++
