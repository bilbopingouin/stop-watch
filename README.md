# stop-watch
A simple ncurse-based stop-watch

# Install

Get the code

    git clone https://github.com/bilbopingouin/stop-watch.git

Within the directory, compile the code

    make

Execute the code

    bin/start-watch

Et voilà.

# Commands

There are a few commands available within the application.

- `q`       quit the application
- `<space>` pause/restart the timer
- `r`       reset the timer (to 0)
- `l`       lap: save the current time to history
- `s`       save history to file (`./history.dat`)

Note that using `l`, `r` and `<space>` (when pausing) make a new entry in the history. 

# Troubleshoot
    
### Compilation fail

Note that it may fail if you don't have the ncurses libraries installed. Try installing the ncurses libraries

    sudo apt-get install build-essential ncurses-base ncurses-bin ncurses-dev

and recompile.

### File saving

Make sure that you have the right to write in the current directory:

    touch a
    
Make sure that the file isn't opened:

    lsof | grep history.dat


