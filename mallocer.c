// mallocer - A program to experiment with memory allocation
// Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <argp.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>

extern void* etext;
extern void* edata;
extern void* end;

static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"fill",     'f', 0,      0,  "Fill allocated memory with data" },
  {"pattern-fill", 'F', "BYTESEQ",  0,  "Fill allocated memory with the given pattern (hex)" },
  {"random-fill", 'r', 0,   0,  "Fill allocated memory with random data" },
  {"calloc",   'C', 0,      0,  "Use calloc() instead of malloc()" },
  {"interval", 'i', "MSEC", 0,  "Time in milisec between allocations" },
  {"count", 'c', "NUM", 0,  "Limit number of memory allocations to NUM" },
  {"increment", 'I', "BYTES", 0, "Increase allocation size by BYTES on each step" },
  {"size",     's', "BYTES", 0, "Bytes to allocate on each step" },
  {"name",     'n', "NAME", 0, "Changes the name of the process" },
  { 0 }
};

struct Options
{
  bool verbose;
  bool fill;
  char* pattern_fill;
  size_t pattern_fill_size;
  bool random_fill;
  bool calloc;
  size_t alloc_size;
  size_t alloc_increment;
  int max_count;
  int interval;
  char* name;
};

void fatal_error(const char* msg)
{
  puts(msg);
  exit(EXIT_FAILURE);
}

/** Given a text string in hex form, convert it to a newly allocated
    sequence of bytes. The caller is responsible to free() 'buf'. */
void hex2bytes(const char* text, char** buf, size_t* buf_size)
{
  size_t len = strlen(text);
  if (len % 2 != 0)
  {
    fatal_error("byte string must be multiple of two");
  }

  *buf_size = len / 2;
  *buf = malloc(*buf_size);
  assert(*buf);

  for (size_t i = 0; i < len; i += 2) {
    if (sscanf(text + i, "%2hhx", &(*buf)[i/2]) != 1)
    {
      fatal_error("incorrect bytes string");
    }
  }
}

struct Unit
{
  const char* name;
  size_t value;
};

struct Unit g_units[] =
{
  { "",  1ull },
  { "K", 1024ull },
  { "M", 1024ull * 1024 },
  { "G", 1024ull * 1024 * 1024 },
  { "T", 1024ull * 1024 * 1024 * 1024 },
  { "P", 1024ull * 1024 * 1024 * 1024 * 1024 },
  { "E", 1024ull * 1024 * 1024 * 1024 * 1024 * 1024 },
  { "Z", 1024ull * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 },
  { "Y", 1024ull * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 },

  { "KiB", 1024ull },
  { "MiB", 1024ull * 1024 },
  { "GiB", 1024ull * 1024 * 1024 },
  { "TiB", 1024ull * 1024 * 1024 * 1024 },
  { "PiB", 1024ull * 1024 * 1024 * 1024 * 1024 },
  { "EiB", 1024ull * 1024 * 1024 * 1024 * 1024 * 1024 },
  { "ZiB", 1024ull * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 },
  { "YiB", 1024ull * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 * 1024 },

  { "kB", 1000ull },
  { "MB", 1000ull * 1000 },
  { "GB", 1000ull * 1000 * 1000 },
  { "TB", 1000ull * 1000 * 1000 * 1000 },
  { "PB", 1000ull * 1000 * 1000 * 1000 * 1000 },
  { "EB", 1000ull * 1000 * 1000 * 1000 * 1000 * 1000 },
  { "ZB", 1000ull * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 },
  { "YB", 1000ull * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 * 1000 },
};


size_t apply_unit(size_t num, const char* unit)
{
  for(int i = 0; i < sizeof(g_units) / sizeof(g_units[0]); ++i)
  {
    if (strcmp(unit, g_units[i].name) == 0)
    {
      return num * g_units[i].value;
    }
  }
  fatal_error("invalid unit name");
  return num;
}

