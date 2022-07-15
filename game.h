#pragma once
#include <stddef.h>
#include <sys/stat.h>
//
#define ESC          "\x1b"
#define CSI          ESC "["
#define RGB_FG(r, g, b) CSI "38;2;" #r ";" #g ";" #b "m"
#define RGB_BG(r, g, b) CSI "48;2;" #r ";" #g ";" #b "m"
#define YELLOW       RGB_BG(255, 255, 0)
#define PEACH        RGB_FG(255, 203, 164)  // 255, 218, 185
#define NO_COLOR     CSI "0m"
#define BOLD         CSI "1m"
#define ITALIC       CSI "3m"
#define NO_ITALIC    CSI "23m"
//
#define BACKGROUND_BLACK         CSI "40m"
#define BRIGHT_BACKGROUND_BLACK  CSI "40m"
#define BRIGHT_BACKGROUND_YELLOW RGB_BG(201, 211, 30)
#define BRIGHT_BACKGROUND_GREEN  RGB_BG(65, 110, 61)

//
#define WORD_LENGTH      5
#define WORD_LENGTH_W_NL (WORD_LENGTH + 1)
typedef char word_t[WORD_LENGTH];
//
typedef enum { black, grey, yellow, green } color_t;
typedef color_t colors_t[WORD_LENGTH];
// return the file size given its name
off_t fsize(const char* filename);
// return the file size given its descriptor
off_t fsize_fd(int fd);
// return the number of words in a file
size_t nb_words(const char* filename);
// load each word from a file into a word array
const word_t** load_words(const char* filename);
// unload the word array
void unload_words(const word_t** words);
//
size_t color_word(const char* guess, const char* secret, colors_t colors);
