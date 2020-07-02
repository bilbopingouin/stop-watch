/*
 *  This code was derived from an example found on the following forum:
 *  https://www.linuxquestions.org/questions/programming-9/timer-stopwatch-program-in-c-and-ncurses-4175500907/
 *  by the user: metashima
 */

//-- Includes -----------------------

#include <stdio.h>
#include <time.h>

#include <ncurses.h>

//-- Defines ------------------------

#define HISTORY_X 70
#define HISTORY_Y 1
#define HISTORY_MAX 10

#define CLOCK_X 10
#define CLOCK_Y 10

//-- Typedef ------------------------
enum watch_status
{
  WATCH_PAUSED,
  WATCH_RUN
};

enum size_mode
{
  LARGE,
  SMALL,
  SMALLER,
  TINY
};

//-- Private variables --------------

char *numbers_image[] = {" #### ", "##  ##", "##  ##", "##  ##", " #### ",

                         "  ##  ", " ###  ", "  ##  ", "  ##  ", " #### ",

                         " #### ", "##  ##", "   ## ", "  ##  ", "######",

                         " #### ", "##  ##", "   ###", "##  ##", " #### ",

                         "   ## ", "  ### ", " # ## ", "######", "   ## ",

                         "######", "##    ", "##### ", "    ##", "##### ",

                         " #### ", "##    ", "##### ", "##  ##", " #### ",

                         "######", "   ## ", "  ##  ", " ##   ", "##    ",

                         " #### ", "##  ##", " #### ", "##  ##", " #### ",

                         " #### ", "##  ##", " #####", "    ##", " #### ",

                         "   ",    " # ",    "   ",    " # ",    "   "};

struct watch_parameters 
{
  int bottom_y;

  struct {
    int x, y;
    struct timespec time[HISTORY_MAX];
  } history;

  struct 
  {
    int x, y;
  } clock;

  struct 
  {
    int x, y;
  } pause;

  struct 
  {
    int x, y;
  } prompt;

  struct
  {
    int x, y;
  } commands;

  struct
  {
    int cols;
    int lines;
  } prev;

  struct
  {
    enum size_mode height;
    enum size_mode width;
  } mode;

  struct 
  {
    struct timespec start;
    struct timespec now;
    struct timespec delay;
  } times;

  enum watch_status state;
};

//-- Private functions --------------

/***
 * Get the current time and calculate the time difference with the 'former'
 * time. Input:  former Output: later
 */

void subtract_time(struct timespec *__restrict later,
                   struct timespec *__restrict former)
{
  clock_gettime(CLOCK_MONOTONIC, later);

  later->tv_sec -= former->tv_sec;
  if (later->tv_nsec < former->tv_nsec)
  {
    later->tv_sec--;
    later->tv_nsec = 1000000000 + later->tv_nsec - former->tv_nsec;
  }
  else
  {
    later->tv_nsec -= former->tv_nsec;
  }
}

/***
 * Print one digit for the clock
 */

void input_nb(int n, int x, int y)
{
  int i;

  for (i = 0; i < 5; ++i)
  {
    mvprintw(y + i, x, numbers_image[n * 5 + i]);
  }
}

/***
 * Print history
 */
void input_history(struct watch_parameters* watch)
{
  int i;
  int yl = watch->history.y;
  int max = HISTORY_MAX;

  if (watch->mode.height == LARGE)
  {
    mvprintw(yl, watch->history.x, "== Previous values ==");
    mvprintw(yl + 1, watch->history.x, "---------------------");
  }
  else
  {
    yl = watch->history.y - 2;
    max = 5;
  }

  for (i = 0; i < max; i++)
  {
    mvprintw(yl + 2 + i, watch->history.x + 2, "%2d. %04d:%02d:%03d", i + 1,
             watch->history.time[i].tv_sec / 60, watch->history.time[i].tv_sec % 60,
             watch->history.time[i].tv_nsec / 1000000);
  }
}

