# Generated automatically from Makefile.in by configure.
# -*- mode: Makefile; -*-
##############################################################################
#
# Copyright (c) 2000, Johan Bengtsson
# 
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
##############################################################################

# Programs to use.
CC   = gcc
LD   = $(CC)
INSTALL = /usr/bin/install -c

# Options passed to the compiler and linker. 
EXTRAFLAGS   = -Wall
EXTRALDFLAGS = 
DEFS         =

# Where to find extra include files.
INCDIR =

# Extra libraries. 
LIBS = 

# The name of the executable.
PROG = ox_perf

# Source files.
SRCS = memtime.c linux.c

# ---- No changes are needed below this line ----

prefix = /usr/local
exec_prefix=${prefix}
bindir = ${exec_prefix}/bin
mandir = ${prefix}/man
srcdir = .

CFLAGS =  -g -O2 $(EXTRAFLAGS)
CPPFLAGS = $(INCDIR) $(DEFS) 
LDFLAGS = $(EXTRALDFLAGS) 


EXE=$(PROG)

OBJS= $(addsuffix .o,$(basename $(SRCS)))
DEPS= $(addsuffix .d,$(basename $(SRCS)))

%.d: %.c
	$(CC) -MM $(CPPFLAGS) $< | sed -e "s/$*\.o/$*.o $@/" > $@

$(EXE): $(OBJS)
	$(LD) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBS)

clean:
	rm -f *.o core *~ *.bak

mostclean: clean 
	rm -f $(EXE) *.d

all: $(EXE)

install: all
	$(INSTALL) $(EXE) $(bindir)

.PHONY:	clean mostclean install

include $(DEPS)

