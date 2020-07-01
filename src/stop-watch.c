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
typedef enum
{
  WATCH_PAUSED,
  WATCH_RUN
} watch_status_typedef;

typedef enum
{
  LARGE,
  SMALL,
  SMALLER,
  TINY
} size_mode_typedef;

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

struct timespec history_time[HISTORY_MAX];

int clock_x = CLOCK_X;
int clock_y = CLOCK_Y;
int history_x = HISTORY_X;
int history_y = HISTORY_Y;
int pause_x = 0;
int pause_y = 0;
int prompt_x = 0;
int prompt_y = 0;
int prev_cols = 0;
int prev_lines = 0;

size_mode_typedef height_mode = LARGE;
size_mode_typedef width_mode = LARGE;

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
void input_history(int x, int y)
{
  int i;
  int yl = y;
  int max = HISTORY_MAX;

  if (height_mode == LARGE)
  {
    mvprintw(yl, x, "== Previous values ==");
    mvprintw(yl + 1, x, "---------------------");
  }
  else
  {
    yl = y - 2;
    max = 5;
  }

  for (i = 0; i < max; i++)
  {
    mvprintw(yl + 2 + i, x + 2, "%2d. %04d:%02d:%03d", i + 1,
             history_time[i].tv_sec / 60, history_time[i].tv_sec % 60,
             history_time[i].tv_nsec / 1000000);
  }
}

void set_prompt(char *message)
{
  int i;
  int max_cols = COLS;

  if (message == NULL)
  {
    if (height_mode == TINY || height_mode == SMALLER)
      max_cols = COLS - 25;

    if (width_mode != TINY)
    {
      for (i = prompt_x; i < max_cols; i++)
        mvprintw(prompt_y, i, " ");
    }
    mvprintw(prompt_y, prompt_x, "");
  }
  else
  {
    if ((height_mode != TINY && height_mode != SMALLER) ||
        (width_mode != TINY && width_mode != SMALLER))
      mvprintw(prompt_y, prompt_x, "> %s", message);
  }
  refresh();
}

/***
 * Update the history table as well as
 * refreshing the screen values
 */
void refresh_history(struct timespec t)
{
  int i;
  for (i = HISTORY_MAX - 1; i > 0; --i)
  {
    history_time[i] = history_time[i - 1];
  }
  history_time[0] = t;
  input_history(history_x, history_y);
  set_prompt("Current time saved to history.");
  refresh();
}

/***
 * Print the clock
 */
void input_clock(struct timespec now)
{
  int shift_x = 22;
  int shift_y = 3;

  if ((width_mode == LARGE || width_mode == SMALL) && height_mode != TINY)
  {
    input_nb((now.tv_sec / 36000) % 10, clock_x - shift_x + 0,
             clock_y - shift_y);
    input_nb((now.tv_sec / 3600) % 10, clock_x - shift_x + 7,
             clock_y - shift_y);
    input_nb(10, clock_x - shift_x + 13, clock_y - shift_y);
    input_nb((((now.tv_sec / 60) % 60) / 10) % 10, clock_x - shift_x + 16,
             clock_y - shift_y);
    input_nb((now.tv_sec / 60) % 10, clock_x - shift_x + 23, clock_y - shift_y);
    input_nb(10, clock_x - shift_x + 29, clock_y - shift_y);
    input_nb(((now.tv_sec % 60) / 10) % 10, clock_x - shift_x + 32,
             clock_y - shift_y);
    input_nb((now.tv_sec) % 10, clock_x - shift_x + 39, clock_y - shift_y);
  }
  else
  {
    shift_x = 5;
    shift_y = 1;
    mvprintw(clock_y - shift_y, clock_x - shift_x, "%04d:%02d:%02d",
             (now.tv_sec / 3600), (now.tv_sec / 60), (now.tv_sec % 60));
  }

  set_prompt(NULL);
  refresh();
}

/***
 * List the commands
 */
void input_commands_list(int x, int y)
{
  int yl = y;

  if (height_mode == LARGE)
  {
    mvprintw(yl, x, "== Commands ====================");
    mvprintw(yl + 1, x, "--------------------------------");
  }
  else
  {
    yl = y - 2;
  }

  if (width_mode == LARGE)
  {
    mvprintw(yl + 2, x, "  q:       leave             ");
    mvprintw(yl + 3, x, "  <SPACE>: start/pause       ");
    mvprintw(yl + 4, x, "  r:       reset to 0        ");
    mvprintw(yl + 5, x, "  l:       lap               ");
    mvprintw(yl + 6, x, "  s:       save history to file ");
  }
  else
  {
    mvprintw(yl + 2, x, "  q:       leave");
    mvprintw(yl + 3, x, "  <SPACE>: pause");
    mvprintw(yl + 4, x, "  r:       reset");
    mvprintw(yl + 5, x, "  l:       lap  ");
    mvprintw(yl + 6, x, "  s:       save ");
  }
  set_prompt(NULL);
}

/***
 * Print frame:
 *
 * Note that this expects the terminal to be at least
 * 35 x 4
 */
void get_height_mode(int *bottom_y)
{
  if (LINES < 8)
  {
    *bottom_y = 2;
    height_mode = TINY;
  }
  else if (LINES < 14)
  {
    *bottom_y = 2;
    height_mode = SMALLER;
  }
  else if (LINES < 21)
  {
    *bottom_y = 8;
    height_mode = SMALL;
  }
  else
  {
    *bottom_y = 15;
    height_mode = LARGE;
  }
}

