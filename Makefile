MCU = attiny2313
FREQ=20000000
TRG = pid
BIN=/Users/rene/avr/bin
AVRLIB=/Users/rene/avr/avrlib
SRC = $(TRG).c
#$(AVRLIB)/uart.c $(AVRLIB)/buffer.c $(AVRLIB)/rprintf.c $(TRG).c

#additional includes to compile this is a gentoo bug fix
#http://bugs.gentoo.org/show_bug.cgi?id=147155
#INC	= /usr/i686-pc-linux-gnu/avr/lib

CFLAGS	= -g -Os -Wall -Wstrict-prototypes -I$(AVRLIB) -mcall-prologues -mmcu=$(MCU) -DF_CPU=$(FREQ) -DMCU=$(MCU)
LDFLAGS = -mmcu=$(MCU)

CC	= avr-gcc
RM	= rm -f
RN	= mv
CP	= cp
OBJCOPY	= avr-objcopy
SIZE	= avr-size
INCDIR = .
FORMAT = ihex	
SRC	+= $(addprefix $(AVRLIB)/,$(AVRLIB_SRC))
OBJ	= $(SRC:.c=.o)

all: $(TRG).elf $(TRG).bin $(TRG).hex $(TRG).eep $(TRG).ok
%.o : %.c 
	$(CC) -c $(CFLAGS) -I$(INCDIR) $< -o $@

%.s : %.c
	$(CC) -S $(CFLAGS) -I$(INCDIR) $< -o $@


%.elf: $(OBJ)
	$(CC) $(OBJ) $(LIB) $(LDFLAGS) -o $@
	
%.bin: %.elf
	$(OBJCOPY) -O binary -R .eeprom $< $@

%.hex: %.elf
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

%.eep: %.elf
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

$(TRG).o : $(TRG).c

%ok:
	$(SIZE) $(TRG).elf
	@echo "Errors: none" 

clean:
	$(RM) $(OBJ)
	$(RM) $(SRC:.c=.s)
	$(RM) $(SRC:.c=.lst)
	$(RM) $(TRG).map
	$(RM) $(TRG).elf
	$(RM) $(TRG).cof
	$(RM) $(TRG).obj
	$(RM) $(TRG).a90
	$(RM) $(TRG).sym
	$(RM) $(TRG).eep
	$(RM) $(TRG).hex
	$(RM) $(TRG).bin
	@echo "Errors: none"
	
size:
	$(SIZE) $(TRG).elf
load: $(TRG).hex
	avrdude -c usbtiny -p t2313 -U flash:w:pid.hex
# fuse byte settings: