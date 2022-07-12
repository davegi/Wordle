#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//
#define ESC          "\x1b"
#define CSI          ESC "["
#define RGB(r, g, b) CSI "38;2;" #r ";" #g ";" #b "m"
#define YELLOW       RGB(255, 255, 0)
#define PEACH        RGB(255, 203, 164)  // 255, 218, 185
#define NO_COLOR     CSI "0m"
#define BOLD         CSI "1m"
#define ITALIC       CSI "3m"
#define NO_ITALIC    CSI "23m"

#define WORD_LENGTH      5
#define WORD_LENGTH_W_NL (WORD_LENGTH + 1)
typedef char word_t[WORD_LENGTH];

// return the file size given its name
off_t fsize(const char* filename) {
  struct stat st;
  if (stat(filename, &st) == 0) return st.st_size;
  perror("fsize()::stat()");
  return -1;
}
// return the file size given its descriptor
off_t fsize_fd(int fd) {
  struct stat st;
  if (fstat(fd, &st) == 0) return st.st_size;
  perror("fsize_fd()::fstat()");
  return -1;
}
// return the number of words in a file
size_t nb_words(const char* filename) {
  off_t size = fsize(filename);
  if (size == -1) return 0;
  size = size % 2 == 0 ? size : size + 1;
  assert(size % 2 == 0);
  assert(size % WORD_LENGTH_W_NL == 0);
  assert(size / WORD_LENGTH_W_NL > 0);
  return size / WORD_LENGTH_W_NL;
}
// load each word from a file into a word array
const word_t** load_words(const char* filename) {
  size_t n_words = nb_words(filename);
  if (n_words == 0) return NULL;
  const word_t** words = NULL;
  FILE* f              = fopen(filename, "r");
  if (f == NULL) return NULL;
  // add room for NULL termination
  words = malloc((n_words + 1) * sizeof(word_t*));
  if (words == NULL) {
    fclose(f);
    return NULL;
  }
  // buffer length includes length of word plus newline plus nul byte
  char buffer[WORD_LENGTH_W_NL + 1];
  for (size_t i = 0; i < n_words; i++) {
    if (fgets(buffer, sizeof(buffer), f) == NULL) {
      fclose(f);
      free(words);
      return NULL;
    }
    // remove '\n' from the end of the word
    buffer[WORD_LENGTH] = '\0';
    words[i]            = (word_t*) strdup(buffer);
  }
  // NULL terminate the array
  words[n_words] = NULL;

  fclose(f);
  return words;
}
// unload the word array
void unload_words(const word_t** words) {
  if (words == NULL) return;
  for (size_t i = 0; words[i]; i++) {
    free((void*) words[i]);
  }
  free((void*) words);
}
