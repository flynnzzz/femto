#include <asm-generic/ioctls.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

#define ESC_CLEAR "\x1b[2J"
#define ESC_CURSTOP "\x1b[H"

struct editor_config {
  int screenrows, screencols;
  struct termios orig_termios;
};
struct editor_config E;

int windows_size(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  }

  *cols = ws.ws_col;
  *rows = ws.ws_row;
  return 0;
}

void editor_draw_rows() {
  for (int y = 0; y < E.screenrows; y++) {
    write(STDOUT_FILENO, "~", 1);
    if (y < E.screenrows - 1)
      write(STDOUT_FILENO, "\r\n", 2);
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

void init_editor() {
  if (windows_size(&E.screenrows, &E.screencols) == -1)
    die("get_window_size");
}

void disable_raw_mode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enable_raw_mode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");

  atexit(disable_raw_mode);

  struct termios termios_new = E.orig_termios;
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
  init_editor();

  while (1) {
    editor_refresh_screen();
    editor_process_keys();
  }

  return 0;
}
