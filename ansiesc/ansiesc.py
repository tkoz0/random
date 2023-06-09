
class _escm:
    '''
    internal class for ansi escape codes
    only supports the 'm' command for text color/format
    support concatenations with each other
    support concatenations with strings
    '''
    def __init__(self, *l: int|list[int]):
        self.l: list[int] = []
        for n in l:
            if isinstance(n,int):
                self.l.append(n)
            else:
                self.l += n
    def __eq__(self, l):
        return isinstance(l,_escm) and self.l == l.l
    def __str__(self) -> str:
        return f'\033[{";".join(str(n) for n in self.l)}m'
    def __repr__(self) -> str:
        return f'{type(self).__name__}({",".join(str(n) for n in self.l)})'
    def __iadd__(self, l) -> '_escm':
        assert isinstance(l,_escm)
        self.l += l.l
        return self
    def __add__(self, l) -> '_escm|str':
        if isinstance(l,str):
            return str(self) + l
        elif isinstance(l,_escm):
            return _escm(self.l+l.l)
        else:
            raise TypeError()
    def __radd__(self, l) -> '_escm|str':
        if isinstance(l,str):
            return l + str(self)
        elif isinstance(l,_escm):
            return _escm(l.l+self.l)
        else:
            raise TypeError()

# text styles
RESET = _escm(0)
BOLD = _escm(1)
FAINT = _escm(2)
ITALIC = _escm(3)
ULINE = _escm(4)
BLINK_SLOW = _escm(5)
BLINK_FAST = _escm(6)
INVERT_ON = _escm(7)
HIDE_ON = _escm(8)
CROSS_ON = _escm(9)
def FONT(n: int = 0) -> _escm:
    # probably not supported
    if n < 0 or n >= 10:
        raise ValueError()
    return _escm(10+n)
ULINE2 = _escm(21)
NORM_INTENSITY = _escm(22)
NORM_STYLE = _escm(23)
ULINE_OFF = _escm(24)
BLINK_OFF = _escm(25)
INVERT_OFF = _escm(27)
HIDE_OFF = _escm(28)
CROSS_OFF = _escm(29)

# normal (non bright) colors (3 bit)
# FG = foreground/text
# BG = background
FG_BLACK = _escm(30)
FG_RED = _escm(31)
FG_GREEN = _escm(32)
FG_YELLOW = _escm(33)
FG_BLUE = _escm(34)
FG_MAGENTA = _escm(35)
FG_CYAN = _escm(36)
FG_WHITE = _escm(37)
BG_BLACK = _escm(40)
BG_RED = _escm(41)
BG_GREEN = _escm(42)
BG_YELLOW = _escm(43)
BG_BLUE = _escm(44)
BG_MAGENTA = _escm(45)
BG_CYAN = _escm(46)
BG_WHITE = _escm(47)

# bright colors (4 bit extension)
# the extra B at the start of the color name means bright
FG_BBLACK = _escm(90)
FG_BRED = _escm(91)
FG_BGREEN = _escm(92)
FG_BYELLOW = _escm(93)
FG_BBLUE = _escm(94)
FG_BMAGENTA = _escm(95)
FG_BCYAN = _escm(96)
FG_BWHITE = _escm(97)
BG_BBLACK = _escm(100)
BG_BRED = _escm(101)
BG_BGREEN = _escm(102)
BG_BYELLOW = _escm(103)
BG_BBLUE = _escm(104)
BG_BMAGENTA = _escm(105)
BG_BCYAN = _escm(106)
BG_BWHITE = _escm(107)

# arrays with all 3 or 4 bit colors
FGB = [_escm(n) for n in range(90,98)]
FG = [_escm(n) for n in range(30,38)] + FGB
BGB = [_escm(n) for n in range(100,108)]
BG = [_escm(n) for n in range(40,48)] + BGB

# 8 bit color
# 0-7 = standard
# 8-15 = bright
# 16-231 = 6x6x6 cube 16+36*r+6*g+b for 0<=r,g,b<6
# 232-255 = black to white grayscale
def FG8(n: int) -> _escm:
    if n < 0 or n >= 256:
        raise ValueError()
    return _escm(38,5,n)

def BG8(n: int) -> _escm:
    if n < 0 or n >= 256:
        raise ValueError()
    return _escm(48,5,n)

# 24 bit color (standard rgb)
def FG24(r: int, g: int, b: int) -> _escm:
    if r < 0 or r >= 256 or g < 0 or g >= 256 or b < 0 or b >= 256:
        raise ValueError()
    return _escm(38,2,r,g,b)

def BG24(r: int, g: int, b: int) -> _escm:
    if r < 0 or r >= 256 or g < 0 or g >= 256 or b < 0 or b >= 256:
        raise ValueError()
    return _escm(48,2,r,g,b)

# cursor position
def CURSOR_MOVE(r: int, c: int) -> str:
    if r < 0 or c < 0:
        raise ValueError()
    return f'\033[{r};{c}H'

def CURSOR_MOVE_UP(n: int) -> str:
    if n < 0:
        raise ValueError()
    return f'\033[{n}A'

def CURSOR_MOVE_DOWN(n: int) -> str:
    if n < 0:
        raise ValueError()
    return f'\033[{n}B'

def CURSOR_MOVE_RIGHT(n: int) -> str:
    if n < 0:
        raise ValueError()
    return f'\033[{n}C'

def CURSOR_MOVE_LEFT(n: int) -> str:
    if n < 0:
        raise ValueError()
    return f'\033[{n}D'

def CURSOR_MOVE_LINE(n: int) -> str:
    if n == 0:
        raise ValueError()
    elif n > 0:
        return f'\033[{n}E'
    else:
        return f'\033[{n}F'

def CURSOR_MOVE_COL(n: int) -> str:
    if n < 0:
        raise ValueError()
    return f'\033[{n}G'

# clearing
CLR_SCR_TO_END = '\033[0J'
CLR_SCR_TO_BEG = '\033[1J'
CLR_SCR_ALL = '\033[2J'
CLR_SCR_BUF = '\033[3J'
CLR_LINE_TO_END = '\033[0K'
CLR_LINE_TO_BEG = '\033[1K'
CLR_LINE_ALL = '\033[2K'

# scrolling
def SCROLL_UP(n: int) -> str:
    if n < 1:
        raise ValueError()
    return f'\033[{n}S'

def SCROLL_DOWN(n: int) -> str:
    if n < 1:
        raise ValueError()
    return f'\033[{n}T'
