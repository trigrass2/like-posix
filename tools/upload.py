#!/usr/bin/env python

import sys
import requests
import glob
import os
from time import sleep

proxies = {
    "http": None
}

ips = [
   # ip list here
] + sys.argv[1:]

# uncomment files we dont want to overwrite every time
filt = [
    #'title.html',
]


files = glob.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)),"*.*"))

for ip in ips:
    for f in files:
        name = os.path.basename(f)
        if name not in filt:
            data = file(f, 'r').read()
            print "send %s, %sB to %s" % (name, len(data), ip)
            resp = requests.post("http://%s/%s"%(ip, name), data=data)
            print resp.reason, resp.ok
        else:
            print "skip", name