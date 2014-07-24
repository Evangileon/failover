# Author Jun Yu

EXECUTABLE := masterfail
LIBS :=
CC := g++
LD := ld


DEBUG_MODE := y

ifeq ($(DEBUG_MODE), y)
	DEBUG := -g
	MACRO := -D__DEBUG__
else
	DEBUG :=
	MACRO :=
endif

LIB += -L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/i386-linux-gnu/ -pthread
CFLAGS := $(DEBUG) $(MACRO) -Wall
CXXFLAGS := $(CFLAGS) -std=c++0x
LDFLAGS := $(DEBUG) $(LIB)

RM-F := rm -f

SOURCE := $(wildcard *.c) $(wildcard *.cpp)
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
CPPFLAGS +=

.PHONY : everything deps objs clean veryclean rebuild

everything : $(EXECUTABLE)


objs : $(OBJS)

clean :
	@$(RM-F) *.o
	@$(RM-F) *.d

veryclean: clean
	@$(RM-F) $(EXECUTABLE)

rebuild: veryclean everything


-include $(DEPS)

$(EXECUTABLE) : $(OBJS)
	$(CC) -o $(EXECUTABLE) $(OBJS) $(LIB) $(addprefix -l,$(LIBS))
