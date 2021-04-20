set "QTDIR=%~dp0"

set "PREFIX=W:\Qt-5.11.3-blokas"

%QTDIR%\configure.bat -prefix %PREFIX% -platform win32-msvc2015 -mp -opensource -confirm-license -nomake tests -nomake examples -no-openssl -no-opengl -verbose ^
	-skip qt3d -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d -skip qtdeclarative -skip qtdoc ^
	-skip qtgamepad -skip qtgraphicaleffects -skip qtlocation -skip qtmultimedia -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 ^
	-skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qttranslations -skip qtvirtualkeyboard -skip qtwayland ^
	-skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview -skip qtxmlpatterns -skip qttools
