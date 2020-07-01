
def repopath(arg0: str):
    import os

    relp = os.path.dirname(arg0)
    absp = os.path.abspath(relp)

    rp = absp.replace('scripts', '')

    return rp