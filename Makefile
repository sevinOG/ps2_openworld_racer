# PS2 Open-World Racer
# Built with Tyra + ps2dev. Run via ./scripts/build.sh (Docker).

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

CFLAGS      :=
LIB         := -ltyra
LIBDIRS     := -L$(ENGINEDIR)/bin
INC         := -I$(INCDIR) -I$(ENGINEDIR)/inc
INCDEP      := -I$(INCDIR) -I$(ENGINEDIR)/inc

include vendor/tyra/Makefile.base

clean-engine:
	cd $(ENGINEDIR) && $(MAKE) cleaner

build-engine:
	cd $(ENGINEDIR) && $(MAKE)

build-release-engine:
	cd $(ENGINEDIR) && $(MAKE) release
