@echo off

IF EXIST "Plugin/External/tbb44_20160526oss_win_0.zip" (
    echo "skipping setup"
) ELSE (
    echo "downloading TBB..."
    Powershell.exe -executionpolicy remotesigned -File  setup.ps1
    cd External
    7z\7za.exe x -aos *.7z
)
