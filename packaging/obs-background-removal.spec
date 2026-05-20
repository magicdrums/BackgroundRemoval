%global model_url https://raw.githubusercontent.com/obs-backgroundremoval/obs-backgroundremoval/main/data/models/mediapipe.onnx

Name:           obs-background-removal
Version:        2.0.0
Release:        1%{?dist}
Summary:        OBS Studio plugin for AI portrait background removal

License:        GPL-2.0-or-later
URL:            https://github.com/vipereir/BackgroundRemoval

Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.16
BuildRequires:  cmake-rpm-macros
BuildRequires:  ninja-build
BuildRequires:  gcc-c++
BuildRequires:  obs-studio-devel
BuildRequires:  opencv-devel
BuildRequires:  onnxruntime-devel
BuildRequires:  gtest-devel
BuildRequires:  curl

Requires:       obs-studio >= 28
Requires:       onnxruntime
Requires:       opencv

%description
Filtro de vídeo para OBS Studio que elimina el fondo mediante segmentación
MediaPipe (ONNX Runtime). Incluye soporte para imagen de fondo personalizada.

%prep
%autosetup -n BackgroundRemoval-%{version} -p1
mkdir -p data/models
curl -fsSL -o data/models/mediapipe.onnx %{model_url}

%build
%cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=%{_prefix} \
    -DLINUX_PORTABLE=OFF \
    -DBUILD_TESTING=ON
%cmake_build

%check
export OBS_BGREMOVAL_MODEL_PATH=%{builddir}/data/models/mediapipe.onnx
ctest --test-dir build --output-on-failure

%install
%cmake_install

%files
%license LICENSE
%{_libdir}/obs-plugins/%{name}.so
%{_datadir}/obs/obs-plugins/%{name}/

%changelog
* Sun May 17 2026 BackgroundRemoval <local@localhost> - 2.0.0-1
- v2: custom background image support
- Unit tests and Fedora RPM packaging
