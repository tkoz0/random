
if __name__ == '__main__': quit()

# PRP
# input n > 1
# choose a such that a != 0 (mod n), typically 1 < a < n-1
# compute r = a^(n-1) mod n
# r != 1 -> n is composite
# r == 1 -> n is probably prime
def prp(n,a=2):
    return pow(a,n-1,n) == 1

# SPRP
# input n > 2, odd
# choose a in the same way as for PRP
# let n-1 = d * 2^s (with d odd)
# n is SPRP to base a if 1 of the following
# 1. a^d == 1 (mod n)
# 2. a^(d * 2^r) == -1 (mod n) for some 0 <= r < s
def sprp(n,a=2):
    s = 0
    d = n-1
    while d % 2 == 0: # compute s and d, n-1 = d * 2^s
        s += 1
        d //= 2
    res = pow(a,d,n)
    if res == 1 or res == n-1: return True # n-1 case handles r == 0
    # handle 1 <= r <= s-1 (if s > 1) by modular squaring s-1 times
    return any((res := (res*res) % n) == n-1 for _ in range(s-1))

quit() # TODO remove old code below

# let n > 1 odd
# choose 1 < a < n-1
# prp if a^(n-1) = 1 (mod n)
def prp(n,a=2):
    if n < 2: return False
    if n == 2: return True
    if n % 2 == 0: return False
    return pow(a,n-1,n) == 1

# let n > 1 odd
# let n-1 = d * 2^s
# sprp if a^d = 1 (mod n) or a^(d * 2^r) = -1 (mod n) for some 0 <= r < s
def sprp(n,a=2):
    if n < 2: return False
    if n == 2: return True
    if n % 2 == 0: return False
    s = 0
    d = n-1
    while d % 2 == 0: # compute s and d
        s += 1
        d //= 2
    res = pow(a,d,n)
    if res == 1: return True
    if res == n-1: return True # r = 0
    # find some r from 1 to s-1
    for _ in range(s-1):
        res = (res*res) % n
        if res == n-1: return True
    return False
