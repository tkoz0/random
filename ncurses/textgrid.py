import curses
import sys

# TODO make set of nonprintable ncurses key codes
keyspecial = {
    curses.KEY_MIN,
    curses.KEY_BREAK,
    curses.KEY_DOWN,
    curses.KEY_UP,
    curses.KEY_LEFT,
    curses.KEY_RIGHT,
    curses.KEY_HOME,
    curses.KEY_BACKSPACE,
    curses.KEY_F0,
    curses.KEY_F1,
    curses.KEY_F2,
    curses.KEY_F3,
    curses.KEY_F4,
    curses.KEY_F5,
    curses.KEY_F6,
    curses.KEY_F7,
    curses.KEY_F8,
    curses.KEY_F9,
    curses.KEY_F10,
    curses.KEY_F11,
    curses.KEY_F12,
    curses.KEY_DL,
    curses.KEY_IL,
    curses.KEY_DC,
    curses.KEY_IC,
    curses.KEY_EIC,
    curses.KEY_CLEAR,
    curses.KEY_EOS,
    curses.KEY_EOL,
    curses.KEY_SF,
    curses.KEY_SR,
    curses.KEY_NPAGE,
    curses.KEY_PPAGE,
    curses.KEY_STAB,
    curses.KEY_CTAB,
    curses.KEY_CATAB,
    curses.KEY_ENTER,
    curses.KEY_SRESET,
    curses.KEY_RESET,
    curses.KEY_PRINT,
    curses.KEY_LL,
    curses.KEY_A1,
    curses.KEY_A3,
    curses.KEY_B2,
    curses.KEY_C1,
    curses.KEY_C3,
    curses.KEY_BTAB,
    curses.KEY_BEG,
    curses.KEY_CANCEL,
    curses.KEY_CLOSE,
    curses.KEY_COMMAND,
    curses.KEY_COPY,
    curses.KEY_CREATE,
    curses.KEY_END,
    curses.KEY_EXIT,
    curses.KEY_FIND,
    curses.KEY_HELP,
    curses.KEY_MARK,
    curses.KEY_MESSAGE,
    curses.KEY_MOVE,
    curses.KEY_NEXT,
    curses.KEY_OPEN,
    curses.KEY_OPTIONS,
    curses.KEY_PREVIOUS,
    curses.KEY_REDO,
    curses.KEY_REFERENCE,
    curses.KEY_REFRESH,
    curses.KEY_REPLACE,
    curses.KEY_RESTART,
    curses.KEY_RESUME,
    curses.KEY_SAVE,
    curses.KEY_SBEG,
    curses.KEY_SCANCEL,
    curses.KEY_SCOMMAND,
    curses.KEY_SCOPY,
    curses.KEY_SCREATE,
    curses.KEY_SDC,
    curses.KEY_SDL,
    curses.KEY_SELECT,
    curses.KEY_SEND,
    curses.KEY_SEOL,
    curses.KEY_SEXIT,
    curses.KEY_SFIND,
    curses.KEY_SHELP,
    curses.KEY_SHOME,
    curses.KEY_SIC,
    curses.KEY_SLEFT,
    curses.KEY_SMESSAGE,
    curses.KEY_SMOVE,
    curses.KEY_SNEXT,
    curses.KEY_SOPTIONS,
    curses.KEY_SPREVIOUS,
    curses.KEY_SPRINT,
    curses.KEY_SREDO,
    curses.KEY_SREPLACE,
    curses.KEY_SRIGHT,
    curses.KEY_SRSUME,
    curses.KEY_SSAVE,
    curses.KEY_SSUSPEND,
    curses.KEY_SUNDO,
    curses.KEY_SUSPEND,
    curses.KEY_UNDO,
    curses.KEY_MOUSE,
    curses.KEY_RESIZE,
    curses.KEY_MAX }

R = int(sys.argv[1])
C = int(sys.argv[2])

def clear_region(stdscr,r1,c1,r2,c2):
    for r in range(r1,r2+1): stdscr.addstr(r,c1,' '*(1+c2-c1))

def draw_border(stdscr,r1,c1,r2,c2,attr):
    assert 0 <= r1 < r2 and 0 <= c1 < c2
    stdscr.addch(r1,c1,curses.ACS_ULCORNER,attr)
    stdscr.addch(r1,c2,curses.ACS_URCORNER,attr)
    stdscr.addch(r2,c1,curses.ACS_LLCORNER,attr)
    stdscr.addch(r2,c2,curses.ACS_LRCORNER,attr)
    for c in range(c1+1,c2):
        stdscr.addch(r1,c,curses.ACS_HLINE,attr)
        stdscr.addch(r2,c,curses.ACS_HLINE,attr)
    for r in range(r1+1,r2):
        stdscr.addch(r,c1,curses.ACS_VLINE,attr)
        stdscr.addch(r,c2,curses.ACS_VLINE,attr)

def write_file(stdscr,grid): # ask for file name and save
    global keyspecial
    stdscr.clear()
    stdscr.addstr(0,0,'Enter file name')
    stdscr.move(1,0)
    c = 0
    fname = ''
    while True:
        ch = stdscr.getch()
        if ch == ord('\n'): break
        elif ch == curses.KEY_BACKSPACE and c > 0:
            stdscr.addch(1,c-1,' ')
            stdscr.move(1,c-1)
            c -= 1
            fname = fname[:-1]
        elif ch in keyspecial: pass
        else:
            stdscr.addch(1,c,chr(ch))
            fname += chr(ch)
            c += 1
    with open(fname,'w') as file:
        for row in grid: file.write(''.join(row)+'\n')
        file.close()

def main(stdscr):
    global R,C,keyspecial
    draw_border(stdscr,0,0,R+1,C+1,0)
    stdscr.addstr(R+2,0,'F1 = quit\nF2 = save')
    stdscr.move(1,1)
    r,c = 1,1
    grid = [[' ']*C for _ in range(R)]
    while True:
        ch = stdscr.getch()
        if ch == curses.KEY_F1: return
        if ch == curses.KEY_F2:
            write_file(stdscr,grid)
            return
        if ch == curses.KEY_LEFT:    c = max(1,c-1); stdscr.move(r,c)
        elif ch == curses.KEY_RIGHT: c = min(C,c+1); stdscr.move(r,c)
        elif ch == curses.KEY_UP:    r = max(1,r-1); stdscr.move(r,c)
        elif ch == curses.KEY_DOWN:  r = min(R,r+1); stdscr.move(r,c)
        elif ch == curses.KEY_DC:
            stdscr.addch(r,c,' ')
            grid[r-1][c-1] = ' '
            stdscr.move(r,c)
        elif ch == curses.KEY_BACKSPACE and c > 1:
            c -= 1
            stdscr.addch(r,c,' ')
            grid[r-1][c-1] = ' '
            stdscr.move(r,c)
        elif ch == ord('\n'): # new line from enter key
            if r < R: # dont move off the box
                r,c = r+1,1
                stdscr.move(r,c)
        elif ch in keyspecial: pass
        else:
            stdscr.addch(r,c,ch)
            grid[r-1][c-1] = chr(ch)
            if c == C: stdscr.move(r,c)
            else: c += 1

curses.wrapper(main) # do the setup and prevent messing up the terminal
