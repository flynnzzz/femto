#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define ESC_CLEAR "\x1b[2J"
#define ESC_CURSTOP "\x1b[H"

struct termios termios_original;

static void editor_refresh_screen() {
  write(STDIN_FILENO, ESC_CLEAR, 4);
  write(STDIN_FILENO, ESC_CURSTOP, 3);
}

static void die(const char *msg) {
  editor_refresh_screen();
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

static char editor_read_key() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1) != 1))
    if (nread == -1 && errno != EAGAIN) {
      die("read");
    }

  return c;
}

static void editor_process_keys() {
  char c = editor_read_key();

  switch (c) {
  case CTRL_KEY('q'):
    editor_refresh_screen();
    exit(0);
    break;
  }
}

int main() {
  enable_raw_mode();

  while (1) {
    editor_refresh_screen();
    editor_process_keys();
  }

  return 0;
}
