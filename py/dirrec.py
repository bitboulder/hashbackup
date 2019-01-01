import os

UNK=0
FILE=1
DIR=2
LNK=3
EXT2=4

def run(bdir,ex,dn,fnc,arg):
    for fn in os.listdir(os.path.join(bdir,dn)):
        if fn=='.' or fn=='..': continue
        fn=os.path.join(dn,fn)
        if not ex is None and ex.chk(fn): continue
        typ=UNK
        bfn=os.path.join(bdir,fn)
        if os.path.isfile(bfn): typ=FILE
        if os.path.isdir(bfn): typ=DIR
        if os.path.islink(bfn): typ=LNK
        fnc(fn,typ,arg)
        if typ==DIR: run(bdir,ex,fn,fnc,arg)
