#
# -------------------------- LICENSE -----------------------------------
#
#  This file is part of the LibSQUID software libraray.
#
#  LibSQUID is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  LibSQUID is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with LibSQUID.  If not, see <http://www.gnu.org/licenses/>.
#
#  Copyright 2014 James Wren and Los Alamos National Laboratory
# 

TARGET_SOURCES = libsquid_wcs libwcsxy
TARGET_OBJECTS = $(patsubst %, %.o, $(TARGET_SOURCES))

GCC     = gcc
CFLAGS  = -g -Wall -fPIC -I. -I../libsquid \
	$(shell pkg-config --cflags cfitsio) \
	$(shell pkg-config --cflags wcslib)
LDFLAGS = -L. -L../libsquid -lsquid -lsquid_wcs -lm \
	$(shell pkg-config --libs cfitsio) \
	$(shell pkg-config --libs wcslib)
LDFLAGS_STATIC = -L. -L../libsquid -lm \
	-Wl,-Bstatic -lsquid -lsquid_wcs \
	-Wl,-Bdynamic \
	$(shell pkg-config --libs cfitsio) \
	$(shell pkg-config --libs wcslib)

all: $(TARGET_OBJECTS) libsquid_wcs.a libsquid_wcs.so bin

%.o: %.c
	$(GCC) -c $(CFLAGS) -o $@ $<

libsquid_wcs.a: $(TARGET_OBJECTS)
	ar cq $@ $^

libsquid_wcs.so : $(TARGET_OBJECTS)
	$(GCC) -shared -fPIC -o $@ $^

bin: libsquid_wcs.a libsquid_wcs.so
	$(MAKE) -C bin

clean:
	rm -f *.o *.a *.so
	$(MAKE) -C bin clean

