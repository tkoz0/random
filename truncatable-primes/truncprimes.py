'''
Truncatable primes program
Usage: truncprimes.py <type> <base>
Types supported: left, right, left-and-right
'''

if __name__ != '__main__': quit()

#from prp import prp, sprp
import sys

# input n should be 0 (recursion root) or prime
# b >= 2 is base
def right_trunc(b,n=0):
    n *= b # shift left making the ones digit 0
    for d in range(1,b):
        n += 1 # increment rightmost digit
        if sprp(n):
            yield n
            yield from right_trunc(b,n)

# input n should be 0 (recursion root) or prime
# v is value to add for left appending a digit (b**n for n >= 0)
# b >= 2 is base
def left_trunc(b,v=1,n=0):
    for d in range(1,b):
        n += v # increase the new leftmost digit from 0
        if sprp(n):
            yield n
            yield from left_trunc(b,v*b,n)

# input n should be 0 (recursion root) or prime
# v is value for adding left digit
# b >= 2 is base
def lr_trunc_helper(b,v,n):
    n *= b
    for d1 in range(1,b):
        n += v*b
        for d2 in range(1,b):
            if sprp(n+d2):
                yield n+d2
                yield from lr_trunc_helper(b,v*b*b,n+d2)

def lr_trunc(b):
    for d in range(2,b):
        if sprp(d): # initialize with single digit primes
            yield from lr_trunc_helper(b,b*b,d)
    for dd in range(b,b*b):
        if sprp(dd): # initialize with double digit primes
            yield from lr_trunc_helper(b,b*b*b,dd)

# parse command line
type = sys.argv[1]
assert type in ['left','right','left-and-right']
base = int(sys.argv[2])
assert base >= 2

if type == 'left':
    primes = left_trunc(base)
elif type == 'right':
    primes = right_trunc(base)
elif type == 'left-and-right':
    primes = lr_trunc(base)

for p in primes:
    print(p)

