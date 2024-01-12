#!/bin/sh
QTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

PREFIX=/opt/Qt-5.11.3-x86_64-blokas

${QTDIR}/configure -prefix ${PREFIX} -debug-and-release -platform macx-clang -opensource -confirm-license -nomake tests -nomake examples -no-openssl --verbose \
	-skip qt3d -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d -skip qtdeclarative -skip qtdoc \
	-skip qtgamepad -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 \
	-skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qttranslations -skip qtvirtualkeyboard -skip qtwayland \
	-skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -no-feature-textodfwriter -no-feature-sharedmemory \
	-no-feature-systemsemaphore -no-feature-im -no-feature-process -no-feature-dom -no-feature-printpreviewwidget -no-feature-printpreviewdialog \
	-no-feature-cups -no-feature-fontdialog -no-feature-datawidgetmapper \
	-no-feature-colornames -no-feature-cups -no-feature-paint_debug \
	-no-feature-codecs -no-feature-big_codecs -no-feature-iconv -no-feature-ftp -no-feature-udpsocket -no-feature-networkproxy -no-feature-socks5 -no-feature-networkdiskcache \
	-no-feature-bearermanagement -skip qttools -no-feature-futimens
