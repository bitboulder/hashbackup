#!/usr/bin/python3

import os
import stat
import subprocess
import hashlib
import datetime
import numpy as np
import sys
import joblib
import struct
import magic
import gzip
import lzma
import pickle

ma1=magic.Magic(mime=False,uncompress=True)
ma2=magic.Magic(mime=True)

def blkfmt(s): return sizefmt(s*4096)
def sizefmt(s):
#    return f'{s}'
    e=['B','kB','MB','GB']
    while len(e)>1 and s>1024: e=e[1:]; s/=1024
    if s<10: return f'{s:3.1f}{e[0]}'
    else: return f'{s:3.0f}{e[0]}'

#man inode(7)
def sta_isreg(sta): return (sta[1]*256)&0o170000==0o100000
def sta_islnk(sta): return (sta[1]*256)&0o170000==0o120000
def sta_size(sta): return struct.unpack('Q',sta[-8:])[0]

class Bckp:
    def __init__(self,db,dn=None,date=None,fn=None):
        self.db=db
        if fn is None:
            if ':' in dn: dn,date=dn.split(':',1)
            if date is None: date=f'{datetime.datetime.now():%y%m%d}'
            self.fn=os.path.join(self.db.dn,date+'_'+os.path.basename(os.path.realpath(dn)))
            if os.path.exists(self.fn): raise ValueError(f'Bckp exists: {self.fn}')
            self.do(dn)
        else:
            print(f'bckp load {fn}')
            self.fn=os.path.join(self.db.dn,os.path.basename(fn))
            with gzip.open(self.fn,'rb') as f:
                self.fns=pickle.load(f)
                self.sta=pickle.load(f)
                self.hsh=pickle.load(f)
    @classmethod
    def load(cls,db,fn): return cls(db,fn=fn)
    def do(self,dn):
        self._find(dn)
        self.sta=[]
        self.hsh=[]
        for fn in self.fns:
            sta=self._addsta(os.path.join(dn,fn))
            if  sta_isreg(sta): h=self._addfile(os.path.join(dn,fn),sta_size(sta))
            elif sta_islnk(sta): h=self._addlnk(os.path.join(dn,fn))
            else: h=bytes(32)
            self.sta.append(sta)
            self.hsh.append(h)
        self.save()
        self.db.storesave()
    def _find(self,dn):
        if dn[-1]==os.path.sep: dn=dn[:-1]
        getdir=lambda dn: sum((getdir(fn) if not os.path.islink(fn) and os.path.isdir(fn) else [fn] for fn in map(lambda fn:os.path.join(dn,fn),os.listdir(dn))),[])
        self.fns=[fn[len(dn)+len(os.path.sep):] for fn in getdir(dn)]
    def _addsta(self,fn):
        sta=os.stat(fn,follow_symlinks=False)
        sta=struct.pack(
            'IIIIIIQ',
            sta.st_mode,
            sta.st_uid,
            sta.st_gid,
            int(sta.st_atime),
            int(sta.st_ctime),
            int(sta.st_mtime),
            sta.st_size,
        )
        return sta
    def _addfile(self,fn,size):
        with open(fn,'rb') as f: buf=f.read(2048)
        if 'ext4' in ma1.from_buffer(buf):
            print(fn)
            t=ma2.from_buffer(buf)
            with open(fn,'rb') as f: return self._addblk(f,t)
        else:
            with open(fn,'rb') as f: return self._addbuf(f.read())
    def _addlnk(self,fn):
        return self._addbuf(os.readlink(fn).encode())
    def _addblk(self,f,t):
        prg=None
        if t=='application/x-xz': prg=['pixz','-d']
        if t=='application/gzip': prg=['pigz','-cd']
        if t=='application/x-bzip2': prg=['bzip2','-cd']
        if not prg is None:
            p=subprocess.Popen(prg,stdin=f,stdout=subprocess.PIPE)
            rd=p.stdout
        else: rd=f
        
        hsh=[]
        while True:
            buf=rd.read(4096)
            if len(buf)==0: break
            hsh.append(self._addbuf(buf))

        if not prg is None: p.wait()
        return hsh
    def _addbuf(self,buf):
        h=hashlib.sha256(buf).digest()
        self.db.storeput(h,buf)
        return h
    def save(self):
        with gzip.open(self.fn,'wb') as f:
            pickle.dump(self.fns,f,pickle.HIGHEST_PROTOCOL)
            pickle.dump(self.sta,f,pickle.HIGHEST_PROTOCOL)
            pickle.dump(self.hsh,f,pickle.HIGHEST_PROTOCOL)
    def ana(self):
        hsh=[
            (h,(si,db.storegetzsize(h)))
            for h,sta in zip(self.hsh,self.sta)
            for h,si in ( zip(h,[4096]*len(h)) if isinstance(h,list) else [(h,sta_size(sta))] )
        ]
        return {
            's':dict(hsh),
            'sizeori':sum(sta_size(sta) for sta in self.sta),
            'sizebuf':sum(h[1][0] for h in hsh),
            'sizezip':sum(h[1][1] for h in hsh),
        }

