Name:           openrcc
Version:        0.1.0
Release:        1%{?dist}
Summary:        Educational reimplementation of distributed game server control architecture
License:        MIT
URL:            https://github.com/m4rcel-lol/openrcc
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  protobuf-compiler
BuildRequires:  grpc-devel
BuildRequires:  spdlog-devel
BuildRequires:  systemd-devel

Requires:       grpc
Requires:       protobuf
Requires:       spdlog
Requires:       systemd

%description
OpenRCC is a clean-room, educational C++20 and Luau-based architecture for distributed game server control.

%prep
%autosetup

%build
%cmake -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%check
ctest --output-on-failure || :

%files
%license LICENSE*
%doc README*
%{_bindir}/openrcc
%{_bindir}/openrcc-soap

%changelog
* Sat Apr 04 2026 OpenRCC Maintainers <maintainers@openrcc.local> - 0.1.0-1
- Initial package
