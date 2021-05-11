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


dirname = args.dir if args.dir else "./"

for f in os.listdir(dirname):
    if f.endswith(".mako"):
        # print("found match: %s" % f) 
        process_template(f, force_update=args.force)

