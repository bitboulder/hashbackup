import os

class EX:
    def __init__(self):
        fn='exclude'
        if os.path.exists(fn):
            with open(fn,'r') as f: 
                self.pat=[p.strip() for p in f.readlines()[0].split('|')]
        else: self.pat=[]
    def chk(self,fn):
        for pat in self.pat:
            i=fn.find(pat)
            if i<0: continue
            if i>0 and (pat[0]=='/' or fn[i-1]=='/'): continue
            if len(fn)==i+len(pat) or fn[i+len(pat)]=='/': return 1
        return 0


