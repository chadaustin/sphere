#include <windows.h>

static HANDLE hOut;
static HANDLE hIn;

void InitStdOut(void)
{

 hOut = GetStdHandle(STD_OUTPUT_HANDLE);
 hIn  = GetStdHandle(STD_INPUT_HANDLE);
}

void GotoXY(int x, int y)
{
 COORD c;
 c.X = x;
 c.Y = y;
 SetConsoleCursorPosition(hOut, c);
}

void WriteXY(int x, int y, LPSTR S)
{
 DWORD result;

 GotoXY(x,y);
 WriteConsole(hOut, S, strlen(S), &result, NULL);
}

void BlankLine(int Y)
{
 DWORD NumWritten;
 COORD c;

 c.X = 0;
 c.Y = Y;

 FillConsoleOutputCharacter(hOut, ' ', 80, c, &NumWritten);
}

void ClrScr(WORD Attr)
{
 DWORD NumWritten;
 COORD c;
 int Space = 80*25;

 c.X = 0;
 c.Y = 0;

 FillConsoleOutputCharacter(hOut, ' ', Space, c, &NumWritten);
 FillConsoleOutputAttribute(hOut, Attr, Space, c, &NumWritten);

 GotoXY(0,0);
}

void Write(LPSTR S)
{
 DWORD NumWritten;
 WriteConsole(hOut, S, strlen(S), &NumWritten, NULL);
}

void WaitForKeypress(void)
{
 DWORD Result;
 INPUT_RECORD Buffer;

 do {
   ReadConsoleInput(hIn, &Buffer, 1, &Result);
 } while (Buffer.EventType != KEY_EVENT);
}
