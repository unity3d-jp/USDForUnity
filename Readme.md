# USD For Unity
<img align="right" src="Documents/CRS_usdview.png">
[USD (Universal Scene Description)](http://graphics.pixar.com/usd/) is a file format that can store time-sampled 3D scene. It is something like 'Better Alembic' developed by Disney / Pixar.  
This plugin allow you to import USD scene to Unity and export Unity scene to USD file. The above image is exported scene of [Candy Rock Star](https://github.com/unity3d-jp/unitychan-crs) in usdview.

USD is currently preview version and only Linux binary is available. For this reason, **this plugin works only on Linux for now. Windows version will be available soon after public release version of USD is available**.


### How to build
Assume your OS is CentOS 7. (USD binary is built for CentOS)

1. download USD binary packages (usd-*.tar.gz and deps-vfx2015.tar.gz) from http://graphics.pixar.com/usd/
- extract packages to /opt/pixar
- execute these commands
```
$ source /opt/pixar/usd/bin/activate.sh
$ git clone https://github.com/unity3d-jp/USDForUnity
$ cd USDForUnity/Plugin
$ cmake . && make && cp libusdi.so ../USDForUnity/Assets/UTJ/Plugins/x86_64/usdi.so
```

### How to use Unity Editor on Linux

On Ubuntu, all you need to do is just install unity-editor-*.deb package. But on CentOS, you need to do bit more.

1. get package (unity-editor-installer-*.sh) from http://forum.unity3d.com/threads/unity-on-linux-release-notes-and-known-issues.350256/
- move to the directory you want to install and execute unity-editor-installer-*.sh
- install dependencies
```
$ sudo yum install epel-release
$ sudo yum install nodejs npm postgresql ld-linux.so.2 libstdc++.so.6
```

### History
- 2016/05/18:  
  preview release for Linux

## License
[MIT](USDForUnity/Assets/StreamingAssets/UTJ/USDForUnity/License.txt)