void set_prompt(struct watch_parameters* watch, char *message)
{
  int i;
  int max_cols = COLS;

  if (message == NULL)
  {
    if (watch->mode.height == TINY || watch->mode.height == SMALLER)
      max_cols = COLS - 25;

    if (watch->mode.width != TINY)
    {
      for (i = watch->prompt.x; i < max_cols; i++)
        mvprintw(watch->prompt.y, i, " ");
    }
    mvprintw(watch->prompt.y, watch->prompt.x, "");
  }
  else
  {
    if ((watch->mode.height != TINY && watch->mode.height != SMALLER) ||
        (watch->mode.width != TINY && watch->mode.width != SMALLER))
      mvprintw(watch->prompt.y, watch->prompt.x, "> %s", message);
  }
  refresh();
}

/***
 * Update the history table as well as
 * refreshing the screen values
 */
void refresh_history(struct watch_parameters* watch)
{
  int i;
  for (i = HISTORY_MAX - 1; i > 0; --i)
  {
    watch->history.time[i] = watch->history.time[i - 1];
  }
  watch->history.time[0] = watch->times.now;
  input_history(watch);
  set_prompt(watch, "Current time saved to history.");
  refresh();
}

/***
 * Print the clock
 */
void input_clock(struct watch_parameters* watch)
{
  int shift_x = 22;
  int shift_y = 3;

  if ((watch->mode.width == LARGE || watch->mode.width == SMALL) && watch->mode.height != TINY)
  {
    input_nb((watch->times.now.tv_sec / 36000) % 10, watch->clock.x - shift_x + 0,
             watch->clock.y - shift_y);
    input_nb((watch->times.now.tv_sec / 3600) % 10, watch->clock.x - shift_x + 7,
             watch->clock.y - shift_y);
    input_nb(10, watch->clock.x - shift_x + 13, watch->clock.y - shift_y);
    input_nb((((watch->times.now.tv_sec / 60) % 60) / 10) % 10, watch->clock.x - shift_x + 16,
             watch->clock.y - shift_y);
    input_nb((watch->times.now.tv_sec / 60) % 10, watch->clock.x - shift_x + 23, watch->clock.y - shift_y);
    input_nb(10, watch->clock.x - shift_x + 29, watch->clock.y - shift_y);
    input_nb(((watch->times.now.tv_sec % 60) / 10) % 10, watch->clock.x - shift_x + 32,
             watch->clock.y - shift_y);
    input_nb((watch->times.now.tv_sec) % 10, watch->clock.x - shift_x + 39, watch->clock.y - shift_y);
  }
  else
  {
    shift_x = 5;
    shift_y = 1;
    mvprintw(watch->clock.y - shift_y, watch->clock.x - shift_x, "%04d:%02d:%02d",
             (watch->times.now.tv_sec / 3600), (watch->times.now.tv_sec / 60), (watch->times.now.tv_sec % 60));
  }

  set_prompt(watch, NULL);
  refresh();
}

/***
 * List the commands
 */
void input_commands_list(struct watch_parameters* watch)
{
  int yl = watch->commands.y;

  if (watch->mode.height == LARGE)
  {
    mvprintw(yl, watch->commands.x, "== Commands ====================");
    mvprintw(yl + 1, watch->commands.x, "--------------------------------");
  }
  else
  {
    yl = watch->commands.y - 2;
  }

  if (watch->mode.width == LARGE)
  {
    mvprintw(yl + 2, watch->commands.x, "  q:       leave             ");
    mvprintw(yl + 3, watch->commands.x, "  <SPACE>: start/pause       ");
    mvprintw(yl + 4, watch->commands.x, "  r:       reset to 0        ");
    mvprintw(yl + 5, watch->commands.x, "  l:       lap               ");
    mvprintw(yl + 6, watch->commands.x, "  s:       save history to file ");
  }
  else
  {
    mvprintw(yl + 2, watch->commands.x, "  q:       leave");
    mvprintw(yl + 3, watch->commands.x, "  <SPACE>: pause");
    mvprintw(yl + 4, watch->commands.x, "  r:       reset");
    mvprintw(yl + 5, watch->commands.x, "  l:       lap  ");
    mvprintw(yl + 6, watch->commands.x, "  s:       save ");
  }
  set_prompt(watch, NULL);
}

