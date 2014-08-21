# Author Jun Yu

EXECUTABLE := masterfail
LIBS :=
CC := clang++
LD := ld
DEBUG_MODE := y

ifeq ($(DEBUG_MODE), y)
	DEBUG := -g
	MACRO := -D__DEBUG__
	OPTIM :=
else
	DEBUG :=
	MACRO :=
	OPTIM := -O2
endif

LIB += -L/usr/lib/x86_64-linux-gnu/ -L/usr/lib/i386-linux-gnu/ -pthread
CFLAGS := $(DEBUG) $(MACRO) -Wall
CXXFLAGS := $(CFLAGS) $(OPTIM) -std=c++0x
LDFLAGS := $(DEBUG) $(LIB) $(OPTIM)

RM-F := rm -f

SOURCE := $(wildcard *.c) $(wildcard *.cpp)
OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) $(patsubst %.d,%.cpp,$(MISSING_DEPS)))
CPPFLAGS += -MD

.PHONY : everything deps objs clean veryclean rebuild

everything : $(EXECUTABLE)

deps : $(DEPS)

objs : $(OBJS)

clean :
	@$(RM-F) *.o
	@$(RM-F) *.d

veryclean: clean
	@$(RM-F) $(EXECUTABLE)

rebuild: veryclean everything

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)

$(EXECUTABLE) : $(OBJS)
	$(CC) -o $(EXECUTABLE) $(OBJS) $(LIB) $(OPTIM) $(addprefix -l,$(LIBS))
