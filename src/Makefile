#++++++++++++++++-I.
#.IDENTIFICATION Makefile.src
#.LANGUAGE       Make (makefile script)
#.AUTHOR         Francois Ochsenbein [CDS]
#.ENVIRONMENT    Unix
#.KEYWORDS       
#.VERSION  1.0   03-Oct-1991
#.VERSION  1.2   22-Apr-1999: GSC 1.2
#.VERSION  1.3   06-Sep-1999; GSC-ACT
#.PURPOSE        Template Makefile for Portable Packages
#.COMMENTS      
#	The "install" option in the Makefile copies the executables and 
#	man pages to public directories. Just edit the Makefile, and modify 
#	the definitions according to your installation.
#	Some alternatives are proposed as comments.
#----------------

#########################################
# There is a need to define GSCdat,
# the root of the directory structure
# containing the 9,000 files.
# That's just the parent directory!
GSCdat = `pwd`/..
#
#########################################
#
##### Turbo-C Definitions on PCs
#CC    = tcc -I. -mh -O -Z -G -N
#CLEAN = del *.obj
#SLIB  =
#DEL   = del
##### Unix definitions
SHELL = /bin/sh
CC    = cc -I. -O
CLEAN = strip a.out ; rm *.o ; mv a.out $@
SLIB  = -lm
COPY  = cp -p
DEL   = rm -f
#########################################

##### Files:
UTIL  = prtgsc.c dispos.c decode_c.c get_head.c find_reg.c \
	to_d.c dtos.c
PGMS  = gsc.exe decode.exe
PLATEPOCHS = ../plates.epoch

# Directories by Declination
GCD = N0000 N1500 N3000 N4500 N6000 N7500 N0730 N2230 N3730 N5230 N6730 N8230 \
      S0000 S1500 S3000 S4500 S6000 S7500 S0730 S2230 S3730 S5230 S6730 S8230 
CDROM = /tourCD/*

# DEPENDENCIES	--------------------------------

default: gsc.exe decode.exe

### Query Program:
gsc.exe: gsc.c $(UTIL) gsc.h
	$(CC) gsc.c $(UTIL) $(SLIB)
	$(CLEAN)

### Edit a full file to stdout:
decode.exe: decode.c decode_c.c get_head.c prtgsc.c dtos.c gsc.h
	$(CC) decode.c decode_c.c get_head.c prtgsc.c dtos.c $(SLIB)
	$(CLEAN)

### Generate the Index of Regions (nit required):
genreg.exe: genreg.c  get_head.c find_reg.c gsc.h
	$(CC) genreg.c get_head.c find_reg.c
	$(CLEAN)

### Relation Plate --> Epoch
epoch: epoch.c get_head.c gsc.h
	$(CC) epoch.c get_head.c
	$(CLEAN)

### Generate the Binary Files from FITS
encode: encode.c
	$(CC) encode.c -lm -o $@

#### Generate all Binary Files from FITS CDROMs
#### NOTE: A local copy (in directory src) can replace a corrupted CD-ROM file
#### (with uppercase letters)
phase1: encode epoch
	for d in $(CDROM)/gsc/[nsNS]???? ; do \
	    D=`echo $$d | rev | cut -d/ -f1 | rev | tr ns NS`; \
	    echo "....Directory: $$d ($$D)"; \
	    if [ -d ../$$D ]; then \
		echo "----Exists:  ../$$D" ; \
	    else \
		echo "....Creating ../$$D " ; \
		mkdir ../$$D ; \
	    fi ; \
	    if [ -r ../$$D/=done= ]; then ls -l ../$$D/=done= ; \
	    else \
		for f in $$d/* ; do \
		    F=`echo $$f | rev | cut -d/ -f1 | rev | tr nsgsc NSGSC`; \
		    if [ -f $$F ]; then f=$$F; else f=$$f; fi; \
		    ./encode < $$f > /tmp/$$F; \
		    ./epoch $(PLATEPOCHS) /tmp/$$F > ../$$D/$$F ; \
		    rm /tmp/$$F ; ls -l ../$$D/$$F ; \
		done ;  \
	        date > ../$$D/=done= ; \
	    fi ; \
	done

#### Phase 2 is just a check
phase2: 
	for d in $(GCD) ; do \
	    cat ../$$d/=done= || exit 1 ; \
	done

install: $(PGMS) genreg.exe phase2
	$(COPY) gsc.exe decode.exe ../bin
	GSCDAT=`pwd`/..; export GSCDAT; genreg.exe -b -c -d

clean:
	$(DEL) *.exe epoch encode core


#############################################################################
