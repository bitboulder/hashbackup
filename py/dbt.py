import os
import gzip
import struct
import importlib

import dirrec
importlib.reload(dirrec)

VERSION=1
MARKER=0x4381E32F

SHALEN=20
LNKLEN=1024

class DBT:
    def __init__(self,dbh):
        self.t={}
        self.dbh=dbh
        dirrec.run('.',None,'dbt',self.loadh,None)
    def loadh(self,fn,typ,arg):
        def readstr(f):
            s=b''
            while True:
                b=f.read(1)
                if b==b'\n': break
                s+=b
            return s.decode('utf-8')
        if typ!=dirrec.FILE: return
        if fn[-4:]!='.dbt': return
        with gzip.open(fn,'rb') as f:
            if struct.unpack('I',f.read(4))[0]!=MARKER: raise ValueError('MARKER wrong')
            if struct.unpack('I',f.read(4))[0]!=VERSION: raise ValueError('VERSION wrong')
            t=struct.unpack('Q',f.read(8))[0]
            self.t[t]=dt=DT(t)
            while True:
                ffn=readstr(f)
                if len(ffn)==0: break
                df=DF(dt,ffn,f)
                dt.f.append(df)
                if df.isfile():
                    dh=self.dbh.get(df.sha)
                    if dh is None: raise ValueError('file "%s" missing'%sha2fn(df.sha))
                    dh.add(df)

class DT:
    def __init__(self,t):
        self.t=t
        self.f=[]

class DF:
    def __init__(self,dt,fn,f=None):
        self.dt=dt
        self.fn=fn
        self.st={}
        self.sha=None
        self.lnk=None
        self.dh=None
        if f is None: return
        self.st=dict(zip(
            ('ftyp','uid','gid','mode','size','atime','mtime','ctime'),
            struct.unpack('IIIIQQQQ',f.read(48))
        ))
        typ=self.st['ftyp']
        if typ==dirrec.FILE:
            self.sha=f.read(SHALEN)
        elif typ==dirrec.LNK:
            self.lnk=f.read(LNKLEN)
        elif typ==dirrec.EXT2:
            raise ValueError('TODO')
    def isfile(self): return 'ftyp' in self.st and self.st['ftyp']==dirrec.FILE
    def setdh(self,dh): self.dh=dh
