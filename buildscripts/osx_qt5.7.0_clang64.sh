#!/bin/bash
V_PROJECT=fem_draw_qt
V_BUILDDIR=build_osx_qt5.7.0_clang64
V_APPNAME="FEM Draw"
V_DMGNAME="fem_draw_qt"

QTDIR="${HOME}/Qt/5.7/clang_64"
CMD_QMAKE="${QTDIR}/bin/qmake"
CMD_DEPLOY="${QTDIR}/bin/macdeployqt"

cd "$(dirname $0)"/..
rm -rf "${V_BUILDDIR}"
mkdir -p "${V_BUILDDIR}"
cd "${V_BUILDDIR}"
${CMD_QMAKE} CONFIG+="release" QMAKE_MACOSX_DEPLOYMENT_TARGET=10.8 "../${V_PROJECT}.pro"
make
sed -e 's/10.6.0/10.8.0/' -i "" "${V_APPNAME}.app/Contents/Info.plist"
RES_PATH="${V_APPNAME}.app/Contents/Resources"
rm -f "${RES_PATH}/empty.lproj"
mkdir -p "${RES_PATH}/en.lproj" "${RES_PATH}/ru.lproj"
${CMD_DEPLOY} "${V_APPNAME}.app" -dmg -verbose=2

hdiutil convert -format UDRW -o "${V_APPNAME}_rw.dmg" "${V_APPNAME}.dmg"
mkdir "${V_APPNAME}_rw_mount"
hdiutil attach -mountpoint "${V_APPNAME}_rw_mount" -noautoopen "${V_APPNAME}_rw.dmg"
cd "${V_APPNAME}_rw_mount"
ln -s /Applications ./Applications
cd ..
hdiutil detach "${V_APPNAME}_rw_mount"
hdiutil convert -format UDRO -o "${V_APPNAME}_ro.dmg" "${V_APPNAME}_rw.dmg"
cp "${V_APPNAME}_ro.dmg" ../"${V_DMGNAME}.dmg"

