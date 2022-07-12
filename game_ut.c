#include <assert.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
//
#include <search.h>

#include "munit.h"
#include "munit_ex.h"
// .clang-format off
#include "game.c"
// .clang-format on

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
// compare function for lfind()
int compare_file_t(const void* a, const void* b) {
  const char* filename = (const char*) a;
  file_t fb            = *((file_t*) b);
  return strcmp(filename, fb.filename);
}
// use lsearch() to find the file_t struct for the given filename
const file_t* find_file_t(const char* filename) {
  size_t size     = sizeof(files) / sizeof(file_t);
  const file_t* f = lfind(filename, files, &size, sizeof(file_t), compare_file_t);
  assert(f);
  return f;
}
//
const file_t* munit_parameters_get_file_t(const MunitParameter params[]) {
  const char* key = munit_parameters_get(params, "file");
  munit_assert_ptr_not_null(key);
  const file_t* file = find_file_t(key);
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
typedef enum { black, grey, yellow, green } color_t;
#define WORD_LENGTH 5
typedef color_t color_array_t[WORD_LENGTH];
void color_word(const char* word1, const char* word2, color_array_t color_array) {
  (void) word1;
  (void) word2;
  memcpy(color_array,((color_array_t[]){ { green, green, green, green, green } }),sizeof(color_array_t));
  return;
}
//
static MunitResult color_word_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  color_array_t colors;
  const file_t* file = munit_parameters_get_file_t(params);
  munit_assert_ptr_not_null(file);
  log_file(file);
  // act
  color_word(file->fw, file->fw, colors);
  // assert
  munit_assert_memory_equal(sizeof(color_array_t), colors, ((color_array_t[]){{green, green, green, green, green}}));

  return MUNIT_OK;
}
//
static MunitResult stat_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  const file_t* file = munit_parameters_get_file_t(params);
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
  const file_t* file = munit_parameters_get_file_t(params);
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
  const file_t* file = munit_parameters_get_file_t(params);
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
  const file_t* file = munit_parameters_get_file_t(params);
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

  const file_t* file = munit_parameters_get_file_t(params);
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
  const file_t* file = munit_parameters_get_file_t(params);
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
  const file_t* file = munit_parameters_get_file_t(params);
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
  const file_t* file = munit_parameters_get_file_t(params);
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
  munit_ex_register_test(color_word_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  { NULL }
};

static const MunitSuite test_suite = { munit_ex_register_suite_easy(stat, test_suite_tests) };

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) { return munit_suite_main(&test_suite, (void*) "µnit", argc, argv); }
