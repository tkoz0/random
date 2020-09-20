'''
Read subsets from a file and compute some statistics on their sums.
'''

import random

numbers = [
    5719825393567961346558155629,
    5487945882843158696672157984,
    4767766531754254874224257763,
    1855924359757732125866239784,
    4289776424589197647513647977,
    7967131961768854889594217186,
    2572967277666133789225764888,
    1294587141921952639693619381,
    4764413635323911361699183586,
    1474343641823476922667154474,
    2578649763684913163429325833,
    5161596985226568681977938754,
    2242632698981685551523361879,
    7474189614567412367516833398,
    6211855673345949471748161445,
    4942716233498772219251848674,
    5516264359672753836539861178,
    5854762719618549417768925747,
    5313691171963952518124735471,
    6737691754241231469753717635,
    4292388614454146728246198812,
    4468463715866746258976552344,
    2638621731822362373162811879,
    1258922263729296589785418839,
    4482279727264797827654899397,
    8749855322285371162986411895,
    1116599457961971796683936952,
    3879213273596322735993329751,
    9212359131574159657168196759,
    3351223183818712673691977472,
    8855835322812512868896449976,
    4332859486871255922555418653,
    2428751582371964453381751663,
    6738481866868951787884276161,
    8794353172213177612939776215,
    2989694245827479769152313629,
    6117454427987751131467589412,
    2761854485919763568442339436,
    6884214746997985976433695787,
    8671829218381757417536862814,
    9431156837244768326468938597,
    4788448664674885883585184169,
    3624757247737414772711372622,
    9361819764286243182121963365,
    9893315516156422581529354454,
    5913625989853975289562158982,
    8313891548569672814692858479,
    2265865138518379114874613969,
    3477184288963424358211752214,
    6321349612522496241515883378,
    1796439694824213266958886393,
    6366252531759955676944496585,
    8545458545636898974365938274,
    3362291186211522318566852576,
    8464473866375474967347772855,
    2892857564355262219965984217,
    4296693937661266715382241936,
    8634764617265724716389775433,
    8415234243182787534123894858,
    2267353254454872616182242154,
    4689911847578741473186337883,
    4428766787964834371794565542,
    7146295186764167268433238125,
    2273823813572968577469388278,
    6686132721336864457635223349,
    3161518296576488158997146221,
    1917611425739928285147758625,
    3516431537343387135357237754,
    7549684656732941456945632221,
    2397876675349971994958579984,
    4675844257857378792991889317,
    2832515241382937498614676246,
    8755442772953263299368382378,
    9833662825734624455736638328,
    5298671253425423454611152788,
    9857512879181186421823417538,
    1471226144331341144787865593,
    3545439374321661651385735599,
    6735367616915626462272211264,
    2141665754145475249654938214,
    8481747257332513758286947416,
    9961217236253576952797397966,
    9941237996445827218665222824,
    6242177493463484861915865966,
    4344843511782912875843632652,
    7568842562748136518615117797,
    2776621559882146125114473423,
    6174299197447843873145457215,
    5387584131525787615617563371,
    5317693353372572284588242963,
    6612142515552593663955966562,
    1314928587713292493616625427,
    2446827667287451685939173534,
    9786693878731984534924558138,
    2926718838742634774778713813,
    3791426274497596641969142899,
    2831727715176299968774951996,
    3281287353463725292271916883,
    9954744594922386766735519674,
    3414339143545324298853248718
]

import sys, struct

infile = open(sys.argv[1],'rb')
subsetsums = []
subsetbits = []
data_read = 0

check_sorted = sys.argv[2] == 'check'
write_sums = sys.argv[3] == 'write'
prev_sum = 0

sumsfile = None
if write_sums:
    sumsfile = open('sums.%d.txt'%random.getrandbits(31),'w');

while True:
    subset = infile.read(16)
    bits = 0
    if len(subset) != 16: break
    data_read += 16
    if data_read % 262144 == 0: print('read %f MB'%(data_read/1048576))
    first64 = struct.unpack('<Q',subset[:8])[0]
    bits = first64
    subsetsum = 0
    for i in range(64):
        subsetsum += (first64 & 1) * numbers[i]
        first64 = first64 >> 1
    second,third = struct.unpack('<LL',subset[8:])
    last36 = (second & 0x1FF) | ((second >> 23) << 9) \
            | ((third & 0x1FF) << 18) | ((third >> 23) << 27)
    bits = bits | (last36 << 64)
    for i in range(64,100):
        subsetsum += (last36 & 1) * numbers[i]
        last36 = last36 >> 1
    if check_sorted and subsetsum < prev_sum:
        print('ERROR:',subsetsum,'<',prev_sum)
        print('read',data_read//16,'subsets')
        assert 0
    if write_sums:
        sumsfile.write('%d,%d\n'%divmod(subsetsum,2**56))
    if prev_sum == subsetsum: # duplicate
        print('duplicates at offsets %d and %d'%(data_read-32,data_read-16))
    prev_sum = subsetsum
    subsetsums.append(subsetsum)
    subsetbits.append(bits)

if sumsfile: sumsfile.close()

print('min =',min(subsetsums))
print('max =',max(subsetsums))
print('diff =',max(subsetsums)-min(subsetsums))
print(len(subsetsums),'subsetsums',len(set(subsetsums)),'deduped')
print(len(subsetbits),'subsetbits',len(set(subsetbits)),'deduped')
