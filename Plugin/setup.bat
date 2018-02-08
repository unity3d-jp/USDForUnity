@echo off

echo "downloading external libararies..."
powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/unity3d-jp/USDForUnity/releases/download/20180208/External.7z', 'External/External.7z')"
cd External
7z\7za.exe x -aos *.7z
cd ..
xcopy /EYF External\libs\win64\*.dll ..\USDForUnity\Assets\StreamingAssets\USDForUnity\plugins_win64\lib\
