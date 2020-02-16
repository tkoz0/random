import curses
import collections

# label is a string, items is a list strings and/or submenus
Menu = collections.namedtuple('Menu',['label','items'])
menus = [Menu('file',['new','open','save','exit']),
         Menu('edit',['preferences']),
         Menu('help',['documentation','about'])]
menucoord = []
FKEYS = [curses.KEY_F1,curses.KEY_F2,curses.KEY_F3]
R,C = None,None

def init_color_pairs():
    curses.init_pair(1,curses.COLOR_BLACK,curses.COLOR_BLUE)
    curses.init_pair(2,curses.COLOR_BLACK,curses.COLOR_CYAN)
    curses.init_pair(3,curses.COLOR_BLACK,curses.COLOR_GREEN)
    curses.init_pair(4,curses.COLOR_BLACK,curses.COLOR_MAGENTA)
    curses.init_pair(5,curses.COLOR_BLACK,curses.COLOR_RED)
    curses.init_pair(6,curses.COLOR_BLACK,curses.COLOR_WHITE)
    curses.init_pair(7,curses.COLOR_BLACK,curses.COLOR_YELLOW)

def draw_title(stdscr):
    stdscr.addstr(0,0,'Title'+' '*(C-5),curses.color_pair(3))

def draw_main_menus(stdscr):
    global menus,menucoord,FKEYS,R,C
    assert len(menus) < 10
    c = 0
    for i in range(len(menus)):
        stdscr.addstr(1,c,'F%d'%(i+1),curses.color_pair(1))
        stdscr.addstr(1,c+2,' '+menus[i].label+' ',curses.color_pair(2))
        menucoord.append((1,c+3))
        c += 4+len(menus[i].label)
    stdscr.addstr(1,c,' '*(C-c),curses.color_pair(3))

def main(stdscr):
    global R,C
    R,C = stdscr.getmaxyx()
    init_color_pairs()
    draw_title(stdscr)
    draw_main_menus(stdscr)
    stdscr.move(2,0)
    while True:
        ch = stdscr.getch()
        if chr(ch).lower() == 'q': break

curses.wrapper(main)
