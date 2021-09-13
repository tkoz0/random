
# PC-1 table, for permuting the 64 bit key (effectively 56 bits)
# excludes bits 8,16,24,32,40,48,56
PC_1 = \
[
    57, 49, 41, 33, 25, 17,  9,
     1, 58, 50, 42, 34, 26, 18,
    10,  2, 59, 51, 43, 35, 27,
    19, 11,  3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
     7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
    21, 13,  5, 28, 20, 12,  4
]
PC_1 = [b-1 for b in PC_1] # adjust for 0-indexing

# PC-2 table, for permuting the 56 bit intermediates to form subkeys
# excludes bits 9,18,22,25,35,38,43,54
PC_2 = \
[
    14, 17, 11, 24,  1,  5,
     3, 28, 15,  6, 21, 10,
    23, 19, 12,  4, 26,  8,
    16,  7, 27, 20, 13,  2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
]
PC_2 = [b-1 for b in PC_2] # adjust for 0-indexing

# IP table, for initial permutation of a message block
IP_table = \
[
    58, 50, 42, 34, 26, 18, 10,  2,
    60, 52, 44, 36, 28, 20, 12,  4,
    62, 54, 46, 38, 30, 22, 14,  6,
    64, 56, 48, 40, 32, 24, 16,  8,
    57, 49, 41, 33, 25, 17,  9,  1,
    59, 51, 43, 35, 27, 19, 11,  3,
    61, 53, 45, 37, 29, 21, 13,  5,
    63, 55, 47, 39, 31, 23, 15,  7
]
IP_table = [b-1 for b in IP_table] # adjust for 0-indexing

# IP^-1 table, undoes the initial permutation, inverse of IP table
FP_table = \
[
    40,  8, 48, 16, 56, 24, 64, 32,
    39,  7, 47, 15, 55, 23, 63, 31,
    38,  6, 46, 14, 54, 22, 62, 30,
    37,  5, 45, 13, 53, 21, 61, 29,
    36,  4, 44, 12, 52, 20, 60, 28,
    35,  3, 43, 11, 51, 19, 59, 27,
    34,  2, 42, 10, 50, 18, 58, 26,
    33,  1, 41,  9, 49, 17, 57, 25
]
FP_table = [b-1 for b in FP_table] # adjust for 0-indexing

# E table, expands intermediate message half in f function
E_table = \
[
    32,  1,  2,  3,  4,  5,
     4,  5,  6,  7,  8,  9,
     8,  9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32,  1
]
E_table = [b-1 for b in E_table] # adjust for 0-indexing

# S boxes
S1 = \
[
    14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
     0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
     4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
]
S2 = \
[
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
     3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
     0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
]
S3 = \
[
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
     1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
]
S4 = \
[
     7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
     3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
]
S5 = \
[
     2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
]
S6 = \
[
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
     9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
     4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
]
S7 = \
[
     4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
]
S8 = \
[
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
     1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
     7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
     2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
]
S_BOX = [S1,S2,S3,S4,S5,S6,S7,S8]

# convert S box number to array of 4 bits
S_BITS = \
[
    [0,0,0,0],
    [0,0,0,1],
    [0,0,1,0],
    [0,0,1,1],
    [0,1,0,0],
    [0,1,0,1],
    [0,1,1,0],
    [0,1,1,1],
    [1,0,0,0],
    [1,0,0,1],
    [1,0,1,0],
    [1,0,1,1],
    [1,1,0,0],
    [1,1,0,1],
    [1,1,1,0],
    [1,1,1,1]
]

# P table, permutes bits from S box output
P_table = \
[
    16,  7, 20, 21,
    29, 12, 28, 17,
     1, 15, 23, 26,
     5, 18, 31, 10,
     2,  8, 24, 14,
    32, 27,  3,  9,
    19, 13, 30,  6,
    22, 11,  4, 25
]
P_table = [b-1 for b in P_table] # adjust for 0-indexing

def make_subkeys(K): # [64] -> [[48]]
    Kp = [K[i] for i in PC_1]
    C,D = Kp[:28],Kp[28:]
    subkeys = []
    # left shift schedule for generating subkeys
    for shamt in [1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1]:
        C = C[shamt:] + C[:shamt]
        D = D[shamt:] + D[:shamt]
        CD = C+D
        subkeys.append([CD[i] for i in PC_2])
    return subkeys

def S_index(i): # [6] -> int (range 0 to 63)
    return 32*i[0]+16*i[5]+8*i[1]+4*i[2]+2*i[3]+1*i[4]

def f(R,k): # [32],[48] -> [32]
    E = [R[i] for i in E_table]
    B = [E[i] ^ k[i] for i in range(48)]
    S = sum([S_BITS[S_BOX[i][S_index(B[6*i:6*i+6])]] for i in range(8)],[])
    P = [S[i] for i in P_table]
    return P

def encrypt(M,K): # [64],[64] -> [64]
    IP = [M[i] for i in IP_table]
    L,R = IP[:32],IP[32:]
    subkeys = make_subkeys(K)
    for k in subkeys:
        F = f(R,k)
        L = [L[i]^F[i] for i in range(32)]
        L,R = R,L
    RL = R+L # final swap
    FP = [RL[i] for i in FP_table]
    return FP

def decrypt(M,K): # [64],[64] -> [64]
    IP = [M[i] for i in IP_table]
    L,R = IP[:32],IP[32:]
    subkeys = make_subkeys(K)
    for k in subkeys[::-1]:
        F = f(R,k)
        L = [L[i]^F[i] for i in range(32)]
        L,R = R,L
    RL = R+L # final swap
    FP = [RL[i] for i in FP_table]
    return FP

# conversion functions
def bits2str(b):
    return ''.join('1' if bit else '0' for bit in b)
def bits2hexstr(b):
    return ''.join('%X'%(8*b[i]+4*b[i+1]+2*b[i+2]+1*b[i+3])
                   for i in range(0,len(b),4))
def hex2bits(h):
    return sum([S_BITS[int(c,16)] for c in h],[])
def int2bits(i,bits=8):
    result = []
    while bits > 0:
        result = [i%2]+result
        i //= 2
        bits -= 1
    return result
def bits2byte(i): # [8]
    return bytes([128*i[0]+64*i[1]+32*i[2]+16*i[3]+8*i[4]+4*i[5]+2*i[6]+1*i[7]])

if __name__ == '__main__':
    import sys
    block = b''
    decrypt_ = False
    if sys.argv[1] == '-d':
        decrypt_ = True
        key = hex2bits(sys.argv[2])
    else:
        key = hex2bits(sys.argv[1])
        if len(sys.argv) > 2 and sys.argv[2] == '-d':
            decrypt_ = True
    while True:
        block = sys.stdin.buffer.read(8)
        if block == b'':
            break
        if len(block) != 8:
            block += b'\x00'*(8-len(block))
        block = sum(map(int2bits,list(block)),[])
        if decrypt_:
            block = decrypt(block,key)
        else:
            block = encrypt(block,key)
        sys.stdout.buffer.write(b''.join(map(bits2byte,
                                        (block[i:i+8] for i in range(0,64,8)))))
