
CFLAGS+=-Wall -pedantic -std=c99
LFLAGS+=-lm

ifeq ($(MAKECMDGOALS),debug)
  CFLAGS+=-g -D_DEBUG_ -D_NOASM_
  BDIR=build/dbg
else
  CFLAGS+=-O4 -D_RELEASE_
  BDIR=build/rel
endif

OBJS=$(SRCS:%.c=$(BDIR)/%.o)
DEPS=$(SRCS:%.c=$(BDIR)/%.d)

$(shell mkdir -p $(BDIR))

all: $(PRG)
debug: all

-include $(DEPS)

$(PRG): Makefile $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $@

$(BDIR)/%.o: Makefile %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $*.c -o $@

$(BDIR)/%.d: Makefile %.c
	$(CC) -MM -MP -MT $(BDIR)/$*.o -MT $@ $(CFLAGS) -MF $@ $*.c

clean:
	rm -fr $(PRG) build

install:
	install -D $(PRG) $(DESTDIR)/usr/bin/$(PRG)

uninstall:
	rm -r $(DESTDIR)/usr/bin/$(PRG)

