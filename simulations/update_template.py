#!/usr/bin/python3
import os
import re
import json
from mako.template import Template
from mako.runtime import Context
from io import StringIO

def get_filename(ftemplate):
    return ftemplate.replace('_template', '')

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
        print ('[updating] %s' % ftemplate)
        with open(fout, "w") as f:
            f.write(mytemplate.render())
    else:
        print ('[keeping]  %s' % ftemplate)


for f in os.listdir("./"):
    if re.match("^[a-zA-Z0-9_\-\.]+?_template\.[a-zA-Z0-9\-]+?$", f) and not f.endswith(".py"):
        # print("found match: %s" % f) 
        process_template(f, force_update=False)

#process_template("configDanilo_template.json")

