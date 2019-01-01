import importlib
import ex
import dbh
import dbt
importlib.reload(ex)
importlib.reload(dbh)
importlib.reload(dbt)

class DB:
    def __init__(self):
        with open('basedir','r') as f: 
            self.bdir=f.readlines()[0].strip()
        self.ex=ex.EX()
        self.dbh=dbh.DBH()
        self.dbt=dbt.DBT(self.dbh)

