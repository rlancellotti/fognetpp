#!/usr/bin/python3
# parse_data V4.0
import sys
import argparse
import numpy as np
import os
import json
import sqlite3
import shlex
import re
from multiprocessing import Pool



parser = argparse.ArgumentParser()
parser.add_argument('-r', '--reset', action='store_true', help='re-create database')
parser.add_argument('-c', '--config', help='.json file, Default config.json')
parser.add_argument('-d', '--db', help='database file, Default test.db')
parser.add_argument('-j', '--jobs', help='number of parallel tasks, Default 1')
parser.add_argument('scanames', metavar='F', nargs='+', help='list of .sca files')
#parser.add_argument('-n', '--no-header', dest='noheader', const=True, default=False, action='store_const', help='do not print header')

args = parser.parse_args()

#### Code do load the SCA file in a data structure ####

def is_top_level(l):
    return l[0] in ["run", "param", "scalar", "statistic"]


def parse_top_level(l):
    if l[0] == "run":
        return parse_run(l)
    if l[0] == "param":
        return parse_param(l)
    if l[0] == "scalar":
        return parse_scalar(l)
    if l[0] == "statistic":
        return parse_statistic(l)


def parse_run(l):
    return {"type": l[0], "name": l[1]}


def parse_param(l):
    return {"type": l[0], "object": l[1], "value": l[2].replace('"', '')}


def parse_scalar(l):
    return {"type": l[0], "object": l[1], "name": l[2], "value": l[3].replace('"', '')}


def parse_statistic(l):
    return {"type": l[0], "object": l[1], "name": l[2]}


def parse_second_level(l):
    if l[0] == "attr" or l[0] == "field":
        return parse_attr_field(l)
    if l[0] == "bin":
        return parse_bin(l)


def parse_attr_field(l):
    return {"type": l[0], "name": l[1], "value": l[2]}


def parse_bin(l):
    return {"type": l[0], "value": l[1], "samples": l[2]}


def add_top_level(data, rec):
    if rec is not None:
        if not rec["type"] in data.keys():
            data[rec["type"]]=[]
        data[rec["type"]].append(rec)
    return rec


def add_second_level(top_rec, second_rec):
    if top_rec is not None and second_rec is not None:
        if not second_rec["type"] in top_rec.keys():
            top_rec[second_rec["type"]]=[]
        top_rec[second_rec["type"]].append(second_rec)


def parse_sca(scafile):
    data={}
    with open(scafile) as f:
        lastrec = None
        for line in f:
            l = shlex.split(line)
            if len(l) > 1:
                if is_top_level(l):
                    lastrec = add_top_level(data, parse_top_level(l))
                else:
                    r = parse_second_level(l)
                    #print(lastrec, r)
                    add_second_level(lastrec, r)
    return data


#### End: Code to load the SCA file in a data structure ####


#### Code to search records in the SCA file data ####

def to_regexp(pattern):
    rv =[]
    for p in pattern.split('.'):
        # check if we have wildcards
        if p == '**':
            # use non-greedy version '*?' instead of '*'
            rv.append('.*?')
        else:
            p = p.replace('[', '\[')
            p = p.replace(']', '\]')
            p = p.replace('*', '[0-9]+?')
            rv.append(p)
    return ('^%s$' % '\.'.join(rv))


def match_field(field, pattern):
    if field is None:
        return False
    re_pattern = to_regexp(pattern)
    return re.match(re_pattern, field)


def match_record(record, pattern):
    for field in pattern:
        if not match_field(record[field], pattern[field]):
            #print ("!! failed match", record[field], pattern[field])
            return False
    return True


def search_in_collection(collection, pattern):
    rv = []
    for rec in collection:
        #print("matching: ", rec)
        if match_record(rec, pattern):
            #print("!! found: ", rec)
            rv.append(rec)
    return rv


def search(data, toplevel_pattern, secondlevel_pattern=None):
    if toplevel_pattern['type'] in data.keys():
        collection = data[toplevel_pattern['type']]
        toplevel_records = search_in_collection(collection, toplevel_pattern)
        #print(toplevel_records)
        if secondlevel_pattern is None:
            return toplevel_records
        else:
            rv = []
            for rec in toplevel_records:
                if secondlevel_pattern['type'] in rec.keys():
                    collection = rec[secondlevel_pattern['type']]
                    secondlevel_records = search_in_collection(collection, secondlevel_pattern)
                    rv.extend(secondlevel_records)
            return rv
    else:
        return []


def extract_field(collection, fieldname):
    rv = []
    for rec in collection:
        if fieldname in rec.keys():
            rv.append(rec[fieldname])
    return rv


#### End: Code do search records in the SCA file data ####


def load_config(configname):
    with open(configname) as f:
        return json.load(f)


def get_element(scaname, record_type, match):
    with open(scaname, 'r') as f:
            for line in f:
                l=line.split()
                if len(l)>1:
                    if l[0] == record_type and l[1] == match:
                        return l[2]

def get_scenario(scadata, schema):
    s={}
    s['name'] = extract_field(search(scadata, {'type': 'run'}, {'type': 'attr', 'name': 'configname'}), 'value')[0]
    s['run'] = extract_field(search(scadata, {'type': 'run'}, {'type': 'attr', 'name': 'repetition'}), 'value')[0]
    #print(s['name'], s['run'])
    for p in schema:
        #print('"%s"' % schema[p]["pattern"])
        records = search(scadata, {'type': 'param', 'object': schema[p]["pattern"]})
        #print(records)
        s[p] = extract_field(records, 'value')[0]
    return s


