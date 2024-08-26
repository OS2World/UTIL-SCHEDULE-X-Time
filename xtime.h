/*--------------------
   XTIME.H header file
  --------------------*/

#define _PARM(a) &argv[a][1]
#define NO_ERROR 0
#define SEMANTIC_ERROR 1
#define PROGRAM_NOT_FOUND 2
#define NO_TIME_GIVEN 3

#define ID_TIMER 1

typedef struct 
  {
  unsigned hours : 5;
  unsigned mins  : 6;
  unsigned secs  : 5;
  }
TIMEINFO;

typedef struct _INITDATA
  {
  COLOR    background;
  COLOR    text_color;
  SWP      wp;
  char     on_off;
  TIMEINFO start_time;
  } 
INITDATA;
