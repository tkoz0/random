'''
Generates parameters for the LCGs.
'''

import random, struct, sys

for _ in range(int(sys.argv[1])): # number of work units
    # a=1 (mod4) and c=1 (mod2) for full period length 2**32
    A0 = [random.getrandbits(30)*4+1 for _ in range(4)]
    C0 = [random.getrandbits(31)*2+1 for _ in range(4)]
    X0 = [random.getrandbits(32) for _ in range(4)]
    A1 = [random.getrandbits(30)*4+1 for _ in range(4)]
    C1 = [random.getrandbits(31)*2+1 for _ in range(4)]
    X1 = [random.getrandbits(32) for _ in range(4)]
    bytes = b''.join(struct.pack('<L',n) for n in A0+C0+X0+A1+C1+X1)
    sys.stdout.buffer.write(bytes)

