@echo off

echo "downloading external libararies..."
powershell.exe -Command "[System.Net.ServicePointManager]::SecurityProtocol=[System.Net.SecurityProtocolType]::Tls12; wget https://github.com/unity3d-jp/USDForUnity/releases/download/20180208/External.7z -OutFile External/External.7z"
cd External
7z\7za.exe x -aos *.7z
cd ..
xcopy /EYF External\libs\win64\*.dll ..\USDForUnity\Assets\StreamingAssets\USDForUnity\plugins_win64\lib\
