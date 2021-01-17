
class InvalidTextException(Exception):
    ''' text not supported '''

# rot13 character mapping
_rot13 = dict()
_rot13chars1 = 'ABCDEFGHIJKLM abcdefghijklm'
_rot13chars2 = 'NOPQRSTUVWXYZ nopqrstuvwxyz'
for i in range(27):
    _rot13[_rot13chars1[i]] = _rot13chars2[i]
    _rot13[_rot13chars2[i]] = _rot13chars1[i]

class CryptoFunctions:
    def encrypt_rot13(text):
        try: return ''.join(_rot13[c] for c in text)
        except: raise InvalidTextException()
    def decrypt_rot13(text):
        try: return ''.join(_rot13[c] for c in text)
        except: raise InvalidTextException()
