####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kflog
kflog_SOURCES = kflogview.cpp kflogdoc.cpp kflog.cpp main.cpp 
kflog_LDADD   =  -lkfile -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = main.cpp kflog.cpp kflog.h kflogdoc.cpp kflogdoc.h kflogview.cpp kflogview.h kflogui.rc kflog.desktop lo32-app-kflog.png lo16-app-kflog.png 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kflog.desktop $(kde_appsdir)/Applications/kflog.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-kflog.png $(kde_icondir)/locolor/32x32/apps/kflog.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-kflog.png $(kde_icondir)/locolor/16x16/apps/kflog.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kflog.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/kflog.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/kflog.png

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kflog_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kflog
rc_DATA = kflogui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kflog.pot; \
	fi

