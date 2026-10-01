# PS2 Open-World Racer — Grok starter
# Compile with ./scripts/build.sh (Docker + h4570/tyra)

TARGET      := racer.elf
ENGINEDIR   := vendor/tyra/engine

SRCDIR      := src
INCDIR      := inc
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res
SRCEXT      := cpp
VSMEXT      := vsm
VCLEXT      := vcl
VCLPPEXT    := vclpp
DEPEXT      := d
OBJEXT      := o

CFLAGS      := -D_EE -Wall -O3
LIB         := -ltyra
LIBDIRS     := -L$(ENGINEDIR)/bin
INC         := -I$(INCDIR) -I$(ENGINEDIR)/inc
INCDEP      := -I$(INCDIR) -I$(ENGINEDIR)/inc

include vendor/tyra/Makefile.base

clean-engine:
	cd $(ENGINEDIR) && $(MAKE) cleaner

build-engine:
	cd $(ENGINEDIR) && $(MAKE)
