#!/usr/bin/ruby

require 'fileutils'
include FileUtils

$search_paths = [
    "/usr/local/lib",
    "/opt/intel/tbb/lib",
    "/opt/pixar/lib",
]
$dst_dir = "dst"
$copy_dependents = true

def find_library(lib)
    $search_paths.each do |dir|
        return dir if File.exists?(dir + "/" + lib)
    end
    nil
end

def do_copy_dependents(dir, name)
    if File.exists? "#{$dst_dir}/#{name}"
        #puts "skipped copy #{name}"
        return nil
    end
    dst = "#{$dst_dir}/#{name}"
    #xattr -d com.apple.quarantine #{dst}
    cmd = "cp #{dir}/#{name} #{dst} && chmod 777 #{dst}"
    puts(cmd)
    `#{cmd}`
    return "#{dst}"
end

def process(target)
    next_targets = []

    id = `otool -D #{target}`.split("\n")[1]

    list = `otool -L #{target}`
    list.each_line do |l|
        next if !l.match(/\t([^ ]+)/)
        l = $1
        next if l.match(/^\/usr\/lib/) || l.match(/^\/System\//)

        path = l
        dir = nil
        name = nil
        if l.match(/(.*)\/(.+)/)
            dir = $1
            name = $2
        else
            name = l
        end

        if $copy_dependents
            nt = nil
            if dir && dir[0] != '@'
                nt = do_copy_dependents(dir, name)
            else
                found = find_library(name)
                if !found
                    puts("couldn't find #{name}. exit.")
                    exit
                else
                    nt = do_copy_dependents(found, name)
                end
            end
            next_targets << nt if nt
        end

        if dir != "@loader_path"
            cmd = ""
            if path == id
                cmd = "install_name_tool -id @loader_path/#{name} #{target}"
            else
                cmd = "install_name_tool -change #{path} @loader_path/#{name} #{target}"
            end
            puts cmd
            `#{cmd}`
        end
    end

    next_targets.each do |t|
        next if !t
        process(t)
    end
end

loop do
    a = ARGV.shift
    break if !a
    if a == "-L"
        $search_paths << ARGV.shift
    elsif a == "-d"
        $dst_dir = ARGV.shift
        mkpath($dst_dir)
    end
    process(a)
end
