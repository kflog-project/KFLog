# $Id$

TARGETS=dataport_300_sv517_rev23.pdf

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

%.ps: %.txt
	rm -f $@
	a2ps -o $@ --medium=letter --border=no -1 -B $<

%.pdf: %.ps
	ps2pdf $< $@
