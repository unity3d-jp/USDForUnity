#!/usr/bin/ruby

def fix(f)
    list = `otool -L #{f}`
    list.each_line do |l|
        if(!l.match(/\t([^ ]+)/))
            next
        end
        l = $1

        [
            "libar.dylib",
            "libarch.dylib",
            "libtf.dylib",
            "libgf.dylib",
            "libpcp.dylib",
            "libvt.dylib",
            "libsdf.dylib",
            "libusd.dylib",
            "libusdGeom.dylib",
            "libboost_python-mt.dylib",
            "libboost_system-mt.dylib",
            "libboost_filesystem-mt.dylib",
            "libtbb.dylib",
            "libtbbmalloc.dylib",
            "libHalf.12.dylib",
            "libGLEW.2.0.0.dylib",
        ].each do |t|
            if(l.match(/#{t}/))
                cmd = "install_name_tool -change #{l} @loader_path/#{t} #{f}"
                puts cmd
                `#{cmd}`
                break
            end
        end
    end
end

ARGV.each do |f|
    fix(f)
end
