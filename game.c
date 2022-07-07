#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
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
//******************************************************************************
#include "munit.h"
#include "munit_ex.h"
typedef struct {
  size_t n_words;
  off_t size;
  const char* filename;
  const char* fw;
  const char* lw;
  const char* mw;
} file_t;
#define WORD_FILES "word files"
const char* get_word_files_path(const char* filename) {
  static char path[PATH_MAX];
  sprintf(path, "%s/%s", WORD_FILES, filename);
  return path;
}
// n_words / size / filename / first_word / last_word / middle_word
#define FILE_TABLE(FILE)                                                                                                                        \
  FILE(10657, 63941, "wordle-allowed-guesses.txt", "aahed", "zymic", "lours")                                                                   \
  FILE(2315, 13889, "wordle-answers-alphabetical.txt", "aback", "zonal", "lousy")                                                               \
  FILE(10638, 63827, "wordle-nyt-allowed-guesses.txt", "aahed", "zymic", "loved")                                                               \
  FILE(2309, 13853, "wordle-nyt-answers-alphabetical.txt", "aback", "zonal", "louse")                                                           \
  FILE(0, 0, NULL, NULL, NULL, NULL)
#define EXPAND_AS_FILE_T(n_words, size, filename, fw, lw, mw)   { n_words, size, filename, fw, lw, mw },
#define EXPAND_AS_FILENAME(n_words, size, filename, fw, lw, mw) filename,
#define EXPAND_AS_FW(n_words, size, filename, fw, lw, mw)       fw,
static const file_t files[]                                = { FILE_TABLE(EXPAND_AS_FILE_T) };
static char* file_params[sizeof(files) / sizeof(files[0])] = { FILE_TABLE(EXPAND_AS_FILENAME) };
#include <search.h>
// compare function for lfind()
int compare_file_t(const void* a, const void* b) {
  const char* filename = (const char*) a;
  file_t fb            = *((file_t*) b);
  return strcmp(filename, fb.filename);
}
// use lsearch() to find the file_t struct for the given filename
const file_t* find_file(const char* filename) {
  size_t size     = sizeof(files) / sizeof(file_t);
  const file_t* f = lfind(filename, files, &size, sizeof(file_t), compare_file_t);
  assert(f);
  return f;
}
//
const file_t* munit_parameters_get_file(const MunitParameter params[]) {
  const char* key = munit_parameters_get(params, "file");
  munit_assert_ptr_not_null(key);
  const file_t* file = find_file(key);
  munit_assert_ptr_not_null(file);
  return file;
}
//
#define log_file_field(file, field, format)                                                                                                     \
  fputs("  ", stderr);                                                                                                                          \
  munit_logf(MUNIT_LOG_INFO, PEACH #field " = " format NO_COLOR, file->field);
//
#define log_file_word(file, word) log_file_field(file, word, "%s")
//
#define log_file(file)                                                                                                                          \
  fputs("  ", stderr);                                                                                                                          \
  munit_logf(MUNIT_LOG_INFO, PEACH "n_words = %lu" NO_COLOR, file->n_words);
//
static MunitResult stat_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  log_file(file);
  // act
  off_t size = fsize(get_word_files_path(file->filename));
  // assert
  munit_assert_int(size, ==, file->size);

  return MUNIT_OK;
}
//
static MunitResult fstat_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  log_file(file);
  int fd = open(get_word_files_path(file->filename), O_RDONLY);
  if (-1 == fd) perror("open");
  munit_assert_int(fd, !=, -1);
  // act
  off_t size = fsize_fd(fd);
  // assert
  munit_assert_int(size, ==, file->size);
  close(fd);

  return MUNIT_OK;
}
//
static MunitResult fsize_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  log_file(file);
  // act
  off_t size = fsize(get_word_files_path(file->filename));
  // assert
  munit_assert_size(size, ==, file->size);

  return MUNIT_OK;
}
//
static MunitResult nb_words_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  log_file(file);
  // act
  size_t n_words = nb_words(get_word_files_path(file->filename));
  // assert
  munit_assert_size(n_words, ==, file->n_words);

  return MUNIT_OK;
}
//
static void* load_words_setup(const MunitParameter params[], void* user_data) {
  (void) user_data;

  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  log_file(file);
  const word_t** words = load_words(get_word_files_path(file->filename));
  munit_assert_ptr_not_null(words);

  return (void*) words;
}
//
static void load_words_tear_down(void* fixture) { unload_words(fixture); }
//
static MunitResult load_words_test_fw(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  log_file_word(file, fw);
  // act
  const word_t** words = (const word_t**) user_data;
  // assert
  munit_assert_ptr_not_null(words);
  munit_assert_string_equal((const char*) words[0], file->fw);

  return MUNIT_OK;
}
//
static MunitResult load_words_test_lw(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  size_t n_words = nb_words(get_word_files_path(file->filename));
  log_file_word(file, lw);
  // act
  const word_t** words = (const word_t**) user_data;
  // assert
  munit_assert_ptr_not_null(words);
  munit_assert_string_equal((const char*) words[n_words - 1], file->lw);

  return MUNIT_OK;
}
//
static MunitResult load_words_test_mw(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file(params);
  munit_assert_ptr_not_null(file);
  size_t n_words = nb_words(get_word_files_path(file->filename));
  log_file_word(file, mw);
  // act
  const word_t** words = (const word_t**) user_data;
  // assert
  munit_assert_ptr_not_null(words);
  munit_assert_string_equal((const char*) words[n_words / 2], file->mw);

  return MUNIT_OK;
}

// static void* setup(const MunitParameter params[], void* user_data) {
//   (void) params;
//   (void) user_data;
//   static int i = 0;

//   return (void*) (uintptr_t) &files[i];
// }

// static void test_compare_tear_down(void* fixture) { munit_assert_ptr_equal(fixture, (void*) (uintptr_t) 0xdeadbeef); }

// static char* bar_params[] = { (char*) "red", (char*) "green", (char*) "blue", NULL };

static MunitParameterEnum test_params[] = {
  { (char*) "file", file_params },
  // { (char*) "bar", bar_params },
  // { (char*) "baz", NULL },
  { NULL },
};

static MunitTest test_suite_tests[] = {
  munit_ex_register_test(stat_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(fstat_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(fsize_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(nb_words_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(load_words_test_fw, load_words_setup, load_words_tear_down, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(load_words_test_lw, load_words_setup, load_words_tear_down, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(load_words_test_mw, load_words_setup, load_words_tear_down, MUNIT_TEST_OPTION_NONE, test_params),
  { NULL }
};

static const MunitSuite test_suite = { munit_ex_register_suite_easy(stat, test_suite_tests) };

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) { return munit_suite_main(&test_suite, (void*) "µnit", argc, argv); }
