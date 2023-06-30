from fractions import Fraction as Frac
import math
import time

terms = 5000

esum1 = Frac(0)

t1 = time.perf_counter()
for k in range(terms+1):
    esum1 += Frac(1,math.factorial(k))
t2 = time.perf_counter()
print('direct sum',t2-t1)

def split(a,b):
    m = (a+b)//2
    if a == m: # base casce
        return Frac(1,math.factorial(a))
    else:
        f1 = split(a,m)
        f2 = split(m,b)
        return f1+f2

t1 = time.perf_counter()
esum2 = split(0,terms+1)
t2 = time.perf_counter()
print('binary splitting',t2-t1)

assert esum1 == esum2

#print(esum)