/***
 * Print frame:
 *
 * Note that this expects the terminal to be at least
 * 35 x 4
 */
void get_height_mode(struct watch_parameters* watch)
{
  if (LINES < 8)
  {
    watch->bottom_y = 2;
    watch->mode.height = TINY;
  }
  else if (LINES < 14)
  {
    watch->bottom_y = 2;
    watch->mode.height = SMALLER;
  }
  else if (LINES < 21)
  {
    watch->bottom_y = 8;
    watch->mode.height = SMALL;
  }
  else
  {
    watch->bottom_y = 15;
    watch->mode.height = LARGE;
  }
}

void get_width_mode(struct watch_parameters* watch)
{
  if (COLS < 35)
    watch->mode.width = TINY;
  else if (COLS < 47)
    watch->mode.width = SMALLER;
  else if (COLS < 61)
    watch->mode.width = SMALL;
  else
    watch->mode.width = LARGE;
}

void draw_horizontal_lines(struct watch_parameters* watch)
{
  int i;

  for (i = 0; i < COLS; i++)
  {
    mvprintw(0, i, "=");
    mvprintw(LINES - watch->bottom_y, i, "=");
    if (watch->mode.height != TINY)
      mvprintw(LINES - 2, i, "=");
  }
}

void print_header(struct watch_parameters* watch)
{
  if (watch->mode.width == SMALL || watch->mode.width == LARGE)
  {
    mvprintw(0, 2, "  ##  START-STOP WATCH  ##  ");
    mvprintw(0, COLS - 8, " v0.1 ");
  }
}

void print_history(struct watch_parameters* watch)
{
  int i;
  for (i = 3; i < watch->bottom_y; i++)
  {
    mvprintw(LINES - i, COLS - 26, "|");
  }
  watch->history.x = COLS - 23;
  watch->history.y = LINES - watch->bottom_y + 1;
  input_history(watch);
}

void input_frame(struct watch_parameters* watch)
{
  int mid_x, mid_y;

  clear();

  watch->prev.cols = COLS;
  watch->prev.lines = LINES;

  get_height_mode(watch);
  get_width_mode(watch);

  /* horizontal lines */
  draw_horizontal_lines(watch);

  /* Top line */
  print_header(watch);

  /* Get Clock position */
  mid_x = COLS / 2;
  mid_y = (LINES - watch->bottom_y - 1) / 2 + 2;
  watch->clock.x = mid_x; // - 22;
  watch->clock.y = mid_y; // - 3;

  /* Get Pause Position */
  watch->pause.x = mid_x; //-7;
  watch->pause.y = mid_y; //-1;

  /* Get Prompt position */
  watch->prompt.x = 0;
  watch->prompt.y = LINES - 1;

  /* Commands */
  if ((watch->mode.height != TINY && watch->mode.height != SMALLER) &&
      (watch->mode.width != TINY && watch->mode.width != SMALLER))
  {
    watch->commands.y = LINES - watch->bottom_y + 1;
    input_commands_list(watch);
  }

  /* History */
  print_history(watch);
}

/***
 * Mark the pause
 */
void mark_pause(struct watch_parameters* watch)
{
  int shift_x = 7;
  int shift_y = 1;

  if (watch->state == WATCH_PAUSED)
    mvprintw(watch->pause.y - shift_y, watch->pause.x - shift_x, " === PAUSED === ");
  else
    mvprintw(watch->pause.y - shift_y, watch->pause.x - shift_x, "                ");
}

/***
 * Save the history to a file
 */
void save_history(struct watch_parameters* watch)
{
  FILE *f = NULL;
  int i;

  f = fopen("history.dat", "wt");
  if (f)
  {
    fprintf(f, "# nb\tmin\ts\tms\n");
    for (i = 0; i < HISTORY_MAX; i++)
    {
      fprintf(f, "%2d\t%04ld\t%02ld\t%03ld\n", i + 1,
              watch->history.time[i].tv_sec / 60, watch->history.time[i].tv_sec % 60,
              watch->history.time[i].tv_nsec / 1000000);
    }
    fclose(f);
  }
}

