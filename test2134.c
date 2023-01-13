#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
  #define THE_OS_WINDOWS
  #define THE_EOL "\r\n"
  #define THE_PATH_SEP "\\"
#else
  #if defined(__APPLE__)
    #define THE_OS_MACOS
  #elif defined(__linux__)
    #define THE_OS_LINUX
  #endif
  #define THE_EOL "\n"
  #define THE_PATH_SEP "/"
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct str {
  char *d;
  size_t l;
};

void *alloc (size_t);
void print (FILE *, const char *, ...);
struct str str_alloc (const char *);
struct str str_copy (const struct str);
void str_free (struct str);
struct str __THE_1_RGB_0_str (enum __THE_1_RGB_0);

void *alloc (size_t l) {
  void *r = malloc(l);
  if (r == NULL) {
    fprintf(stderr, "Error: failed to allocate %zu bytes" THE_EOL, l);
    exit(EXIT_FAILURE);
  }
  return r;
}
void print (FILE *stream, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[512];
  struct str s;
  while (*fmt) {
    switch (*fmt++) {
      case 't': fputs(va_arg(args, int) ? "true" : "false", stream); break;
      case 'b': sprintf(buf, "%u", va_arg(args, unsigned)); fputs(buf, stream); break;
      case 'c': fputc(va_arg(args, int), stream); break;
      case 'e':
      case 'f':
      case 'g': snprintf(buf, 512, "%f", va_arg(args, double)); fputs(buf, stream); break;
      case 'h':
      case 'j':
      case 'v':
      case 'w': sprintf(buf, "%d", va_arg(args, int)); fputs(buf, stream); break;
      case 'i':
      case 'k': sprintf(buf, "%" PRId32, va_arg(args, int32_t)); fputs(buf, stream); break;
      case 'l': sprintf(buf, "%" PRId64, va_arg(args, int64_t)); fputs(buf, stream); break;
      case 'p': sprintf(buf, "%p", va_arg(args, void *)); fputs(buf, stream); break;
      case 's': s = va_arg(args, struct str); fwrite(s.d, 1, s.l, stream); str_free(s); break;
      case 'u': sprintf(buf, "%" PRIu32, va_arg(args, uint32_t)); fputs(buf, stream); break;
      case 'y': sprintf(buf, "%" PRIu64, va_arg(args, uint64_t)); fputs(buf, stream); break;
      case 'z': fputs(va_arg(args, char *), stream); break;
    }
  }
  va_end(args);
}
struct str str_alloc (const char *r) {
  size_t l = strlen(r);
  char *d = alloc(l);
  memcpy(d, r, l);
  return (struct str) {d, l};
}
struct str str_copy (const struct str s) {
  char *d = alloc(s.l);
  memcpy(d, s.d, s.l);
  return (struct str) {d, s.l};
}
void str_free (struct str s) {
  free(s.d);
}
struct str __THE_1_RGB_0_str (enum __THE_1_RGB_0 n) {
}

int main () {
  enum __THE_1_RGB_0 __THE_0_color_0 = __THE_0_RGB_0->__THE_0_Red;
  const enum __THE_1_RGB_0 __THE_0_green_0 = __THE_1_RGBSDGreen_0;
  const enum __THE_1_RGB_0 __THE_0_blue_0 = __THE_0_RGB_0->__THE_0_Blue;
  __THE_0_color_0 = __THE_1_RGBSDGreen_0;
  __THE_0_color_0 = __THE_0_RGB_0->__THE_0_Green;
  print(stdout, "szszsz", __THE_1_RGB_0_str(__THE_0_color_0), " ", __THE_1_RGB_0_str(__THE_0_color_0), " ", str_copy(__THE_0_color_0->__THE_0_rawValue), THE_EOL);
  print(stdout, "szszsz", __THE_1_RGB_0_str(__THE_0_RGB_0->__THE_0_Red), " ", __THE_1_RGB_0_str(__THE_0_RGB_0->__THE_0_Red), " ", str_copy(__THE_0_RGB_0->__THE_0_Red->__THE_0_rawValue), THE_EOL);
}
