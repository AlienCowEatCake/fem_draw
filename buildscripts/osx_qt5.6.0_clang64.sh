#!/bin/bash
V_PROJECT=fem_draw_qt
QTDIR=/Applications/Qt/5.6/clang_64
V_BUILDDIR=build_osx_qt5.6.0_clang64
V_APPNAME="FEM Draw"

cd "$(dirname $0)"/..
rm -rf "${V_BUILDDIR}"
mkdir -p "${V_BUILDDIR}"
cd "${V_BUILDDIR}"
"${QTDIR}"/bin/qmake CONFIG+="release" QMAKE_MACOSX_DEPLOYMENT_TARGET=10.7 "../${V_PROJECT}.pro"
make
"${QTDIR}"/bin/macdeployqt "${V_APPNAME}.app" -dmg -verbose=2

hdiutil convert -format UDRW -o "${V_APPNAME}_rw.dmg" "${V_APPNAME}.dmg"
mkdir "${V_APPNAME}_rw_mount"
hdiutil attach -mountpoint "${V_APPNAME}_rw_mount" -noautoopen "${V_APPNAME}_rw.dmg"
cd "${V_APPNAME}_rw_mount"
ln -s /Applications ./Applications
cd ..
hdiutil detach "${V_APPNAME}_rw_mount"
hdiutil convert -format UDRO -o "${V_APPNAME}_ro.dmg" "${V_APPNAME}_rw.dmg"
cp "${V_APPNAME}_ro.dmg" ../"${V_APPNAME}.dmg"

