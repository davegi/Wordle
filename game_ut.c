#include <assert.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
//
#include <search.h>

#include "munit.h"
#include "munit_ex.h"
// .clang-format off
#include "game.h"
#include "game_ut.h"
// .clang-format on

typedef struct {
  size_t n_words;
  off_t size;
  const char* filename;
  const char* fw;
  const char* lw;
  const char* mw;
  colors_t fw_lw;
  colors_t fw_mw;
  colors_t mw_lw;
} file_t;
#define WORD_FILES "word files"
const char* get_word_files_path(const char* filename) {
  static char path[PATH_MAX];
  sprintf(path, "%s/%s", WORD_FILES, filename);
  return path;
}
#define PARAMS(a, b, c, d, e) a, b, c, d, e  // __VA_ARGS__
// n_words / size / filename / first_word / last_word / middle_word / fw_lw /
#define FILE_TABLE(FILE)                                                                                                                        \
  FILE(10657, 63941, "wordle-allowed-guesses.txt", "aahed", "zymic", "lours", PARAMS({ grey, grey, grey, grey, grey }))                         \
  FILE(2315, 13889, "wordle-answers-alphabetical.txt", "aback", "zonal", "lousy", PARAMS({ yellow, grey, yellow, grey, grey }))                 \
  FILE(10638, 63827, "wordle-nyt-allowed-guesses.txt", "aahed", "zymic", "loved", PARAMS({ grey, grey, grey, grey, grey }))                     \
  FILE(2309, 13853, "wordle-nyt-answers-alphabetical.txt", "aback", "zonal", "louse", PARAMS({ yellow, grey, yellow, grey, grey }))             \
  FILE(0, 0, NULL, NULL, NULL, NULL, PARAMS({ black, black, black, black, black }))
#define EXPAND_AS_FILE_T(n_words, size, filename, fw, lw, mw, fw_lw)   { n_words, size, filename, fw, lw, mw, fw_lw },
#define EXPAND_AS_FILENAME(n_words, size, filename, fw, lw, mw, fw_lw) filename,
#define EXPAND_AS_FW(n_words, size, filename, fw, lw, mw)              fw,
static const file_t files[]                                = { FILE_TABLE(EXPAND_AS_FILE_T) };
static char* file_params[sizeof(files) / sizeof(files[0])] = { FILE_TABLE(EXPAND_AS_FILENAME) };
// compare function for lfind()
// int compare_file_t(const void* a, const void* b) {
int compare_file_t(const char* a, file_t* b) {
  const char* filename = (const char*) a;
  file_t fb            = *((file_t*) b);
  return strcmp(filename, fb.filename);
}
// use lsearch() to find the file_t struct for the given filename
const file_t* find_file_t(const char* filename) {
  size_t size     = sizeof(files) / sizeof(file_t);
  const file_t* f = lfind(filename, files, &size, sizeof(file_t), (__compar_fn_t) compare_file_t);
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
static MunitResult color_word_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  colors_t colors    = { black };
  const file_t* file = munit_parameters_get_file_t(params);
  munit_assert_ptr_not_null(file);
  log_file_word(file, fw);
  // act
  color_word(file->fw, file->fw, colors);
  color_word(file->lw, file->lw, colors);
  color_word(file->mw, file->mw, colors);
  // assert
  munit_assert_memory_equal(sizeof(colors_t), colors, ((colors_t[]){ { green, green, green, green, green } }));

  return MUNIT_OK;
}
//
static MunitResult fw_lw_color_word_test(const MunitParameter params[], void* user_data) {
  (void) user_data;
  // arrange
  colors_t colors    = { black };
  const file_t* file = munit_parameters_get_file_t(params);
  munit_assert_ptr_not_null(file);
  log_file_word(file, fw);
  // act
  color_word(file->fw, file->lw, colors);
  // assert
  munit_assert_memory_equal(sizeof(colors_t), colors, file->fw_lw);

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

MunitTest test_suite_tests[] = {
  munit_ex_register_test(stat_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(fstat_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(fsize_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(nb_words_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(load_words_test_fw, load_words_setup, load_words_tear_down, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(load_words_test_lw, load_words_setup, load_words_tear_down, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(load_words_test_mw, load_words_setup, load_words_tear_down, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(color_word_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  munit_ex_register_test(fw_lw_color_word_test, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params),
  { NULL }
};
