OBJS=Main.o $(BITMAPS) soundbank.o
OBJS7=Main.arm7.o
LIBS=-L$(DEVKITPRO)/libnds/lib -L$(DEVKITPRO)/maxmod/lib -lnds9 -lm -lmm9
LIBS7=-L$(DEVKITPRO)/libnds/lib -lnds7 -lm

BITMAPS=gfx/pucmcawesome.o gfx/bgtop.o gfx/bgbottom.o gfx/girder.o gfx/png_shared.o gfx/ghosties.o gfx/bgtop_menu.o gfx/font.o

NAME=PucMcAwesome
DEFINES=

CC=$(DEVKITARM)/bin/arm-eabi-gcc
AS=$(DEVKITARM)/bin/arm-eabi-as
LD=$(DEVKITARM)/bin/arm-eabi-gcc
CFLAGS=-std=gnu99 -O3 -mcpu=arm9e -mtune=arm9e -fomit-frame-pointer -ffast-math \
-mthumb -mthumb-interwork -I$(DEVKITPRO)/libnds/include -I$(DEVKITPRO)/maxmod/include -DARM9 $(DEFINES)
CFLAGSARM=-std=gnu99 -O3 -mcpu=arm9e -mtune=arm9e -fomit-frame-pointer -ffast-math \
-mthumb-interwork -I$(DEVKITPRO)/libnds/include -I$(DEVKITPRO)/maxmod/include -DARM9 $(DEFINES)
CFLAGS7=-std=gnu99 -Os -mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer -ffast-math \
-mthumb -mthumb-interwork -I$(DEVKITPRO)/libnds/include -I$(DEVKITPRO)/maxmod/include -DARM7 $(DEFINES)
LDFLAGS=-specs=ds_arm9.specs -mthumb -mthumb-interwork -mno-fpu
LDFLAGS7=-specs=./ds_arm7.specs -mthumb-interwork -mno-fpu

.SUFFIXES: .o .png
.png.o :
	$(DEVKITARM)/bin/grit $< -ftc -o$<.c
	$(CC) -c $<.c -o $@

$(NAME).nds: $(NAME).arm9 $(NAME).arm7
	$(DEVKITARM)/bin/ndstool -c $@ -9 $(NAME).arm9

$(NAME).arm9: $(NAME).arm9.elf
	$(DEVKITARM)/bin/arm-eabi-objcopy -O binary $< $@

$(NAME).arm7: $(NAME).arm7.elf
	$(DEVKITARM)/bin/arm-eabi-objcopy -O binary $< $@

$(NAME).arm9.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(NAME).arm7.elf: $(OBJS7)
	$(LD) $(LDFLAGS7) -o $@ $(OBJS7) $(LIBS7)

Main.arm7.o: Main.arm7.c
	$(CC) $(CFLAGS7) -c -o $@ $<

	
clean:
	rm -f $(NAME).nds $(NAME).arm9 $(NAME).arm7 $(NAME).arm9.elf $(NAME).arm7.elf $(OBJS) $(OBJS7) $(BITMAPS) gfx/*.c gfx/*.h gfx/*.s *~

test: $(NAME).nds
	/usr/bin/wine $(DEVKITPRO)/nocash/NOCASH.EXE $(NAME).nds

todisk: $(NAME).nds
	/bin/mkdir disk
	/usr/bin/sudo mount /dev/mmcblk0p1 disk
	/usr/bin/sudo cp $(NAME).nds disk/Homebrew/Development/
	/usr/bin/sudo umount disk
	/bin/rm -rf diskw

cleanimages:
	rm -f $(BITMAPS) gfx/*.c gfx/*.h

Main.arm7.o: Main.arm7.c
Main.o: Main.c $(BITMAPS)
