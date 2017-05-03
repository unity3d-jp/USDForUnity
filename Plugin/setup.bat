@echo off

IF NOT EXIST "External\ispc.exe" (
    IF NOT EXIST "External\External.7z" (
        echo "downloading external libararies..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/unity3d-jp/USDForUnity/releases/download/USD-0.7.5/External.7z', 'External/External.7z')"
    )
    cd External
    7z\7za.exe x -aos *.7z
    cd ..
    xcopy /EYF External\python27\lib\*.dll ..\USDForUnity\Assets\UTJ\USDForUnity\Plugins\x86_64
    xcopy /EYF External\tbb\lib\*.dll ..\USDForUnity\Assets\UTJ\USDForUnity\Plugins\x86_64
    xcopy /EYF External\USD\lib\*.dll ..\USDForUnity\Assets\StreamingAssets\USDForUnity\plugins_win64\lib
)
