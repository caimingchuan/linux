
CC:=qcc -Vgcc_ntoarmv7le

HEAD := $(wildcard *.h)

SRCS := $(wildcard *.c)

OBJS := $(SRCS:.c=.o)

OBJD := $(SRCS:.c=.d)

TARGET := $(shell pwd | sed 's/\(.*\)[/]//g')

INCLUDE_PATH += /home/caimingchuan/work/qnx660/target/qnx6/usr/include /home/caimingchuan/work/qnx660/target/qnx6/usr/include/io-pkt .

LIB_PATH += /home/caimingchuan/work/qnx660/target/armle-v7/lib /home/caimingchuan/work/qnx6/target/armle-v7/usr/lib .

ifeq (1, $(DEBUG))
	CFLAGS += -DDEBUG
endif

LIB += socket 

libopts += $(addprefix -l, $(LIB))

CFLAGS += $(foreach dir, $(INCLUDE_PATH), -I$(dir))

CFLAGS += $(foreach lib, $(LIB_PATH), -L$(lib))

CFLAGS += -DDEBUG

CFLAGS += -DQNX_SYSTEM

flag = $(shell echo $(CFLAGS) | grep "\-fPIC")$(shell echo $(CFLAGS) | grep "\-shared")

ifeq (, $(flag))
	target = $(TARGET)
else
	target = lib$(TARGET).so
endif

all: $(SRCS) $(HEAD)
	$(CC)  $(CFLAGS) -c $(SRCS) $(libopts)
	$(CC)  $(CFLAGS) $(OBJS) -o $(target) $(libopts)
	@-rm $(OBJS)

%o: %c
	$(CC)  $(CFLAGS) -c %c $(libopts)

debug: $(SRCS) $(HEAD)
	@-$(CC) -DDEBUG $(CFLAGS) -c $(SRCS) $(libopts)
	$(CC) -DDEBUG $(CFLAGS) $(OBJS) -o $(target) $(libopts)
	@-rm $(OBJS)

	
cp:
	cp $(target) ~/work/
	cp $(target) ~/windows/S7DL/
	
clean:
	-rm ~/work/$(target) $(target)
