cd ..\sdk
call C:\Qt\5.7\msvc2013\bin\qmake.exe sdk.pro -spec win32-msvc2013 "CONFIG+=debug" "CONFIG+=qml_debug"
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\nmake.exe"
cd ..\samples
call C:\Qt\5.7\msvc2013\bin\qmake.exe samples.pro -spec win32-msvc2013 "CONFIG+=debug" "CONFIG+=qml_debug"
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\nmake.exe"
cd ..\bin
copy tests\*UnitTest* .
rmscryptoUnitTestsd.exe -xunitxml > tests\rmsUnitTestResults.txt
rmsauthUnitTestsd.exe -xunitxml >> tests\rmsUnitTestResults.txt
rmsplatformUnitTestsd.exe -xunitxml >> tests\rmsUnitTestResults.txt