class Store:
    def __init__(self,fn):
        self.version=1
        self.fn=fn
        self.use=0
        self.idx={}
        self.load()
    @classmethod
    def from_old(cls,dn):
        self=cls(dn+'.store')
        getdir=lambda dn: sum((getdir(fn) if os.path.isdir(fn) else [fn] for fn in map(lambda fn:os.path.join(dn,fn),os.listdir(dn))),[])
        fns=[fn[len(dn)+len(os.path.sep):] for fn in getdir(dn)]
        for fn in fns:
            with open(os.path.join(dn,fn),'rb') as f:
                self.add(fn.replace(os.path.sep,''),f.read())
        self.save()
        return self
    def load(self):
        if not os.path.exists(self.fn): self.save()
        with open(self.fn,'rb') as f:
            f.seek(-9,2)
            l=f.tell()
            self.use,v=struct.unpack('Qb',f.read(9))
            if v!=self.version: raise ValueError(f'Version missmatch in store: {v}!={self.version}')
            f.seek(self.use)
            self.idx=pickle.loads(gzip.decompress(f.read(l-self.use)))
    def save(self):
        with open(self.fn,'rb+' if os.path.exists(self.fn) else 'wb') as f:
            f.seek(self.use)
            f.write(gzip.compress(pickle.dumps(self.idx,pickle.HIGHEST_PROTOCOL)))
            f.write(struct.pack('Qb',self.use,self.version))
    def add(self,key,buf):
        if key in self.idx:
            if self.get(key)==buf: return
            raise ValueError(f'key colission in store {self.fn}: {key}')
        with open(self.fn,'rb+') as f:
            fmt,buf=self._packbuf(buf)
            self.idx[key]=(self.use,len(buf),fmt)
            f.seek(self.use)
            f.write(buf)
            self.use+=len(buf)
    def get(self,key):
        if not key in self.idx: raise ValueError(f'key not found in store {self.fn}: {key}')
        s,l,fmt=self.idx[key]
        with open(self.fn,'rb') as f:
            f.seek(s)
            return self._unpackbuf(fmt,f.read(l))
    def getzsize(self,key):
        if not key in self.idx: raise ValueError(f'key not found in store {self.fn}: {key}')
        return self.idx[key][1]
    def _packbuf(self,buf):
        return min([
            (' ',buf),
            ('x',lzma.compress(buf)),
            ('g',gzip.compress(buf))
        ],key=lambda v:len(v[1]))
    def _unpackbuf(self,fmt,buf):
        if fmt==' ': return buf
        if fmt=='x': return lzma.decompress(buf)
        if fmt=='g': return gzip.decompress(buf)
        raise ValueError(f'unknown format {fmt}')

class Db:
    def __init__(self,dn):
        self.dn=dn
        self.fns=[fn for fn in map(lambda fn:os.path.join(dn,fn),os.listdir(dn)) if not os.path.isdir(fn)] if os.path.exists(dn) else []
        #self.stores={i:Store(os.path.join(dn,'db',f'{i}.store')) for i in map(lambda i:f'{i:02x}',range(256))}
        self.stores=dict(joblib.Parallel(n_jobs=-1)(
            joblib.delayed(lambda i:(i,Store(os.path.join(dn,'db',f'{i}.store'))))(i)
            for i in map(lambda i:f'{i:02x}',range(256))
        ))
    def storeput(self,key,buf):
        key=key.hex()
        self.stores[key[:2]].add(key[2:],buf)
    def storesave(self):
        for s in self.stores.values(): s.save()
    def storeget(self,key):
        key=key.hex()
        return self.stores[key[:2]].get(key[2:])
    def storegetzsize(self,key):
        key=key.hex()
        return self.stores[key[:2]].getzsize(key[2:])
    def ana(self):
        bckps={fn:Bckp.load(self,fn).ana() for fn in self.fns
#        bckps=dict(joblib.Parallel(n_jobs=-1,prefer='threads')(
#            joblib.delayed(lambda fn:(fn,Bckp.load(self,fn).ana()))(fn)
#            for fn in self.fns
#        ))
        al=np.array([0,0,0])
        hsh={}
        for fn,a in sorted(bckps.items()):
            e=set(a['s'])
            for fn1,a1 in bckps.items():
                if fn!=fn1: e=e.difference(a1['s'])
            a['hshbuf']=sum(sb for sb,sz in a['s'].values())
            a['hshzip']=sum(sz for sb,sz in a['s'].values())
            a['excbuf']=sum(a['s'][h][0] for h in e)
            a['exczip']=sum(a['s'][h][1] for h in e)
            hsh.update(a['s'])
            al+=[a['sizeori'],a['sizebuf'],a['hshbuf']]
            print(f'{os.path.basename(fn):28s} {sizefmt(a["sizeori"])} {sizefmt(a["sizebuf"])} {sizefmt(a["hshbuf"])} {sizefmt(a["hshzip"])} {sizefmt(a["excbuf"])} {sizefmt(a["exczip"])}')
        print(f'{"SUM":28s} {sizefmt(al[0])} {sizefmt(al[1])} {sizefmt(al[2])}       {sizefmt(sum(sb for sb,sz in hsh.values()))} {sizefmt(sum(sz for sb,sz in hsh.values()))}')

if '-n' in sys.argv: raise SystemExit()
db=Db('/mnt/auto/hugo_xxx/hashbackup')
if len(sys.argv)>1:
    if sys.argv[1]=='ana': db.ana()
    else: bckp=Bckp(db,sys.argv[1])

#s=Store.from_old(os.path.join(db.dn,'db','00'))
