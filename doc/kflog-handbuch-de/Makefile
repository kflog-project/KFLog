#
# General Makefile for LaTeX-files
#
# (c) Heiner Lamprecht
#
# $Id$
#

##################################################################
#
# Only the following variables have to be set:
#
# The name of the main-file:
BASE = KFlog_Handbuch
#
# Check all TeX-Files in the current directory
OBJECTS = *.tex
#
# Default. It's either 'ps' or 'dvi' or 'pdf'
DEFAULT = $(BASE).ps
#
#################################################################

.SUFFIXES: .tex .dvi .ps .pdf

.tex.dvi:
	latex $(BASE).tex

.dvi.ps: dvi
	dvips -o $(BASE).ps $(BASE).dvi

.ps.pdf: ps
	ps2pdf $(BASE).ps $(BASE).pdf

all: $(DEFAULT)

$(BASE).dvi: $(OBJECTS)

dvi: $(BASE).dvi

ps: $(BASE).ps

pdf: $(BASE).pdf

clean:
	rm -f *.aux \
	rm -f *~ \
	rm -r *.lot \
	rm -f *.log \
	rm -f *.toc \
	rm -f $(BASE).dvi \
	rm -f $(BASE).ps \
	rm -f $(BASE).pdf