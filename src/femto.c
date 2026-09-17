#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define ESC_CLEAR "\x1b[2J"
#define ESC_CURSTOP "\x1b[H"

#define TERM_Y 24

struct termios termios_original;

void editor_draw_rows() {
  for (int y = 0; y < TERM_Y; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}

void editor_clear_screen() {
  write(STDOUT_FILENO, ESC_CLEAR, 4);
  write(STDOUT_FILENO, ESC_CURSTOP, 3);
}

void editor_refresh_screen() {
  editor_clear_screen();
  editor_draw_rows();
  write(STDOUT_FILENO, ESC_CURSTOP, 3);
}

void die(const char *msg) {
  editor_clear_screen();
  perror(msg);
  exit(1);
}
void disable_raw_mode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_original) == -1)
    die("tcsetattr");
}

void enable_raw_mode() {
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

char editor_read_key() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1) != 1))
    if (nread == -1 && errno != EAGAIN) {
      die("read");
    }

  return c;
}

void editor_process_keys() {
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
