#ifndef __TTY_H__
#define __TTY_H__

void tty_init(void);
void tty_uninit(void);
int  available(void);
int  getkey(void);
void raw_mode(void);
void cooked_mode(void);

#endif /* __TTY_H__ */
