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

#define HISTORY_X   70
#define HISTORY_Y   1
#define HISTORY_MAX 10

#define CLOCK_X 10
#define CLOCK_Y 10

//-- Typedef ------------------------
typedef enum {
  WATCH_PAUSED,
  WATCH_RUN
} watch_status_typedef;

//-- Private variables --------------

char *numbers_image [] = 
{
  " #### ", 
  "##  ##", 
  "##  ##", 
  "##  ##", 
  " #### ",

  "  ##  ", 
  " ###  ",
  "  ##  ",
  "  ##  ",
  " #### ",

  " #### ", 
  "##  ##",  
  "   ## ",  
  "  ##  ",  
  "######", 

  " #### ",  
  "##  ##", 
  "   ###", 
  "##  ##", 
  " #### ",

  "   ## ", 
  "  ### ",
  " # ## ",
  "######", 
  "   ## ",

  "######", 
  "##    ", 
  "##### ", 
  "    ##", 
  "##### ",

  " #### ", 
  "##    ", 
  "##### ", 
  "##  ##", 
  " #### ",

  "######",  
  "   ## ",  
  "  ##  ",  
  " ##   ",  
  "##    ",

  " #### ",  
  "##  ##", 
  " #### ",
  "##  ##", 
  " #### ",

  " #### ",  
  "##  ##",
  " #####",
  "    ##",
  " #### ",

  "   ",
  " # ",
  "   ",
  " # ",
  "   "
};

struct timespec history_time[HISTORY_MAX];

//-- Private functions --------------

/***
  * Get the current time and calculate the time difference with the 'former' time. 
  * Input:  former
  * Output: later
  */

void subtract_time (struct timespec * __restrict later, struct timespec * __restrict former)
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

  for (i=0;i<5;++i)
  {
    mvprintw (y+i,x,numbers_image[n*5+i]);
  }
}

/***
  * Print history
  */
void input_history(int x, int y)
{
  int i;

  mvprintw (y,   x, "== Previous values ==");
  mvprintw (y+1, x, "---------------------");

  for (i=0;i<HISTORY_MAX;i++)
  {
    mvprintw (y+2+i, x+2, "%2d. %04d:%02d:%03d", i+1, history_time[i].tv_sec / 60 , history_time[i].tv_sec % 60 , history_time[i].tv_nsec / 1000000);
  }
}

void set_prompt(void)
{
  mvprintw (0,0,"");
  refresh();
}

/***
  * Update the history table as well as
  * refreshing the screen values
  */
void refresh_history(struct timespec t)
{
  int i;
  for (i=HISTORY_MAX-1;i>0;--i)
  {
    //printf("%d",i);
    history_time[i] = history_time[i-1];
  }
  history_time[0] = t;
  //printf("\t%d",history_time[0].tv_sec);
  input_history(HISTORY_X,HISTORY_Y);
  set_prompt();
  refresh();
}

/***
  * Print the clock
  */
void input_clock(int x,int y, struct timespec now)
{
  input_nb((now.tv_sec / 36000)%10      ,x+0 ,y);
  input_nb((now.tv_sec / 3600)%10       ,x+7,y);
  input_nb(10                           ,x+13,y);
  input_nb((((now.tv_sec/60)%60)/10)%10 ,x+16,y);
  input_nb((now.tv_sec / 60)%10         ,x+23,y);
  input_nb(10                           ,x+29,y);
  input_nb(((now.tv_sec%60)/ 10)%10     ,x+32,y);
  input_nb((now.tv_sec )%10             ,x+39,y);

  set_prompt();
  refresh();
}

/***
  * List the commands
  */
void input_commands_list(int x, int y)
{
  mvprintw(y,	x," <ESC>/q: leave  ");
  mvprintw(y+1,	x," <SPACE>: start/pause");
  mvprintw(y+2, x," r:       reset to 0");
  mvprintw(y+3, x," l:       lap");
  mvprintw(y+4, x," s:       save history to file");
}

/***
  * Mark the pause
  */
void mark_pause(int x, int y, watch_status_typedef s)
{
  if (s == WATCH_PAUSED)
    mvprintw (y, x, " === PAUSED === ");
  else
    mvprintw (y, x, "                ");
}

/***
  * Save the history to a file
  */
void save_history(void)
{
  FILE *f = NULL;
  int i;

  f = fopen("history.dat","wt");
  if (f)
  {
    fprintf(f, "# nb\tmin\ts\tms\n");
    for (i=0;i<HISTORY_MAX;i++)
    {
      fprintf (f, "%2d\t%04d\t%02d\t%03d\n", i+1, history_time[i].tv_sec / 60 , history_time[i].tv_sec % 60 , history_time[i].tv_nsec / 1000000);
    }
    fclose(f);
  }
}

//-- Main function ------------------

int main (int argc, char *argv[])
{
  int ch;
  unsigned int countdown;
  struct timespec start;
  struct timespec now;

  /* Start the timer */
  clock_gettime(CLOCK_MONOTONIC, &start);

  /* Initialise ncurses */
  initscr();
  cbreak();
  noecho();
  nodelay (stdscr, TRUE);

  /* Initialise some structures */
  struct timespec delay;
  delay.tv_sec = 0;
  delay.tv_nsec = 1000000;

  /* Watch presentation */
  mvprintw(0,1, "  ##  START-STOP WATCH  ##  ");
  input_commands_list(1,40);

  input_history(HISTORY_X,HISTORY_Y);

  /* Initialise some values */
  watch_status_typedef state = WATCH_RUN;
  struct timespec prev = start;

  /* Loop program */
  for (;;)
  {
    if (state == WATCH_RUN)
    {
      /* Calculate the current time since start */
      subtract_time (&now, &start);

      /* Update clock on each second */
      if (now.tv_sec != prev.tv_sec)
      {
        input_clock(CLOCK_X,CLOCK_Y,now);
        prev = now;
      }
    }
    else
    {
      /* Update the start value */
      subtract_time (&start, &now);
    }

    /* Interaction */
    if (ERR != (ch = getch()))
    {
      /* Quit the application */
      if (27 == ch || 'q' == ch || 'Q' == ch)
      {
        break;
      }
      /* Reset the values */
      else if ('r' == ch || 'R' == ch)
      {
        if (state == WATCH_RUN)
          refresh_history(now);

        clock_gettime(CLOCK_MONOTONIC, &start);

        if (state == WATCH_PAUSED)
        {
          now.tv_sec = 0;
          now.tv_nsec = 0;
          input_clock(CLOCK_X,CLOCK_Y,now);
        }
      }
      else if ('l' == ch || 'L' == ch)
      {
	refresh_history(now);
      }
      else if ('s' == ch || 'S' == ch)
      {
	save_history();
        set_prompt();
	printf("Data saved.");
      }
      else if (' ' == ch)
      {
        if (state == WATCH_RUN)
        {
          refresh_history(now);
          //attron (A_STANDOUT);
          state = WATCH_PAUSED;
          set_prompt();
        }
        else
        {
          //attroff (A_STANDOUT);
          state = WATCH_RUN;
        }
	mark_pause(23,12,state);
        refresh();
      }
    }
    clock_nanosleep (CLOCK_MONOTONIC, 0, &delay, NULL);
  }

  endwin();
  return 0;
}
