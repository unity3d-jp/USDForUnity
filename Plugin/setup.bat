@echo off

IF EXIST "External/tbb44_20160526oss_win_0.zip" (
    echo "skipping setup"
) ELSE (
    echo "downloading TBB..."
    Powershell.exe -executionpolicy remotesigned -File setup.ps1
    cd External
    7z\7za.exe x -aos *.7z
    cd ..
    xcopy /EYF External\python27\lib\*.dll ..\USDForUnity\Assets\UTJ\Plugins\x86_64
    xcopy /EYF External\tbb44_20160526oss\bin\intel64\vc14\tbb.dll ..\USDForUnity\Assets\UTJ\Plugins\x86_64
    xcopy /EYF External\USD\lib\*.dll ..\USDForUnity\Assets\UTJ\Plugins\x86_64
    xcopy /EYF External\USD\plugin\*.dll ..\USDForUnity\Assets\UTJ\Plugins\x86_64
    xcopy /EYF External\USD\share\usd\plugins ..\USDForUnity\Assets\StreamingAssets\UTJ\USDForUnity\plugins
)
