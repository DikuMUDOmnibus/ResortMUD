CC      = gcc
#PROF    = -p

#Uncomment to compile in Cygwin
#CYGWIN = -DCYGWIN

# Please read all the documents in the system dir and README.1st in main dir.
# By default, this MUD will start on port 4100.
 
# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket
 
#IMC2 - Comment out to disable IMC2 support
IMC = 1

#Code support for Intermud-3. Comment out to disable.
I3 = 1

W_FLAGS = -Wall -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls #-Wshadow

C_FLAGS = -g3 -Os $(W_FLAGS) $(PROF) $(SOLARIS_FLAG)
L_FLAGS = $(PROF) $(SOLARIS_LINK)
 
C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c arena.c \
          ban.c bank.c base64.c boards.c build.c clans.c comm.c \
          comments.c const.c db.c deity.c fight.c grub.c handler.c hashstr.c \
          hiscores.c hskelter.c imm_host.c immscore.c \
          interp.c locker.c magic.c makeobjs.c mapout.c medit.c misc.c motd.c \
          mpxset.c mud_comm.c mud_prog.c oedit.c planes.c player.c \
          polymorph.c redit.c reset.c save.c services.c sha256.c \
          shops.c skills.c special.c stat_obj.c suicide.c tables.c \
          track.c update.c websvr.c \
 
ifdef IMC
   C_FILES := imc.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCSMAUG
endif

ifdef I3
   C_FILES := i3.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DI3 -DI3SMAUG
endif

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h)

all:
	$(MAKE) -s rmexe

# pull in dependency info for *existing* .o files
-include dependencies.d

ifdef CYGWIN
rmexe: $(O_FILES)
	rm -f rmexe.exe
	$(CC) -o rmexe.exe $(O_FILES) $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w rmexe.exe
	chmod a+x rmexe.exe
	chmod g+w $(O_FILES)

clean:
	rm -f o/*.o rmexe.exe *~
else
rmexe: $(O_FILES)
	rm -f rmexe
	$(CC) -o rmexe $(O_FILES) $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w rmexe
	chmod a+x rmexe
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o rmexe *~
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
endif
 
indent:
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES)
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(H_FILES)

indentclean:
	rm *.c~ *.h~

o/%.o: %.c
	echo "ResortMUD is now compiling... $@";
	$(CC) -c $(C_FLAGS) $< -o $@
 
backup:
	rm -f o/*.o rmexe src.tar.gz
	tar -cvzf src.tar.gz *

restore:
	tar -xzvf src.tar.gz
	make
