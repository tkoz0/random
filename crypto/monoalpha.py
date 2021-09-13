import sys

operation = sys.argv[1]
assert operation in ['encrypt','decrypt']
key = sys.argv[2]
CAPITAL_LETTERS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
assert len(key) == len(CAPITAL_LETTERS) and set(key) == set(CAPITAL_LETTERS)
WHITESPACE = ' \n\t'

lettermap = dict(i[::-1] for i in enumerate(CAPITAL_LETTERS))
encmap = dict()
decmap = dict()
for i,c in enumerate(CAPITAL_LETTERS):
    encmap[c] = key[i]
    decmap[key[i]] = c
for c in WHITESPACE:
    encmap[c] = c
    decmap[c] = c

def encrypt(plaintext):
    return ''.join(encmap[c] for c in plaintext)

def decrypt(plaintext):
    return ''.join(decmap[c] for c in plaintext)

for line in sys.stdin:
    if operation == 'encrypt':
        print(encrypt(line[:-1]))
    else:
        print(decrypt(line[:-1]))

