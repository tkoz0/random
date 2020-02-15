import curses
import sys

# infinite loop to display key code information
# useful to determine key codes for writing ncurses programs
# press ctrl+c to exit

def main(stdscr):
    while True:
        c = stdscr.getch()
        kn = 'key name: %s'%curses.keyname(c)
        kv = 'key value: %d'%c
        stdscr.addstr(0,0,kn+' '*(32-len(kn))) # row 0
        stdscr.addstr(1,0,kv+' '*(32-len(kv))) # row 1

curses.wrapper(main) # do the setup and prevent messing up the terminal
