import random

def hash(s):
    h = 0
    for c in s:
        h = (31*h+ord(c))%2**32
    return h

#CHARSET = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
CHARSET = ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~'

changemap = dict() # (pos,oldchar,newchar) -> delta hash
deltahash = dict() # delta hash -> (pos,oldchar,newchar)

for pos in range(20):
    pow31 = pow(31,pos,2**32)
    for oldchar in CHARSET:
        for newchar in CHARSET:
            if oldchar == newchar:
                continue
            dh = (pow31 * (ord(newchar) - ord(oldchar))) % 2**32
            key = (pos,oldchar,newchar)
            changemap[key] = dh
            if dh not in deltahash:
                deltahash[dh] = []
            deltahash[dh].append(key)

print(f'created {len(changemap)} cached entries')
print(f'total of {len(deltahash)} delta hashes')
print(f'is 0 in deltahash? {0 in deltahash}')

MINLEN = 10
MAXLEN = 20
STRLIM = 10

print('generating strings')

strs_found = []
while len(strs_found) < STRLIM:
    s = [random.choice(CHARSET) for _ in range(random.randint(MINLEN,MAXLEN))]
    h = 0
    edits = 0 # number of changes to random string
    for c in s:
        h = (31*h+ord(c))%2**32
    assert h == hash(s)
    print(f'trying initial string {repr("".join(s))} with hash {h}')
    # hash = 31**(n-1)*s[0] + 31**(n-2)*s[1] + ... + 31*s[-2] + s[-1]
    for _ in range(10000000):
        dh = 2**32 - h
        found = False
        if dh in deltahash:
            print(f'dh in deltahash after {edits} edits {repr("".join(s))}')
            for p,old,new in deltahash[dh]:
                if p < len(s) and s[-p-1] == old:
                    s[-p-1] = new
                    print(f'found {repr("".join(s))} after {edits} edits')
                    found = True
                    strs_found.append(s)
                    assert hash(s) == 0
                    break
        if not found: # make a random change to s
            p = random.randint(0,len(s)-1)
            ch = random.choice(CHARSET)
            while ch == s[-p-1]:
                ch = random.choice(CHARSET)
            h = (h+changemap[(p,s[-p-1],ch)])%2**32
            s[-p-1] = ch
            edits += 1
            #assert h == hash(s)
        else:
            break # move on to another random string

print('ZERO HASH STRINGS FOUND')
for s in strs_found:
    print(repr(''.join(s)))
