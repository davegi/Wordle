#include "munit_ex.h"

static MunitResult test_compare(const MunitParameter params[], void* user_data) {
  (void) params;
  (void) user_data;

  return MUNIT_OK;
}

static MunitResult
test_rand(const MunitParameter params[], void* user_data) {
  (void) params;
  (void) user_data;

  return MUNIT_OK;
}
//
static MunitResult test_parameters(const MunitParameter params[], void* user_data) {
  (void) params;
  (void) user_data;

  return MUNIT_OK;
}

static void* test_compare_setup(const MunitParameter params[], void* user_data) {
  (void) params;
  (void) user_data;

  return (void*) (uintptr_t) 0xdeadbeef;
}

static void
test_compare_tear_down(void* fixture) {
  munit_assert_ptr_equal(fixture, (void*)(uintptr_t)0xdeadbeef);
}

static char* foo_params[] = {
  (char*) "one", (char*) "two", (char*) "three", NULL
};

static char* bar_params[] = {
  (char*) "red", (char*) "green", (char*) "blue", NULL
};

static MunitParameterEnum test_params[] = {
  { (char*) "foo", foo_params },
  { (char*) "bar", bar_params },
  { (char*) "baz", NULL },
  { NULL, NULL },
};

static MunitTest test_suite_tests[] = {
  {
    (char*) "/example/compare",
    test_compare,
    test_compare_setup,
    test_compare_tear_down,
    MUNIT_TEST_OPTION_NONE,
    NULL
  },
  { (char*) "/example/rand", test_rand, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
  { (char*) "/example/parameters", test_parameters, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params },
  { NULL }
};

static const MunitSuite test_suite = {
  (char*) "",
  test_suite_tests,
  NULL,
  1,
  MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
