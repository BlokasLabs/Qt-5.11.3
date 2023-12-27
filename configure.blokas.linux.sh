#!/bin/sh
QTDIR="$(dirname "$(readlink -f "$0")")"

PREFIX=/opt/Qt-5.11.3-blokas

${QTDIR}/configure -prefix ${PREFIX} -opensource -confirm-license -nomake tests -nomake examples -no-openssl --verbose \
	-skip qt3d -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d -skip qtdeclarative -skip qtdoc \
	-skip qtgamepad -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 \
	-skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qttranslations -skip qtvirtualkeyboard \
	-skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -no-feature-textodfwriter \
	-no-feature-im -no-feature-dom -no-feature-printpreviewwidget -no-feature-printpreviewdialog \
	-no-feature-cups -no-feature-fontdialog -no-feature-datawidgetmapper -no-feature-imageformat_bmp -no-feature-imageformat_ppm \
	-no-feature-imageformat_jpeg -no-feature-image_text -no-feature-colornames -no-feature-cups -no-feature-paint_debug \
	-no-feature-big_codecs -no-feature-iconv -no-feature-ftp -no-feature-udpsocket -no-feature-networkproxy -no-feature-socks5 -no-feature-networkdiskcache \
	-no-feature-bearermanagement -skip qttools -no-eglfs $@
