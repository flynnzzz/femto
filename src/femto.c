#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios termios_original;

static void die(const char *msg) {
  perror(msg);
  exit(1);
}
static void disable_raw_mode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_original) == -1)
    die("tcsetattr");
}

static void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &termios_original) == -1)
    die("tcgetattr");

  atexit(disable_raw_mode);

  struct termios termios_new = termios_original;
  termios_new.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  termios_new.c_iflag &= ~(IXON | ICRNL);
  termios_new.c_oflag &= ~(OPOST);
  termios_new.c_cc[VMIN] = 0;
  termios_new.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_new) == -1)
    die("tcsetattr");
}

int main() {
  enable_raw_mode();

  char c;
  while (1) {
    c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read");

    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q')
      break;
  }

  return 0;
}
