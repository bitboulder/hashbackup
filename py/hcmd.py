import time
import importlib
import dbh
importlib.reload(dbh)

def timefmt(t):
    return time.strftime('%y-%m-%d %H:%M',time.localtime(t))

def sizefmt(si):
    e=' kMGT'
    while si>1000 and len(e)>1:
        si/=1024
        e=e[1:]
    k=0
    if si<100: k+=1
    if si<10: k+=1
    return ('%.'+str(k)+'f'+e[:1])%si

def tlistt(db,dt):
    nf=len(dt.f)
    si=sum(df.st['size'] for df in dt.f)
    gz=sum(dh.getsi() for dh in db.dbh.h.values() if 0!=dh.exdt(dt)&dbh.IN)
    ex=sum(dh.getsi() for dh in db.dbh.h.values() if 0==dh.exdt(dt)&dbh.EX)
    su=sum(dh.getsi() for dh in db.dbh.h.values() if 0==dh.exdt(dt)&dbh.OLD)
    print('%s nf %4i si %5s gz %5s ex %5s sum %5s'%
        (timefmt(dt.t),nf,sizefmt(si),
            sizefmt(gz),sizefmt(ex),sizefmt(su)))

def tlist(db,tspec=None):
    for t,dt in sorted(db.dbt.t.items()): tlistt(db,dt)
