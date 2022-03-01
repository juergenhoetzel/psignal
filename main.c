#include <unistd.h>

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void obtain_signal_info(const char *pidstr) {
  FILE *stream;
  char statfile[PATH_MAX];
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  snprintf(statfile, sizeof(statfile), "/proc/%s/status", pidstr);
  stream = fopen(statfile, "r");
  if (stream == NULL) {
    perror("fopen");
    exit(1); /* FIXME proper error handling */
  }

  while ((nread = getline(&line, &len, stream)) != -1) {
    if (strncmp("Sig", line, 3) == 0 || (strncmp("ShdPnd", line, 6) == 0)) {
      switch (line[3]) {
      case 'Q': {
        fputs(line, stdout);
        break;
      }
      case 'P':
      case 'B':
      case 'I':
      case 'C': {
        const char *s = strchr(line, ':') + 1;
        printf("%.6s:\t", line);
        long signals = strtol(s, NULL, 16);
        int initial = 1;
        for (int i = 1; i <= SIGSYS; i++) {
          if (signals & (1 << (i - 1))) {
            printf("%sSIG%s", initial ? "" : ", ", sigabbrev_np(i));
            initial = 0;
          }
        }
        puts("");
        break;
      }

      default:
        break;
      }
    }
  }

  free(line);
  fclose(stream);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fputs("Usage: psignal PID\n", stderr);
    return 1;
  }
  obtain_signal_info(argv[1]);
  return 0;
  ;
}
