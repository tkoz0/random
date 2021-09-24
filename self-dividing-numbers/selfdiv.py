import sys

def is_self_div(x):
    x2 = x
    while x2:
        x2,rem = divmod(x2,10)
        if rem == 0: return False
        if x % rem != 0: return False
    return True

def brute_force(limit):
    yield from filter(is_self_div,range(1,1+limit))

def digits_arr_and_lcm(prefix):
    digits = [False]*10
    lcm = 1
    while prefix:
        prefix,rem = divmod(prefix,10)
        digits[rem] = True
    if any(digits[x] for x in [2,4,6,8]):
        lcm *= 2
    if any(digits[x] for x in [4,8]):
        lcm *= 2
    if digits[8]:
        lcm *= 2
    if any(digits[x] for x in [3,6,9]):
        lcm *= 3
    if digits[9]:
        lcm *= 3
    if digits[5]:
        lcm *= 5
    if digits[7]:
        lcm *= 7
    return digits,lcm

def prefix_mode(limit,non_prefix_digits=3):
    prefix_max = limit//10**non_prefix_digits
    yield from filter(is_self_div,range(1,min(1+limit,10**non_prefix_digits)))
    for prefix in range(1,prefix_max):
        digits,lcm = digits_arr_and_lcm(prefix)
        if digits[0]: # 0 can't divide number
            continue
        hi = (prefix+1)*10**non_prefix_digits
        lo = prefix*10**non_prefix_digits-1
        lo += lcm - (lo%lcm)
        for n in range(lo,hi,lcm):
            assert n % lcm == 0
            n2 = n
            fail = False
            for _ in range(non_prefix_digits):
                n2,rem = divmod(n2,10)
                if rem == 0:
                    fail = True
                    break
                if not digits[rem] and n % rem != 0:
                    fail = True
                    break
            if not fail: yield n
    lo = prefix_max*10**non_prefix_digits
    yield from filter(is_self_div,range(lo,1+limit))

limit = int(sys.argv[1])
mode = sys.argv[2]
if mode == 'pm':
    nums = prefix_mode(limit)
elif mode == 'bf':
    nums = brute_force(limit)
else:
    assert 0

for n in nums:
    print(n)
