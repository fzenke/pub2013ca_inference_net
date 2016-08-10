CC = /usr/lib64/mpich2/bin/mpicxx 
CC = mpicxx 
# Your Auryn install path
# Update the following lines to your system requirements
AURYNPATH=$(HOME)/auryn

# Path to Auryn include files
AURYNINC=$(AURYNPATH)/src

# Path to Auryn library
AURYNLIB=$(AURYNPATH)/build/release/src

# The following should not require updating in most cases 
CC = mpic++
CFLAGS=-ansi -pipe -O3 -march=native -ffast-math -pedantic -I/usr/include -I$(AURYNINC)
LDFLAGS=-lboost_program_options -lboost_mpi -lboost_serialization -L$(AURYNLIB) -lauryn

all: sim.timestamp

sim.timestamp: sim_lgnet ratemod.dat
	./sim_lgnet
	> $<

ratemod.dat: ratemod.dat.gz
	gunzip -c $< > $@

sim_%: sim_%.o $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $< $(LDFLAGS) -o $(subst .o,,$<)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f sim_lgnet ratemod.dat data/lgnet.*
