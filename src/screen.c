#include <stdio.h>
#include <stdint.h>
#include "screen.h"

// CSI is the "Control Sequence Introducer" (Escape left-square-bracket)
// The terminal recognizes this as the start of a cursor control sequence.
// See this article for more information:
//   https://en.wikipedia.org/wiki/ANSI_escape_code
#define CSI "\e["

// Print a string with no newline.
static void putstr(const char *s)
{
    while(*s)
        putchar(*s++);
}

// Print an integer.
static void putint(int x) {
        char buf[20];
        int off;
        int sign = 1;
        if (x < 0) {
                sign = -1;
                x = -x;
        }
        for(off=0; off < 18; off++) {
                buf[off] = (x % 10) + '0';
                x = x / 10;
                if (x == 0)
                        break;
        }
        if (sign == -1) {
                buf[++off] = '-';
        }
        for(     ; off >= 0; off--)
                putchar(buf[off]);
}

// Put the cursor in the upper left.
void home()
{
  putstr(CSI "1;1H");
}

// Clear the screen.
void clear()
{
  home();
  fgbg(0xf0);
  int x;
  for(x=0; x<4000; x++)
    putchar(' ');
}

// Set the cursor position to (x,y) [zero-based indexing]
void setpos(int x, int y)
{
  putstr(CSI); putint(y+1); putstr(";"); putint(x+1); putstr("H");
}

// Make the cursor invisible.
void cursor_off()
{
  putstr(CSI "?25l");
}

// Make the cursor visible.
void cursor_on()
{
  putstr(CSI "?25h");
}

// Set the foreground color to rgb.
// Each component must have the value 0 ... 5.
void fg(int r, int g, int b)
{
  putstr(CSI "38;5;"); putint(16 + 36 * r + 6*g + b);
}

// Set the foreground color to rgb.
// Each component must have the value 0 ... 5.
void bg(int r, int g, int b)
{
  putstr(CSI "48;5;"); putint(16 + 36 * r + 6*g + b);
}

// Set the foreground and background to the byte specified by n.
// The most significant nibble is the foreground.
// The least significant nibble is the background.
// The bits in each nibble specifie:
//  IBGR
//  I    High intensity
//   B   Blue
//    G  Green
//     R Red
// 
void fgbg(int n)
{
  int f = (n>>4) & 0xf;
  int b = (n>>0) & 0xf;
  if (f < 8)
    f += 30;
  else
    f += 90 - 8;

  if (b < 8)
    b += 40;
  else
    b += 100 - 8;

  putstr(CSI); putint(f); putstr(";"); putint(b); putstr("m");
}
