'''
Generates parameters for the LCGs.

For now, generates 960KiB worksets which each will produce roughly 10GB of
output. Parameters (a and c pairs) are stored in RAM to prevent duplicates,
since LCGs with the same a and c value will produce the same sequence. The
parameters are chosen so the resulting LCGs have the full 2**32 period length.
The parameters are chosen by using the random module in Python3.
'''

import random, struct, sys

parameters = set() # store (a,c) pairs

def get_ac(): # returns an a,c pair for LCG that hasnt been used already
    global parameters
    while True:
        a = random.getrandbits(30)*4+1
        c = random.getrandbits(31)*2+1
        if (a,c) not in parameters:
            parameters.add((a,c))
            return (a,c)

def get_work_unit(): # returns the next work unit as a byte string of length 96
    AC = [get_ac() for _ in range(8)] # (a,c) pairs
    XX = [random.getrandbits(32) for _ in range(8)]
    A0 = [AC[i][0] for i in range(4)]
    C0 = [AC[i][1] for i in range(4)]
    X0 = XX[:4]
    A1 = [AC[i][0] for i in range(4,8)]
    C1 = [AC[i][1] for i in range(4,8)]
    X1 = XX[4:]
    bytes = b''.join(struct.pack('<L',n) for n in A0+C0+X0+A1+C1+X1);
    assert len(bytes) == 96
    return bytes

for i in range(1000):
    # 10000 parameter sets per file
    print('writing file',i)
    outfile = open('worksets/ws10K.%06d.workset'%i,'wb')
    for _ in range(10000):
        outfile.write(get_work_unit())
    outfile.flush()
    outfile.close()

quit() # TODO code below is for reuse to improve this code

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

