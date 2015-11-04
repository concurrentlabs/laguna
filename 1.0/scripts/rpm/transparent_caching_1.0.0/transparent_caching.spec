Summary: Laguna Transparent Caching.
Name: transparent_caching
Version: %{_version}
Release: %{_release}
License: ASL 2.0
Group: Applications/Tools
# source: %{name}-%{version}.tar.gz
BuildRoot: %{_topdir}/%{name}-root

%description
%{summary}

%define __spec_install_post /usr/lib/rpm/brp-compress

%prep
#

%build
#

%install
rm -fr ${RPM_BUILD_ROOT}
install -d ${RPM_BUILD_ROOT}/etc/init.d
install -d ${RPM_BUILD_ROOT}/etc/cron.daily
install -d ${RPM_BUILD_ROOT}/etc/sysconfig/%{name}
install -d ${RPM_BUILD_ROOT}/usr/local/bin/%{name}
install -d ${RPM_BUILD_ROOT}/usr/local/bin/%{name}/logs
install -d ${RPM_BUILD_ROOT}/usr/lib/systemd
install -d ${RPM_BUILD_ROOT}/usr/lib/systemd/system
install -p -m 700 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/init.d/transc \
        ${RPM_BUILD_ROOT}/etc/init.d
install -p -m 700 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/systemd/transc.service \
        ${RPM_BUILD_ROOT}/usr/lib/systemd/system
install -p -m 700 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/init.d/gwdisc \
        ${RPM_BUILD_ROOT}/etc/init.d
install -p -m 700 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/systemd/gwdisc.service \
        ${RPM_BUILD_ROOT}/usr/lib/systemd/system
install -p -m 755 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/cron.daily/cleanupSimulationLogs.sh \
        ${RPM_BUILD_ROOT}/etc/cron.daily
install -p -m 755 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/cron.daily/cleanupYamls.sh \
        ${RPM_BUILD_ROOT}/etc/cron.daily
install -p -m 644 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/sysconfig/%{name}/config.yaml \
        ${RPM_BUILD_ROOT}/etc/sysconfig/%{name}
install -p -m 644 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/sysconfig/%{name}/dfltsys.yaml \
        ${RPM_BUILD_ROOT}/etc/sysconfig/%{name}
install -p -m 644 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/sysconfig/%{name}/trlog.conf \
        ${RPM_BUILD_ROOT}/etc/sysconfig/%{name}
install -p -m 644 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/sysconfig/%{name}/README.* \
        ${RPM_BUILD_ROOT}/etc/sysconfig/%{name}
install -p -m 644 \
    %__PKGS_DIR/%{name}_%{_scversion}/etc/sysconfig/%{name}/mhue.conf.tcs \
        ${RPM_BUILD_ROOT}/etc/sysconfig/%{name}
install -p -m 700 \
    %__PKGS_DIR/%{name}_%{_scversion}/usr/local/bin/%{name}/transc \
        ${RPM_BUILD_ROOT}/usr/local/bin/%{name}/transc
install -p -m 700 \
    %__PKGS_DIR/%{name}_%{_scversion}/usr/local/bin/%{name}/gwdisc.py \
        ${RPM_BUILD_ROOT}/usr/local/bin/%{name}/gwdisc.py

%preun
if [ $1 = 0 ]; then
   service transc stop >/dev/null 2>&1
   /sbin/chkconfig --del transc
   service gwdisc stop >/dev/null 2>&1
   /sbin/chkconfig --del gwdisc
fi

%postun
# post-uninstall
/sbin/ldconfig

%post
# chkconfig --add transc
# chkconfig --level 2345 transc on
# chkconfig --add gwdisc
# chkconfig --level 2345 gwdisc on
/sbin/ldconfig
systemctl enable transc.service
systemctl enable gwdisc.service

%clean
rm -fr ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
/etc/init.d/transc
/etc/init.d/gwdisc
/usr/lib/systemd/system/transc.service
/usr/lib/systemd/system/gwdisc.service
/etc/cron.daily/cleanupSimulationLogs.sh
/etc/cron.daily/cleanupYamls.sh
/usr/lib/systemd/system/transc.service
/usr/local/bin/%{name}/transc
/usr/local/bin/%{name}/gwdisc.py
/etc/sysconfig/%{name}/dfltsys.yaml
%config(noreplace, missingok) /etc/sysconfig/%{name}/config.yaml
%config(noreplace, missingok) /etc/sysconfig/%{name}/trlog.conf
%config(noreplace, missingok) /etc/sysconfig/%{name}/README.*
%config(noreplace, missingok) /etc/sysconfig/%{name}/mhue.conf.tcs

