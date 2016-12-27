@echo off

IF NOT EXIST "External/USD" (
    cd External
    7z\7za.exe x -aos *.7z
    cd ..
    xcopy /EYF External\python27\lib\*.dll ..\USDForUnity\Assets\UTJ\Plugins\x86_64
    xcopy /EYF External\tbb\lib\*.dll ..\USDForUnity\Assets\UTJ\Plugins\x86_64
    xcopy /EYF External\USD\lib\*.dll ..\USDForUnity\Assets\StreamingAssets\USDForUnity\plugins_win64\lib
)
