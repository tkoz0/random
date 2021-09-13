import sys

operation = sys.argv[1]
assert operation in ['encrypt','decrypt']
shift = int(sys.argv[2])

CAPITAL_LETTERS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
#LOWER_LETTERS = 'abcdefghijklmnopqrstuvwxyz'
#DIGITS = '0123456789'
WHITESPACE = ' \n\t'

lettermap = dict(i[::-1] for i in enumerate(CAPITAL_LETTERS))
encmap = dict((c,CAPITAL_LETTERS[(lettermap[c]+shift)%len(CAPITAL_LETTERS)]) for c in CAPITAL_LETTERS)
decmap = dict((c,CAPITAL_LETTERS[(lettermap[c]-shift)%len(CAPITAL_LETTERS)]) for c in CAPITAL_LETTERS)
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

