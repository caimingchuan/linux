
CC:= gcc
HEAD := $(wildcard *.h)

SRCS := $(wildcard *.c)

OBJS := $(SRCS:.c=.o)

TARGET := calculate

INCLUDE_PATH += .

LIB_PATH += .

ifeq (1, $(DEBUG))
	CFLAGS += -DDEBUG
endif

LIB += 

libopts += $(addprefix -l, $(LIB))

CFLAGS += $(foreach dir, $(INCLUDE_PATH), -I$(dir))

CFLAGS += $(foreach lib, $(LIB_PATH), -L$(lib))

all: $(SRCS) $(HEAD)
	@-$(CC)  $(CFLAGS) -c $(SRCS) $(libopts)
	$(CC)  $(CFLAGS) $(OBJS) -o $(TARGET) $(libopts)
	@-rm $(OBJS)

%o: %c
	$(CC)  $(CFLAGS) -c %c $(libopts)

debug: $(SRCS) $(HEAD)
	@-$(CC) -DDEBUG $(CFLAGS) -c $(SRCS) $(libopts)
	$(CC) -DDEBUG $(CFLAGS) $(OBJS) -o $(TARGET) $(libopts)
	@-rm $(OBJS)

	
cp:
	cp $(TARGET) ~/work/
	
clean:
	-rm ~/work/$(TARGET) $(TARGET)
