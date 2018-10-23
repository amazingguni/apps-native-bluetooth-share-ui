%define _usrdir	/usr
%define _appdir %{_usrdir}/apps

Name:       org.tizen.bluetooth-share-ui
Summary:    bluetooth share UI application
Version:    0.2.1
Release:    1
Group:      TO_BE_FILLED
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz

BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-bluetooth)
BuildRequires:  pkgconfig(bluetooth-share-api)
BuildRequires:  pkgconfig(notification)

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-devel
BuildRequires:  hash-signer

%define PKG_NAME %{name}

%description
bluetooth share UI application


%prep
%setup -q

%build
export CFLAGS+=" -fpie -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"

cmake . -DCMAKE_INSTALL_PREFIX=%{_appdir}/org.tizen.bluetooth-share-ui
make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install
PKG_ID=%{name}
%define tizen_sign 1
%define tizen_sign_base /usr/apps/${PKG_ID}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

%post

%postun


#%files
#%defattr(-,root,root,-)

%files
%manifest bluetooth-share-ui.manifest
%license LICENSE
%defattr(-,root,root,-)
%{_appdir}/%{name}/res/edje/bt-share-layout.edj
%{_appdir}/%{name}/res/edje/images.edj
%{_appdir}/%{name}/bin/bluetooth-share-ui
%{_appdir}/%{name}/author-signature.xml
%{_appdir}/%{name}/signature1.xml
%{_usrdir}/share/icons/default/small/%{name}.png
%{_usrdir}/share/packages/%{name}.xml

