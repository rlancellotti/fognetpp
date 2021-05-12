#!/usr/bin/python3
import os
import re
import json
import argparse
from mako.template import Template
from mako.runtime import Context
from io import StringIO

parser = argparse.ArgumentParser()
parser.add_argument('-f', '--force', action='store_true', help='force update')
parser.add_argument('-r', '--recursive', action='store_true', help='recursive')
parser.add_argument('-d', '--dir', help='database file, Default ./')

args = parser.parse_args()

def get_filename(ftemplate):
    return ftemplate.replace('.mako', '')

def should_update(ftemplate):
    fout = get_filename(ftemplate)
    if os.path.isfile(fout):
        return os.path.getmtime(ftemplate) > os.path.getmtime(fout)
    else:  
        return True

def process_template(ftemplate, force_update=True):
    fout = get_filename(ftemplate)
    if force_update or should_update(ftemplate):
        mytemplate=Template(filename=ftemplate)
        print ('[updating] %s -> %s' % (ftemplate, fout))
        with open(fout, "w") as f:
            f.write(mytemplate.render())
    else:
        print ('[keeping]  %s' % fout)

def process_directory(dirname, force_update=True, recursive=False):
    #print("working into directory %s (force_update=%s, recursive=%s)" % (dirname, force_update, recursive))
    for f in os.listdir(dirname):
        f=dirname+"/"+f
        if f.endswith(".mako"):
            # print("found match: %s" % f) 
            process_template(f, force_update=force_update)
        if os.path.isdir(f):
            #print("recursing into directory %s" % f)
            process_directory(f, force_update=force_update, recursive=recursive)
    #print("done with directory %s" % dirname)


dirname = args.dir if args.dir else "."
process_directory(dirname, force_update=args.force, recursive=args.recursive)