void initialise_ncurses(struct watch_parameters* watch)
{
  int cnt;

  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);

  watch->clock.x = CLOCK_X;
  watch->clock.y = CLOCK_Y;
  watch->history.x = HISTORY_X;
  watch->history.y = HISTORY_Y;
  watch->pause.x = 0;
  watch->pause.y = 0;
  watch->prompt.x = 0;
  watch->prompt.y = 0;
  watch->prev.cols = 0;
  watch->prev.lines = 0;
  watch->mode.height = LARGE;
  watch->mode.width = LARGE;
  watch->state = WATCH_RUN;
  watch->commands.x = 2;

  for (cnt=0 ; cnt<HISTORY_MAX ; ++cnt)
  {
    watch->history.time[cnt].tv_nsec = 0;
    watch->history.time[cnt].tv_sec = 0;
  }
}

bool ncurses_ui_stop(struct watch_parameters* watch)
{
  int ch;
  if (ERR != (ch = getch()))
  {
    /* Quit the application */
    if (/*27 == ch ||*/ 'q' == ch || 'Q' == ch)
    {
      return true;
    }
    /* Reset the values */
    else if ('r' == ch || 'R' == ch)
    {
      if (watch->state == WATCH_RUN)
        refresh_history(watch);

      clock_gettime(CLOCK_MONOTONIC, &(watch->times.start));

      if (watch->state == WATCH_PAUSED)
      {
        watch->times.now.tv_sec = 0;
        watch->times.now.tv_nsec = 0;
        input_clock(watch);
      }
    }
    else if ('l' == ch || 'L' == ch)
    {
      refresh_history(watch);
    }
    else if ('s' == ch || 'S' == ch)
    {
      save_history(watch);
      set_prompt(watch, "Data saved to file.");
    }
    else if (' ' == ch)
    {
      if (watch->state == WATCH_RUN)
      {
        refresh_history(watch);
        // attron (A_STANDOUT);
        watch->state = WATCH_PAUSED;
        set_prompt(watch, "Timer paused.                 ");
      }
      else
      {
        // attroff (A_STANDOUT);
        watch->state = WATCH_RUN;
      }
      mark_pause(watch);
      refresh();
    }
  }

  return false;
}

void ncurses_main(struct watch_parameters* watch)
{
  /* Initialise some structures */
  watch->times.delay.tv_sec = 0;
  watch->times.delay.tv_nsec = 1000000;

  /* Watch presentation */
  input_frame(watch);

  /* Initialise some values */
  watch->state = WATCH_RUN;
  struct timespec prev = watch->times.start;

  /* Loop program */
  for (;;)
  {
    if (is_term_resized(watch->prev.lines, watch->prev.cols) == TRUE)
    {
      input_frame(watch);
    }

    if (watch->state == WATCH_RUN)
    {
      /* Calculate the current time since start */
      subtract_time(&(watch->times.now), &(watch->times.start));

      /* Update clock on each second */
      if (watch->times.now.tv_sec != prev.tv_sec)
      {
        input_clock(watch);
        prev = watch->times.now;
      }
    }
    else
    {
      /* Update the start value */
      subtract_time(&(watch->times.start), &(watch->times.now));
    }

    /* Interaction */
    if (true == ncurses_ui_stop(watch))
    {
      break;
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &(watch->times.delay), NULL);
  }
}

//-- Main function ------------------

int main(int argc, char *argv[])
{
  struct watch_parameters watch;

  /* Start the timer */
  clock_gettime(CLOCK_MONOTONIC, &(watch.times.start));

  /* Initialise ncurses */
  initialise_ncurses(&watch);

  /* Run the ncurses */
  ncurses_main(&watch);

  /* Stops the ncurses */
  endwin();

  /* Ends the program */
  return 0;
}
