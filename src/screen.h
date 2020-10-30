#ifndef __SCREEN_H__
#define __SCREEN_H__

// Put the cursor in the upper left.
void home(void);

// Clear the screen.
void clear(void);

// Set the cursor position to x,y (zero-based indexing)
void setpos(int x, int y);

// Turn the cursor off.
void cursor_off(void);

// Turn the cursor on.
void cursor_on(void);

// Set the foreground color to rgb.
// Each component must have the value 0 ... 5.
void fg(int r, int g, int b);

// Set the foreground color to rgb.
// Each component must have the value 0 ... 5.
void bg(int r, int g, int b);

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
void fgbg(int n);

#endif /* __SCREEN_H__ */
