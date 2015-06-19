#!/usr/bin/env python

import sys
import requests
import glob
import os
from time import sleep

proxies = {
    "http": None
}

# list ip addresses to uplload to here
# they may be appended on the command line as well
ips = sys.argv[1:]

path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "*.*")
print path
files = glob.glob(path)

print files

for ip in ips:
    print ip
    for f in files:
        name = os.path.basename(f)
        if name != "upload.py":
            data = file(f, 'r').read()
            print "send %s, %sB to %s" % (name, len(data), ip)
            resp = requests.post("http://%s/%s"%(ip, name), data=data)
            print resp.reason, resp.ok
            sleep(0.5)

