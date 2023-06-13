from fractions import Fraction as Frac
import math

# each is a single variable polynomial
# list of fractions for x^0,x^1,x^2,... terms
# length must be >= 1, zero polynomial is a list containing only 0
# index p represents power sum with exponent p
formulas: list[list[Frac]] = []
formulas.append([Frac(0),Frac(1)]) # p = 0
#formulas.append([Frac(0),Frac(1,2),Frac(1,2)]) # p = 1
#formulas.append([Frac(0),Frac(1,6),Frac(1,2),Frac(1,3)]) # p = 2
#formulas.append([Frac(0),Frac(0),Frac(1,4),Frac(1,2),Frac(1,4)]) # p = 3

def polyeval(poly: list[Frac], x: Frac) -> Frac:
    # evaluate polynomial function, not using horner's method
    xp = Frac(1)
    ret = Frac(0)
    for c in poly:
        ret += c*xp
        xp *= x
    return ret

def polyadd(poly1: list[Frac], poly2: list[Frac]) -> list[Frac]:
    # add 2 polynomials
    ret = [Frac(0)]*max(len(poly1),len(poly2))
    for i,c in enumerate(poly1):
        ret[i] += c
    for i,c in enumerate(poly2):
        ret[i] += c
    while ret and ret[-1] == 0:
        ret.pop()
    return ret if ret else [Frac(0)]

def polymul(poly1: list[Frac], poly2: list[Frac]) -> list[Frac]:
    # multiply 2 polynomials, slow n^2 method
    ret = [Frac(0)]*(len(poly1)+len(poly2)-1)
    for i,c in enumerate(poly1):
        for j,d in enumerate(poly2):
            ret[i+j] += c*d
    while ret and ret[-1] == 0:
        ret.pop()
    return ret if ret else [Frac(0)]

def testformula(formula: list[Frac], p: int, lim: int) -> bool:
    # check power sum formula for correctness
    powersum = 0
    for n in range(lim):
        powersum += n**p
        polyval = polyeval(formula,Frac(n))
        if polyval != powersum:
            print(f'p={p},n={n},powersum={powersum},polyeval={polyval}')
            return False
    return True

def choose(n: int, k: int) -> int:
    # for binomial coefficients
    return math.factorial(n) // math.factorial(k) // math.factorial(n-k)

def printpoly(poly: list[Frac]) -> str:
    # make a string representation
    terms = []
    for i in range(len(poly)-1,-1,-1):
        if poly[i] < 0:
            terms.append(f'- {abs(poly[i])}*n^{i}')
        elif poly[i] > 0:
            terms.append(f'+ {poly[i]}*n^{i}')
    return ' '.join(terms) if terms else '0*n^0'

def findnext():
    # generate the next power sum formula and append to global array
    global formulas
    p = len(formulas)
    #print(f'findnext(): p = {p}')
    # evaluate sum(i=1,n)[ sum(j=1,n)[j^(p-1)] - sum(j=1,i-1)[j^(p-1)] ]
    # function of n from the first inner summation (multiply by n)
    n_part = polymul([Frac(0),Frac(1)],formulas[p-1])
    #print(f'n_part = {printpoly(n_part)}')
    # function of i inside sum(i=1,n) summation, top is i-1
    i_part = [Frac(0)]*(p+1)
    for m,c in enumerate(formulas[p-1]):
        # add c*(i-1)^m to i_part
        for k in range(m+1):
            i_part[k] += c * choose(m,k) * (-1)**(m-k)
    i_part = [-c for c in i_part] # negate since it is subtracted
    #print(f'i_part = {printpoly(i_part)}')
    # for one side, const_factor * sum(i=1,n) i^p
    const_factor = Frac(1) - i_part[p] # should be 1+1/p
    #print(f'const_factor = {const_factor}')
    # other side is n_part + sum(i=1,n) i_part (after pop off last one)
    i_part.pop()
    # add summation of i_part to n_part by using lower power sums
    for m,c in enumerate(i_part):
        n_part = polyadd(n_part,polymul([c],formulas[m]))
    # divide constant factor
    n_part = [c/const_factor for c in n_part]
    formulas.append(n_part)
    #print(f'p={p}: {printpoly(formulas[-1])}')

while len(formulas) <= 20:
    findnext()

for p,f in enumerate(formulas):
    assert testformula(f,p,2*p)
    #print(printpoly(f))
    # remove denominators for nicer formatting
    mult = 1
    all_ints = False
    while not all_ints:
        all_ints = all(c.denominator == 1 for c in f)
        if all_ints: break
        for c in f:
            if c.denominator == 1: continue
            mult *= c.denominator
            f = polymul([Frac(c.denominator)],f)
            break
    assert testformula(polymul([Frac(1,mult)],f),p,2*p)
    print(f'p={p}: 1/{mult} * [ {printpoly(f)} ]')
