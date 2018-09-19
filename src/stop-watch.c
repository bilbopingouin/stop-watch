/* https://www.linuxquestions.org/questions/programming-9/timer-stopwatch-program-in-c-and-ncurses-4175500907/ */

#include <stdio.h>
#include <time.h>

#include <ncurses.h>

//char number_image [] = {1,2,3};
//char *number_image [] = {"1","2","3"};
/* not working
   char **number_image [] = { 
   {"1","2","3"},
   {"4","5","6"},
   };*/

#define HISTORY_X   70
#define HISTORY_Y   1
#define HISTORY_MAX 10

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

/*inline*/ void subtract_time (struct timespec * __restrict later, struct timespec * __restrict former)
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



void input_nb(int n, int x, int y)
{
  int i;

  for (i=0;i<5;++i)
  {
    mvprintw (y+i,x,numbers_image[n*5+i]);
  }
  /*
     mvprintw(y+0,x," #### ");
     mvprintw(y+1,x,"##  ##");     
     mvprintw(y+2,x,"##  ##");     
     mvprintw(y+3,x,"##  ##");     
     mvprintw(y+4,x," #### ");     
   */
}

void input_history(int x, int y)
{
  int i;

  mvprintw (y,   x, "== Previous values ==");
  mvprintw (y+1, x, "---------------------");

  for (i=0;i<HISTORY_MAX;i++)
  {
    /*printf("%d: %d\n",i,history_time[i].tv_sec);*/
    mvprintw (y+2+i, x+2, "%2d. %04d:%02d:%03d", i+1, history_time[i].tv_sec / 60 , history_time[i].tv_sec % 60 , history_time[i].tv_nsec / 1000000);
  }
}

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
}

int main (int argc, char *argv[])
{
  int ch;
  unsigned int countdown;
  struct timespec start;
  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC, &start);
  if (2 == argc)
  {
    sscanf (argv[1], "%u", &countdown);
    start.tv_sec += countdown * 60;
  }
  else if (1 != argc)
  {
    printf ("Usage: %s <minute_countdown>\n", argv[0]);
    return 1;
  }

  initscr();
  cbreak();
  noecho();
  nodelay (stdscr, TRUE);

  struct timespec delay;
  delay.tv_sec = 0;
  delay.tv_nsec = 1000000;


  mvprintw(0,1, "  ##  START-STOP WATCH  ##  ");
  mvprintw(40,1,"  <ESC>/q: leave  | <SPACE>: start/pause | R: reset to 0");

  input_history(HISTORY_X,HISTORY_Y);

  int paused = 0;
  struct timespec prev = start;
  for (;;)
  {
    if (0 == paused)
    {
      subtract_time (&now, &start);
      if (now.tv_sec != prev.tv_sec)
      {
        //mvprintw (1, 2, "%+04d:%+03d", now.tv_sec / 60 , now.tv_sec % 60);
        //input_nb(0,10,10);

        input_nb((now.tv_sec / 36000)%10      ,10,10);
        input_nb((now.tv_sec / 3600)%10       ,17,10);
        input_nb(10                           ,23,10);
        input_nb((((now.tv_sec/60)%60)/10)%10 ,26,10);
        input_nb((now.tv_sec / 60)%10         ,33,10);
        input_nb(10                           ,39,10);
        input_nb(((now.tv_sec%60)/ 10)%10     ,42,10);
        input_nb((now.tv_sec )%10             ,49,10);

        refresh();
        prev = now;
      }
    }
    else
    {
      subtract_time (&start, &now);
    }

    if (ERR != (ch = getch()))
    {
      if (27 == ch || 'q' == ch || 'Q' == ch)
      {
        break;
      }
      else if ('r' == ch || 'R' == ch)
      {
        refresh_history(now);
        clock_gettime(CLOCK_MONOTONIC, &start);
      }
      else
      {
        if (0 == paused)
        {
          refresh_history(now);
          attron (A_STANDOUT);
          paused = 1;
        }
        else
        {
          attroff (A_STANDOUT);
          paused = 0;
        }
        //mvprintw (1, 2, "%+04d:%+03d", now.tv_sec / 60 , now.tv_sec % 60);
        //refresh_history(now);
        refresh();
      }
    }
    clock_nanosleep (CLOCK_MONOTONIC, 0, &delay, NULL);
  }

  endwin();
  return 0;
}