void get_width_mode(void)
{
  if (COLS < 35)
    width_mode = TINY;
  else if (COLS < 47)
    width_mode = SMALLER;
  else if (COLS < 61)
    width_mode = SMALL;
  else
    width_mode = LARGE;
}

void draw_horizontal_lines(int bottom_y)
{
  int i;

  for (i = 0; i < COLS; i++)
  {
    mvprintw(0, i, "=");
    mvprintw(LINES - bottom_y, i, "=");
    if (height_mode != TINY)
      mvprintw(LINES - 2, i, "=");
  }
}

void print_header(void)
{
  if (width_mode == SMALL || width_mode == LARGE)
  {
    mvprintw(0, 2, "  ##  START-STOP WATCH  ##  ");
    mvprintw(0, COLS - 8, " v0.1 ");
  }
}

void print_history(int bottom_y)
{
  int i;
  for (i = 3; i < bottom_y; i++)
  {
    mvprintw(LINES - i, COLS - 26, "|");
  }
  history_x = COLS - 23;
  history_y = LINES - bottom_y + 1;
  input_history(history_x, history_y);
}

void input_frame(void)
{
  int i;
  int mid_x, mid_y;
  int bottom_y;

  clear();

  prev_cols = COLS;
  prev_lines = LINES;

  get_height_mode(&bottom_y);
  get_width_mode();

  /* horizontal lines */
  draw_horizontal_lines(bottom_y);

  /* Top line */
  print_header();

  /* Get Clock position */
  mid_x = COLS / 2;
  mid_y = (LINES - bottom_y - 1) / 2 + 2;
  clock_x = mid_x; // - 22;
  clock_y = mid_y; // - 3;

  /* Get Pause Position */
  pause_x = mid_x; //-7;
  pause_y = mid_y; //-1;

  /* Get Prompt position */
  prompt_x = 0;
  prompt_y = LINES - 1;

  /* Commands */
  if ((height_mode != TINY && height_mode != SMALLER) &&
      (width_mode != TINY && width_mode != SMALLER))
    input_commands_list(2, LINES - bottom_y + 1);

  /* History */
  print_history(bottom_y);
}

/***
 * Mark the pause
 */
void mark_pause(watch_status_typedef s)
{
  int shift_x = 7;
  int shift_y = 1;

  if (s == WATCH_PAUSED)
    mvprintw(pause_y - shift_y, pause_x - shift_x, " === PAUSED === ");
  else
    mvprintw(pause_y - shift_y, pause_x - shift_x, "                ");
}

/***
 * Save the history to a file
 */
void save_history(void)
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
              history_time[i].tv_sec / 60, history_time[i].tv_sec % 60,
              history_time[i].tv_nsec / 1000000);
    }
    fclose(f);
  }
}

void initialise_ncurses(void)
{
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
}

bool ncurses_ui_stop(struct timespec *start, struct timespec *now,
                     watch_status_typedef *state)
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
      if (*state == WATCH_RUN)
        refresh_history(*now);

      clock_gettime(CLOCK_MONOTONIC, start);

      if (*state == WATCH_PAUSED)
      {
        now->tv_sec = 0;
        now->tv_nsec = 0;
        input_clock(*now);
      }
    }
    else if ('l' == ch || 'L' == ch)
    {
      refresh_history(*now);
    }
    else if ('s' == ch || 'S' == ch)
    {
      save_history();
      set_prompt("Data saved to file.");
    }
    else if (' ' == ch)
    {
      if (*state == WATCH_RUN)
      {
        refresh_history(*now);
        // attron (A_STANDOUT);
        *state = WATCH_PAUSED;
        set_prompt("Timer paused.                 ");
      }
      else
      {
        // attroff (A_STANDOUT);
        *state = WATCH_RUN;
      }
      mark_pause(*state);
      refresh();
    }
  }

  return false;
}

void ncurses_main(struct timespec *start)
{
  struct timespec now;

  /* Initialise some structures */
  struct timespec delay;
  delay.tv_sec = 0;
  delay.tv_nsec = 1000000;

  /* Watch presentation */
  input_frame();

  /* Initialise some values */
  watch_status_typedef state = WATCH_RUN;
  struct timespec prev = *start;

  /* Loop program */
  for (;;)
  {
    if (is_term_resized(prev_lines, prev_cols) == TRUE)
    {
      input_frame();
    }

    if (state == WATCH_RUN)
    {
      /* Calculate the current time since start */
      subtract_time(&now, start);

      /* Update clock on each second */
      if (now.tv_sec != prev.tv_sec)
      {
        input_clock(now);
        prev = now;
      }
    }
    else
    {
      /* Update the start value */
      subtract_time(start, &now);
    }

    /* Interaction */
    if (true == ncurses_ui_stop(start, &now, &state))
    {
      break;
    }

    clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, NULL);
  }
}

//-- Main function ------------------

int main(int argc, char *argv[])
{
  struct timespec start;

  /* Start the timer */
  clock_gettime(CLOCK_MONOTONIC, &start);

  /* Initialise ncurses */
  initialise_ncurses();

  /* Run the ncurses */
  ncurses_main(&start);

  /* Stops the ncurses */
  endwin();

  /* Ends the program */
  return 0;
}