size_t text2bytes(const char* text)
{
  // find the first character that isn't a digit, this is the start of
  // the unit string
  const char* digit_end = text;
  while(isdigit(*digit_end) && *digit_end != '\0')
  {
    ++digit_end;
  }

  // skip white space
  const char* unit = digit_end;
  while(isspace(*unit) && *unit != '\0')
  {
    ++unit;
  }

  char* endptr;
  errno = 0;
  size_t result = strtol(text, &endptr, 10);
  if (errno != 0)
  {
    perror(text);
    exit(EXIT_FAILURE);
  }

  if (endptr == digit_end || *endptr == '\0')
  {
    return apply_unit(result, unit);
  }
  else
  {
    printf("invalid bytes string: '%s'", text);
    exit(EXIT_FAILURE);
  }
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct Options* opts = state->input;

  switch(key)
  {
    case 'v':
      opts->verbose = true;
      break;

    case 'C':
      opts->calloc = true;
      break;

    case 'c':
      opts->max_count = atoi(arg);
      break;

    case 'f':
      opts->fill = true;
      break;

    case 'F':
      hex2bytes(arg,
                &opts->pattern_fill,
                &opts->pattern_fill_size);
      break;

    case 'r':
      opts->random_fill = true;
      break;

    case 's':
      opts->alloc_size = text2bytes(arg);
      break;

    case 'I':
      opts->alloc_increment = strtoll(arg, NULL, 10);
      break;

    case 'i':
      opts->interval = atoi(arg);
      break;

    case 'n':
      opts->name = arg;
      break;

    default:
      ARGP_ERR_UNKNOWN;
      break;
  }

  return 0;
}

static struct argp argp = {
  options, parse_opt,
  "",
  "A program to experiment with memory allocation"
};

void run(struct Options* opts)
{
  uint8_t first_byte = 0xff;

  printf("program break according to sbrk() at %p\n", sbrk(0));
  printf("First byte in main is at: %p\n", &first_byte);

  printf("First address past:\n");
  printf("    program text (etext)      %10p\n", &etext);
  printf("    initialized data (edata)  %10p\n", &edata);
  printf("    uninitialized data (end)  %10p\n", &end);

  puts("mallocer is going to allocate some memory...");

  int alloc_count = 1;
  size_t total_heap = 0;
  char* last_buffer = NULL;
  while(true)
  {
    size_t len = opts->alloc_size + opts->alloc_increment * (alloc_count - 1);
    char* buffer;
    if (opts->calloc)
    {
      printf("%d) trying to allocate %zu with calloc()\n", alloc_count, len);
      buffer = calloc(1, len);
    }
    else
    {
      printf("%d) trying to allocate %zu with malloc()\n", alloc_count, len);
      buffer = malloc(len);
    }

    if (!buffer)
    {
      puts("error: out of memory, sleeping for 1sec, then trying again");
      sleep(1);
      continue;
    }
    else
    {
      total_heap += len;
      alloc_count += 1;

      printf("allocation succesful, new total memory: %zu at %p\n",
             total_heap, buffer);
      if (last_buffer != NULL)
      {
        printf("distance to last allocation %zd\n", last_buffer - buffer);
      }
      last_buffer = buffer;
    }

    if (opts->fill)
    {
      const char fill_pattern[] = { 0xde, 0xad, 0xbe, 0xef };
      printf("filling memory with pattern data\n");
      for(int i = 0; i < len; ++i)
      {
        buffer[i] = fill_pattern[i % sizeof(fill_pattern)];
      }
    }
    else if (opts->pattern_fill)
    {
      printf("filling memory with custom pattern data\n");
      for(int i = 0; i < len; ++i)
      {
        buffer[i] = opts->pattern_fill[i % opts->pattern_fill_size];
      }
    }
    else if (opts->random_fill)
    {
      printf("filling memory with random data\n");
      for(int i = 0; i < len; ++i)
      {
        buffer[i] = rand() % 0xff;
      }
    }

    if (opts->interval < 0 ||
        (opts->max_count != -1 && alloc_count > opts->max_count))
    {
      printf("going to sleep forever\n");
      while(true) {
        sleep(UINT_MAX);
      }
    }
    else
    {
      usleep(opts->interval * 1000);
    }
  }
}

int main(int argc, char** argv)
{
  struct Options opts = {
    .verbose = false,
    .fill = false,
    .pattern_fill = NULL,
    .pattern_fill_size = 0,
    .random_fill = false,
    .calloc = false,
    .alloc_size = 1024 * 1024,
    .alloc_increment = 0,
    .max_count = -1,
    .interval = 1000,
    .name = NULL
  };

  argp_parse(&argp, argc, argv, 0, 0, &opts);

  // change the processes name
  if (opts.name != NULL)
  {
    // this changes argv[0], which is used by 'ps'
    if (strcmp(opts.name, argv[0]) != 0)
    {
      argv[0] = opts.name;
      if (execvp("/proc/self/exe", argv) < 0)
      {
        perror("execv failed:");
        exit(EXIT_FAILURE);
      }
    }

    // this sets /proc/$PID/comm which is used by Python's
    // psutil.Proccess.name()
    prctl(PR_SET_NAME, opts.name);
  }

  printf("process is named '%s' with pid %d\n", argv[0], (int)getpid());
  run(&opts);

  return 0;
}

/* EOF */
