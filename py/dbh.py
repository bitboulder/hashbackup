import os
import importlib

import dirrec
importlib.reload(dirrec)

def fn2sha(fn):
    return bytes.fromhex(fn.replace('dbh/','').replace('/',''))

def filesize(fn): return os.stat(fn).st_size

IN=0x1
EX=0x2
OLD=0x4

class DBH:
    def __init__(self):
        self.h={}
        dirrec.run('.',None,'dbh',self.loadh,None)
    def loadh(self,fn,typ,arg):
        if typ!=dirrec.FILE: return
        sha=fn2sha(fn)
        si=filesize(fn)
        self.h[sha]=DH(si)
    def get(self,sha): return self.h[sha] if sha in self.h else None

class DH:
    def __init__(self,si):
        self.si=si
        self.f=set()
    def add(self,df):
        df.setdh(self)
        self.f.add(df)
    def getsi(self): return self.si
    def exdt(self,dt):
        r=0
        for df in self.f:
            if df.dt==dt: r|=IN
            else:
                r|=EX
                if df.dt.t>dt.t: r|=OLD
        return r
