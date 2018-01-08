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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <argp.h>

static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,  "Produce verbose output" },
  {"fill",     'f', 0,      0,  "Fill allocated memory with data" },
  {"calloc",   'c', 0,      0,  "Use calloc() instead of malloc()" },
  {"interval", 'i', "MSEC", 0,  "Time in milisec between allocations" },
  {"increment", 'I', "BYTES", 0, "Increase allocation size by BYTES on each step" },
  {"size",     's', "BYTES", 0, "Bytes to allocate on each step" },
  { 0 }
};

struct Options
{
  bool verbose;
  bool fill;
  bool calloc;
  size_t alloc_size;
  size_t alloc_increment;
  int  interval;
};

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

    case 'c':
      opts->calloc = true;
      break;

    case 'f':
      opts->fill = true;
      break;

    case 's':
      opts->alloc_size = strtoll(arg, NULL, 10);
      break;

    case 'I':
      opts->alloc_increment = strtoll(arg, NULL, 10);
      break;

    case 'i':
      opts->interval = atoi(arg);
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

  printf("First byte in main is at: %p\n", &first_byte);
  puts("mallocer is going to allocate some memory...");

  int alloc_index = 1;
  size_t total_heap = 0;
  char* last_buffer = NULL;
  while(true)
  {
    size_t len = opts->alloc_size + opts->alloc_increment * alloc_index;
    char* buffer;
    if (opts->calloc)
    {
      printf("%d) trying to allocate %zu with calloc()\n", alloc_index, len);
      buffer = calloc(1, len);
    }
    else
    {
      printf("%d) trying to allocate %zu with malloc()\n", alloc_index, len);
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
      alloc_index += 1;

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
      printf("filling memory with random data\n");
      for(int i = 0; i < len; ++i)
      {
        buffer[i] = rand() % 0xff;
      }
    }

    usleep(opts->interval * 1000);
  }
}

int main(int argc, char** argv)
{
  struct Options opts = {
    .verbose = false,
    .fill = false,
    .calloc = false,
    .alloc_size = 1024 * 1024,
    .alloc_increment = 0,
    .interval = 1000
  };

  argp_parse(&argp, argc, argv, 0, 0, &opts);

  run(&opts);

  return 0;
}

/* EOF */
