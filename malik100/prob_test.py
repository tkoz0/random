'''
Test the probabilistic approach that relies on the birthday problem for finding
collisions.
'''

import math
import random

# LCG wiwh modulus 900000 for generating random 6 digit numbers
#c = 1103
#c = 69691
c = 412619
m = 900000 # m = 2^5 * 3^2 * 5^5
#a = 6901
#a = 61081
a = 721321
assert math.gcd(m,c) == 1
assert (a-1) % 30 == 0 # 2,3,5 divides a-1 iff 30 divides a-1
assert (m % 4 != 0) or ((a-1) % 4 == 0) # 4|m --> 4|(a-1)

X = 318617 # seed
numbers = []
for _ in range(25): # generate 128 numbers
    X = (a*X + c) % m
    numbers.append(100000 + X);

print(numbers)
print('sum =',sum(numbers)) # 13744875
print('subset count / sum =',2**25/sum(numbers))

# choose 106*2^16 = 6946816 and 107*2^16 = 7012352 as sum range bounds
# determine by shifting sum right 16 bits, should equal 106

def generate_subsets(NUM_SUBSETS):
    global numbers
    subsets = []

    while True:
        subset = random.getrandbits(25)
        subsetsum = 0
        bit = 2**24
        i = 24
        while bit:
            if subset & bit != 0:
                subsetsum += numbers[i]
            bit = bit >> 1
            i -= 1
        if (subsetsum >> 16) == 106:
            subsets.append((subsetsum,subset))
        if len(subsets) >= NUM_SUBSETS:
            return subsets

def get_duplicates(subsets):
    subsetsort = sorted(subsets)
    result = []
    for i in range(1,len(subsetsort)): # search for duplicates
        if subsetsort[i][0] == subsetsort[i-1][0]: # same sum
            result.append((subsetsort[i],subsetsort[i-1]))
    return result

def show_dupe(duplicate):
    dup1,dup2 = duplicate[0][1], duplicate[1][1]
    print('a duplicate =',dup1,dup2)

    set1,set2 = set(),set()
    n1,n2 = dup1,dup2
    i = 24
    bit = 2**24
    while bit:
        if n1 & bit != 0: set1.add(numbers[i])
        if n2 & bit != 0: set2.add(numbers[i])
        i -= 1
        bit >>= 1
    shared = set1 & set2
    assert set1 <= set(numbers)
    assert set2 <= set(numbers)
    print('duplicate sets:')
    print(set1)
    print(set2)
    set1 -= shared
    set2 -= shared
    print('with duplicates removed:')
    print(set1)
    print(set2)

success = 0
duplicate = None
for i in range(100):
    ss = generate_subsets(550)
    dups = get_duplicates(ss)
    if len(dups) > 0:
        success += 1
        duplicate = dups[0]
        show_dupe(duplicate)
        quit()
    print('trial',i,'success' if len(dups) > 0 else 'failure')
print('probability =',success/100)
show_dupe(duplicate)
