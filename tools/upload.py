#!/usr/bin/env python

import sys
import requests
import glob
import os
from time import sleep

proxies = {
    "http": None
}

ip = sys.argv[1]

files = glob.glob(os.path.join(os.path.abspath(os.getcwd()), sys.argv[2], "*.*"))

for f in files:
    name = os.path.basename(f)
    data = file(f, 'r').read()
    print "send %s, %sB to %s" % (name, len(data), ip)
    resp = requests.post("http://%s/%s"%(ip, name), data=data)
    print resp.reason, resp.ok
   