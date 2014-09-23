Summary: small init with build-in SVC and cron
Name: ninit
Version: 0.14
Release: 1
Group: System Environment/Daemons
Packager: Nikola Vladov <v20-ninit_rpm@riemann.fmi.uni-sofia.bg>
Source: http://riemann.fmi.uni-sofia.bg/ninit/ninit-%{version}.tar.bz2
URL: http://riemann.fmi.uni-sofia.bg/ninit/
License: GPL
Prefix: /
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description

Ninit is a small daemon which can be PID 1.  It has build-in SVC and cron.
Read more about it on http://riemann.fmi.uni-sofia.bg/ninit/

%prep

%setup -q

%build
MYARCH=`uname -m | sed -e's/i[4-9]86/i386/' -e's/armv[3-6]t\?e\?[lb]/arm/'`
test "i386" != ${MYARCH} -a "x86_64" != ${MYARCH} && MYARCH=withdiet
make ${MYARCH} prefix=${RPM_BUILD_ROOT}
test -d bin-${MYARCH} && cp bin-${MYARCH}/* .

%install
mkdir -p ${RPM_BUILD_ROOT}/usr/share/man
make install DESTDIR=${RPM_BUILD_ROOT}
rm -f ${RPM_BUILD_ROOT}/etc/ninit/in ${RPM_BUILD_ROOT}/etc/ninit/out

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
test -p /etc/ninit/in  || mkfifo -m 600 /etc/ninit/in
test -p /etc/ninit/out || mkfifo -m 600 /etc/ninit/out

aa=`readlink /proc/1/exe`
if test "$aa" = "/sbin/ninit" ; then
 /sbin/ninit-reload -v -u /sbin/ninit
else 
 echo See the home page of ninit how to prepare the host to boot
 echo with /sbin/ninit instead of default /sbin/init.
 echo http://riemann.fmi.uni-sofia.bg/ninit/
fi
%postun

%files
%defattr(-,root,root)
/sbin
/usr
/etc
/bin
%doc README CHANGES

%changelog
* Thu May 14 2008 Nikola Vladov <v20-ninit_rpm@riemann.fmi.uni-sofia.bg>
- Create rpm
