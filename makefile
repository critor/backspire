DEBUG = FALSE

GCC = nspire-gcc
AS  = nspire-as
GXX = nspire-g++
LD  = nspire-ld
GENZEHN = genzehn

GCCFLAGS = -Wall -W -marm
LDFLAGS =
ZEHNFLAGS = --name "backSpire" --240x320-support true

ifeq ($(DEBUG),FALSE)
	GCCFLAGS += -Os
else
	GCCFLAGS += -O0 -g
endif

OBJS = $(patsubst %.c, %.o, $(shell find . -name \*.c))
OBJS += $(patsubst %.cpp, %.o, $(shell find . -name \*.cpp))
OBJS += $(patsubst %.S, %.o, $(shell find . -name \*.S))
EXE = backspire
DISTDIR = .
vpath %.tns $(DISTDIR)
vpath %.elf $(DISTDIR)

all: $(EXE).tns

%.o: %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

%.o: %.cpp
	$(GXX) $(GCCFLAGS) -c $< -o $@
	
%.o: %.S
	$(AS) -c $< -o $@

$(EXE).elf: $(OBJS)
	mkdir -p $(DISTDIR)
	$(LD) $^ -o $@ $(LDFLAGS)

$(EXE).tns: $(EXE).elf
	$(GENZEHN) --input $^ --output $@.zehn $(ZEHNFLAGS)
	make-prg $@.zehn $@
	rm $@.zehn

clean:
	rm -f $(OBJS) $(DISTDIR)/$(EXE).tns $(DISTDIR)/$(EXE).elf $(DISTDIR)/$(EXE).zehn
