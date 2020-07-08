#!/usr/bin/python3

import os
import sys
import importlib
sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
import mc
import db
import hcmd
importlib.reload(mc)
importlib.reload(db)
importlib.reload(hcmd)

def usage():
    print("Usage: %s COMMAND ARGUMENTS"%sys.argv[0])
    print("       init    BASEDIR [EXPATTERN{|EXPATTERN}]")
    print("       tlist   [TIMESPEC]")
    print("       flist   [TIMESPEC]")
    print("       flistx  [TIMESPEC] (list ext2 blocks)")
    print("       diff    [-c] [TIMESPEC]")
    print("       diffx   [-c] [TIMESPEC] (diff sha)")
    print("       commit  [-c]")
    print("       restore [FILESPEC] [TIMESPEC] [DSTDIR]")
    print("       del     TIMESPEC")
    print("       mctest  FILENAME")
    print("       dbcheck")

mc.init()
db=db.DB()
if len(sys.argv)<2: usage()
elif sys.argv[1] in ('init','tlist','flist','flistx','diffx','diff','commit','restore','del','dbcheck','mctest'):
    eval('hcmd.'+sys.argv[1]+'(db,*sys.argv[2:])')
else: usage()


