#include "game.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
//
size_t color_word(const char* guess, const char* secret, colors_t colors) {
  assert(guess);
  assert(secret);
  assert(colors);
  // assume no matches
  memset(colors, black, sizeof(colors_t));
  // compare each letter in the guess to each letter in the secret word
  // if the letters match, set the color to yellow,
  // if the letters are in the same position, set the color to green,
  // else set the color to grey
  size_t n_matches = 0;
  for (size_t i = 0; i < WORD_LENGTH; i++) {
    char guess_letter = guess[i];
    for (size_t j = 0; j < WORD_LENGTH; j++) {
      char secret_letter = secret[j];
      // colors[i]          = grey;
      if (guess_letter == secret_letter) {
        colors[i] = yellow;
        if (i == j) {
          n_matches++;
          colors[i] = green;
          break;
        }
      }
    }
    colors[i] = colors[i] == black ? grey : colors[i];
  }
  // if (n_matches) n_matches--;
  // assert(n_matches <= WORD_LENGTH);
  return n_matches;
}
