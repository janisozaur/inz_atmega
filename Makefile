###############################################################################
# Makefile for the project inz
###############################################################################

## General Flags
PROJECT = inz
MCU = atmega8
TARGET = $(PROJECT).elf
CC = avr-gcc
F_CPU = 8000000UL

## Compilation directories
SRCDIR = src
BUILDDIR = build
BINDIR = bin

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(BUILDDIR)/$(*F).o -MF dep/$(@F).d -DF_CPU=$(F_CPU)

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(BUILDDIR)/$(PROJECT).map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = main.o
PATH_OBJECTS = $(patsubst %.o, $(BUILDDIR)/%.o, $(OBJECTS))

## Objects explicitly added by the user
LINKONLYOBJECTS = 

FILES = $(TARGET) $(BUILDDIR)/$(PROJECT).hex $(BINDIR)/$(PROJECT).hex $(BUILDDIR)/$(PROJECT).eep $(BUILDDIR)/$(PROJECT).lss

## Build
all: init $(FILES) size

## Compile
main.o: $(SRCDIR)/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $(BUILDDIR)/$@

##Link
$(TARGET): $(OBJECTS)
	/bin/pwd
	$(CC) $(LDFLAGS) $(PATH_OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(BUILDDIR)/$(TARGET)
	cp $(BUILDDIR)/$(TARGET) $(BINDIR)/$(TARGET)

init:
	test -d $(BUILDDIR) || mkdir -p $(BUILDDIR)
	test -d $(BINDIR) || mkdir -p $(BINDIR)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $(BUILDDIR)/$< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $(BUILDDIR)/$< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $(BUILDDIR)/$< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} $(BINDIR)/${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(PATH_OBJECTS) $(FILES) $(BINDIR)/$(TARGET) $(BUILDDIR)/$(TARGET) $(BUILDDIR)/$(PROJECT).map $(patsubst %.o, dep/%.o.d, $(OBJECTS))
	-test -d dep && rmdir dep
	-test -d $(BUILDDIR) && rmdir $(BUILDDIR)
	-test -d $(BINDIR) && rmdir $(BINDIR)

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

