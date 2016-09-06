Add-Type -AssemblyName System.IO.Compression.FileSystem
function Unzip
{
    param([string]$zipfile, [string]$outpath)
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

wget "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/windows/tbb44_20160526oss_win_0.zip" -OutFile External/tbb44_20160526oss_win_0.zip
Unzip "External/tbb44_20160526oss_win_0.zip" "External"
