#!/bin/bash
V_PROJECT="fem_draw_qt"
V_BUILDDIR="build_linux_qt4.8.7_gcc"
V_SUFFIX="$(gcc -dumpmachine)"

QTDIR="/opt/qt-4.8.7-static"
CMD_QMAKE="${QTDIR}/bin/qmake"

cd "$(dirname $0)"/..
rm -rf "${V_BUILDDIR}"
mkdir -p "${V_BUILDDIR}"
cd "${V_BUILDDIR}"
${CMD_QMAKE} CONFIG+="release" CONFIG+="use_static_qico" "../${V_PROJECT}.pro"
make
strip --strip-all "${V_PROJECT}"
cp "${V_PROJECT}" ../"${V_PROJECT}_${V_SUFFIX}.elf"

