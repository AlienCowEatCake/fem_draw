#!/bin/bash
V_PROJECT=fem_draw_qt
V_BUILDDIR=build_osx_qt4.8.7_clang64
V_APPNAME="FEM Draw"
V_DMGNAME="fem_draw_qt"
V_INFO_PLIST="resources/Info.plist"
V_ICON="resources/fem_draw.icns"

CMD_QMAKE="qmake"
CMD_DEPLOY="macdeployqt"

cd "$(dirname $0)"/..
rm -rf "${V_BUILDDIR}"
mkdir -p "${V_BUILDDIR}"
cd "${V_BUILDDIR}"
${CMD_QMAKE} CONFIG+="release" CONFIG+="x86_64" -r -spec unsupported/macx-clang QMAKE_MACOSX_DEPLOYMENT_TARGET=10.5 "../${V_PROJECT}.pro"
make
cp -a "../${V_INFO_PLIST}" "${V_APPNAME}.app/Contents/Info.plist"
sed -e 's/10.7.0/10.5.0/' -i "" "${V_APPNAME}.app/Contents/Info.plist"
cp -a "../${V_ICON}" "${V_APPNAME}.app/Contents/Resources/"
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

