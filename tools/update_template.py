#!/usr/bin/python3
import os
import re
import json
from mako.template import Template
from mako.runtime import Context
from io import StringIO

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


for f in os.listdir("./"):
    if f.endswith(".mako"):
        # print("found match: %s" % f) 
        process_template(f, force_update=False)