def get_scalar(scadata, scalarname, objname):
    if '/' in scalarname:
        [statname, fieldname] = scalarname.split('/')
        rv = extract_field(search(scadata, {'type': 'statistic', 'object': objname, 'name': statname}, {'type': 'field', 'name': fieldname}), 'value')
    else:
        rv = extract_field(search(scadata, {'type': 'scalar', 'object': objname, 'name': scalarname}), 'value')
        #print(objname, scalarname)
        #print(rv)
    rv = [float(i) for i in rv]
    return rv


def aggregate(data, aggr):
    arr=np.array(data)
    if aggr == "sum":
        return arr.sum()
    if aggr == "avg":
        return arr.mean()
    if aggr == "std":
        return arr.std()
    if aggr == "none":
        #print(arr)
        return arr[0]

def get_metric_value(scadata, metric):
    data = get_scalar(scadata, metric["scalar_name"], metric["module"])
    #print(metric["scalar_name"], metric["module"], data) 
    v = {}
    for aggr in metric["aggr"]:
        v[aggr] = aggregate(data, aggr)
    return v


def init_db(conn, schema):
    c = conn.cursor()
    #get the count of tables with the name
    c.execute(''' SELECT count(name) FROM sqlite_master WHERE type='table' AND name='scenario' ''')
    if c.fetchone()[0]!=1:
        descr = None
        for p in schema:
            descr = "%s, %s %s" % (descr, p, schema[p]["type"]) if descr is not None else "%s %s" % (p, schema[p]["type"])
        c.execute('''CREATE TABLE 'scenario' (name text, run int, %s) ''' % descr)
    c.execute(''' SELECT count(name) FROM sqlite_master WHERE type='table' AND name='value' ''')
    if c.fetchone()[0]!=1:
        c.execute('''CREATE TABLE 'value' (scenarioid int, metric text, aggregation text, value real, FOREIGN KEY(scenarioid) REFERENCES scenario(rowid) )''')
    conn.commit()

def save_scenario(conn, scenario):
    c = conn.cursor()
    rv = None
    descr = None
    val = None
    c.execute(''' SELECT count(*) FROM 'scenario' WHERE name='%s' AND run='%s' ''' % (scenario['name'], scenario['run']))
    if c.fetchone()[0]!=1:
        # insert and return id
        for p in scenario:
            v = "'%s'" % scenario[p]
            descr = "%s, %s" % (descr, p) if descr is not None else "%s" % (p)
            val = "%s, %s" % (val, v) if val is not None else "%s" % (v)
        #print(descr, val)
        c.execute('''INSERT INTO 'scenario' (%s) VALUES (%s)''' % (descr, val))
        conn.commit()
        return c.lastrowid
    else:
        c.execute(''' SELECT rowid FROM 'scenario' WHERE name='%s' ''' % scenario['name'])
        return c.fetchone()[0]

def save_metric(conn, values, metric, scenario_id):
    c = conn.cursor()
    for v in values:
        c.execute('''SELECT count(*) FROM 'value' WHERE scenarioid = '%s' AND metric = '%s' AND aggregation = '%s' ''' % (scenario_id, metric, v))
        if c.fetchone()[0]!=1:
            c.execute('''INSERT INTO 'value' (scenarioid, metric, aggregation, value) VALUES ('%s', '%s', '%s', '%s')''' % (scenario_id, metric, v, values[v]))
            conn.commit()

def process_sca(scaname, config):
    # parse sca file and return relevant info
    print(scaname)
    schema = config["scenario_schema"]
    metrics = config["metrics"]
    scadata = parse_sca(scaname)
    #with open(scaname.replace('.sca','.json'), 'w') as f:
    #    json.dump(scadata, f, indent=4)
    scenario = get_scenario(scadata, schema)
    val = []
    for m in metrics:
        val.append({"metric": m, "values": get_metric_value(scadata, metrics[m])})
    return {"scenario": scenario, "metrics":val}


def list_sca(scaname, config):
    return (scaname, len(config))

#FIXME: we should support a wide range of scanames without any switch!
#scaname = args.input if args.input else "test.sca"
configname = args.config if args.config else "config.json"
dbname = args.db if args.db else "test.db"
config = load_config(configname)
jobs = int(args.jobs) if args.jobs else 1

scanames = args.scanames

call_params = []
print("# processing results")
for f in scanames:
    call_params.append((f, config))
with Pool(jobs) as p:
    #results = p.starmap(list_sca, call_params)
    results = p.starmap(process_sca, call_params)
#print(results)
#exit()
schema = config["scenario_schema"]
if args.reset:
    if os.path.exists(dbname):
        os.remove(dbname)
    print("# saving results on %s (resetting db)" % dbname)
else:
    print("# saving results on %s" % dbname)
conn = sqlite3.connect(dbname)
init_db(conn, schema)
for res in results:
    scenario_id = save_scenario(conn, res["scenario"])
    metrics = res["metrics"]
    for m in metrics:
        #print(m["metric"])
        #print(m["values"])
        #print(scenario_id)
        save_metric(conn, m["values"], m["metric"], scenario_id)

