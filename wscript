#!/usr/bin/env python

import Options

from os import unlink, symlink
from os.path import exists, lexists

srcdir = "."
blddir = "build"
VERSION = "0.1.3"

def set_options(opt):
    opt.tool_options("compiler_cxx")

def configure(conf):
    conf.check_tool("compiler_cxx")
    conf.check_tool("node_addon")
    conf.check_cfg(package='libsparsehash', mandatory=1, args='--cflags --libs')
    conf.env.append_value('CXXFLAGS', ['-O2'])

def build(bld):
    obj = bld.new_task_gen("cxx", "shlib", "node_addon")

    obj.target = "rawhash"

    obj.find_sources_in_dirs("src")#see http: //www.mail-archive.com/programming@jsoftware.com/msg05886.html
    
def shutdown(): #HACK to get binding.node out of build directory.#better way to do this ?
    if Options.commands['clean']:
        if lexists('rawhash.node'):
            unlink('rawhash.node')
    else:
        if not lexists('rawhash.node'):
            if lexists('./build/Release/rawhash.node'):
                symlink('./build/Release/rawhash.node', 'rawhash.node')
            elif lexists('./build/default/default/rawhash.node'):
                symlink('./build/default/rawhash.node', 'rawhash.node')
