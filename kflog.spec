# This spec file was generated using Kpp
# If you find any problems with this spec file please report
# the error to ian geiser <geiseri@msoe.edu>
Summary:   Soaring-pilot-software for KDE
Name:      kflog
Version:   2.0.7
Release:   1
Copyright: GPL
Vendor:    Heiner Lamprecht <heiner@kflog.org>
Url:       http://www.kflog.org/

Packager:  Heiner Lamprecht <heiner@kflog.org>
Group:     X11/KDE/Applications
Source:    kflog-2.0.7.tar.gz
BuildRoot: /tmp/build/

%description


%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure \
                --prefix=/opt/kde2 --enable-debug \
                $LOCALFLAGS
%build
# Setup for parallel builds
numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "$numprocs" = "0" ]; then
  numprocs=1
fi

make -j$numprocs

%install
make install-strip DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.kflog
find . -type f | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.kflog
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.kflog

%clean
rm -rf $RPM_BUILD_ROOT/*
rm -rf $RPM_BUILD_DIR/kflog
rm -rf ../file.list.kflog


%files -f ../file.list.kflog
