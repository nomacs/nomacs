import os

def repopath(arg0 = ""):
    
    relp = os.path.dirname(arg0)
    rp = os.path.abspath(relp)

    rp = os.path.join(rp, os.pardir) 

    return os.path.abspath(rp)

def mypath(file = ""):

    if not file:
        file = __file__

    fp = os.path.dirname(file)
    rp = os.path.abspath(fp)

    return rp

def version_cache():
    return os.path.join(mypath(), "version.cache")

def version():

    vp = version_cache()

    with open(vp, "r") as src:
        for l in src.read().splitlines():

            # assuming: version: 3.15.0.1444
            pl = l.split(":")
            if len(pl) == 2:
                return pl[1]

    print("WARNING: could not parse version from " + vp)

    return ""
