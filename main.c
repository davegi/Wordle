// #define _MUNIT_
#if !defined(_MUNIT_)
  #include <assert.h>
  // #include <readline/readline.h>
  #include <stdbool.h>
  #include <stdio.h>
  #include <stdlib.h>

  #define MUNIT_ARRAY_PARAM(name) name
  #include "game.h"
  #define N_GUESSES 6

const char* map_colors_t_to_COLORS[] = {
  [grey]   = BACKGROUND_BLACK,
  [green]  = BRIGHT_BACKGROUND_GREEN,
  [yellow] = BRIGHT_BACKGROUND_YELLOW,
  [black]  = BRIGHT_BACKGROUND_BLACK,
};
#else
  #include "game_ut.h"
  #include "munit_ex.h"

static const MunitSuite test_suite = { munit_ex_register_suite_easy(stat, test_suite_tests) };
#endif  // __MUNIT__

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
#if !defined(_MUNIT_)
  assert(argc == 2);
  const word_t** words = load_words(argv[1]);
  assert(words);
  size_t n_words = nb_words(argv[1]);
  assert(n_words > 0);
  srand(0);
  int secret_word_index     = rand() % n_words;
  const word_t* secret_word = words[secret_word_index];
  assert(secret_word);
  puts((const char*) secret_word);
  char* guess      = NULL;
  size_t line_size = 0;
  for (size_t i = 0; i < N_GUESSES; i++) {
    // char* guess = readline("Guess: ");
    printf("Guess: %zu: ", i);
    ssize_t s = getline(&guess, &line_size, stdin);
    assert(s > 0);
    assert(guess);
    colors_t colors;
    size_t matches = color_word(guess, (const char*) secret_word, colors);
    printf("matches: %zu\n", matches);
    for (size_t i = 0; i < WORD_LENGTH; i++) {
      printf("%s%c", map_colors_t_to_COLORS[colors[i]], guess[i]);
    }
    puts(NO_COLOR);
    if (WORD_LENGTH == matches) {
      printf("You win!\n");
      break;
    }
  }
  free(guess);
  unload_words(words);
#else
  return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
#endif  // __MUNIT__
}
