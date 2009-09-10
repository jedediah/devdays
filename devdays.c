
/* Programmed in 2009 by Jedediah Smith
 * For benevolent use only.
 * 
 * Using Visual C 2005 or later, build with:
 *
 *   cl devdays.c /D _CONSOLE /link user32.lib /SUBSYSTEM:CONSOLE
 *
 * Run:
 *
 *   devdays.exe HHMM "DevDays will resume in %s"
 *
 * Hit ESC to quit or run out the clock.
 *
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


char* text[6] = {
  "A fatal exception 0E has occurred at 0137:BFFA21C9.  The current",
  "application will be terminated.",
  "",
  "*  Press any key to terminate the current application.",
  "*  Press CTRL+ALT+DEL again to restart your computer. You will",
  "   lose any unsaved information in all applications.",
};

void must(char* msg, int p) {
	DWORD code;
  if (!p) {
  	code = GetLastError();
    fprintf(stderr,"%s failed with code %i\n",msg,code);
    exit(-1);
  }
}

int main(int argc, char* argv[]) {
  time_t now, then;
  struct tm deadline;
  int minutes, seconds;
  char digits[7];
  char *format;
  char message[81];
  int length;

  int i;
  COORD coord;
  INPUT_RECORD ir;
  SMALL_RECT sr;
  CONSOLE_CURSOR_INFO cursor;
  WORD whiteOnBlue;
  HANDLE csb, input;

  if (argc != 3 || strlen(argv[1]) != 4) {
    fprintf(stderr,"YOU FAIL AT THE INVOCATION");
    return -2;
  }
    
  now = time(NULL);
  localtime_s(&deadline,&now);
  deadline.tm_hour = (argv[1][0]-'0')*10 + (argv[1][1]-'0');
  deadline.tm_min =  (argv[1][2]-'0')*10 + (argv[1][3]-'0');
  then = mktime(&deadline);

  if (then <= now) {
    fprintf(stderr,"Stop living in the past");
    return -2;
  }

  format = argv[2];
  length = strlen(format)+3;

  whiteOnBlue = BACKGROUND_BLUE|
                FOREGROUND_RED|
                FOREGROUND_GREEN|
                FOREGROUND_BLUE|
                FOREGROUND_INTENSITY;

  csb = GetStdHandle(STD_OUTPUT_HANDLE);
                                 
  must("CreateConsoleScreenBuffer",
       csb != INVALID_HANDLE_VALUE);
  sr.Top = 0; sr.Left = 0; sr. Right = 79; sr.Bottom = 24;
  must("SetConsoleWindowInfo",
       SetConsoleWindowInfo(csb,TRUE,&sr));
  coord.X = 80; coord.Y = 25;
	must("SetConsoleScreenBufferSize",
	     SetConsoleScreenBufferSize(csb,coord));
  must("SetConsoleActiveScreenBuffer",
       SetConsoleActiveScreenBuffer(csb));
  must("SetConsoleDisplayMode",
       SetConsoleDisplayMode(csb,CONSOLE_FULLSCREEN_MODE,&coord));
  input = GetStdHandle(STD_INPUT_HANDLE);
  must("GetStdHandle",
       input != INVALID_HANDLE_VALUE);
  must("SetConsoleMode",
       SetConsoleMode(input,ENABLE_EXTENDED_FLAGS));

  coord.X = coord.Y = 0;
  FillConsoleOutputCharacter(csb,' ',80*25,coord,NULL);
  FillConsoleOutputAttribute(csb,whiteOnBlue,80*25,coord,NULL);
  
  
  coord.X = 36; coord.Y = 8;
  FillConsoleOutputAttribute(csb,
                             BACKGROUND_RED|
                             BACKGROUND_GREEN|
                             BACKGROUND_BLUE|
                             FOREGROUND_BLUE,
                             9,
                             coord,
                             NULL);
  WriteConsoleOutputCharacter(csb," Windows ",9,coord,NULL);

  SetConsoleTextAttribute(csb,whiteOnBlue);

  for (i = 0; i < 6; i++) {
    coord.X = 8; coord.Y = 10+i;
    WriteConsoleOutputCharacter(csb,text[i],strlen(text[i]),coord,NULL);
  }

  cursor.dwSize = 100; cursor.bVisible = FALSE;
  must("SetConsoleCursorInfo",
       SetConsoleCursorInfo(csb,&cursor));
  ShowCursor(FALSE);
  SetCursor(NULL);

  coord.X = 40-length/2; coord.Y = 17;
  while (now < then) {
    now = time(NULL);
    minutes = (then-now)/60;
    seconds = (then-now)%60;
    _snprintf(digits,6,"%2i:%02i",minutes,seconds);
    _snprintf(message,80,format,digits);
    WriteConsoleOutputCharacter(csb,message,strlen(message),coord,NULL);

    for (;;) {
    	PeekConsoleInput(input,&ir,1,&i);
    	if (i == 0) break;
    	ReadConsoleInput(input,&ir,1,&i);
    	if (ir.EventType == KEY_EVENT) {
    		if (ir.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
    			goto bye;
    		}
    	}
  	}

    Sleep(500);
  }
  
  bye:
  SetConsoleDisplayMode(csb,CONSOLE_WINDOWED_MODE,&coord);
  cursor.dwSize = 20; cursor.bVisible = TRUE;
  must("SetConsoleCursorInfo",
       SetConsoleCursorInfo(csb,&cursor));
  ShowCursor(TRUE);
  
  return 0;
}
