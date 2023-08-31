/*
   Test of largest contiguous malloc possible. Comparing 32bit vs 64bit Pi systems,
   as well as similar 64bit systems with different sizes of RAM (i.e. 1Gb, 4Gb, 8Gb)

   In writing this, I discovered that a large allocation sometimes appeared to
   succeed as per malloc, but when it was freed, an error was reported.  This
   only happened when some other process was using a noticeable amount of the
   physical memory.  The problem went away with smaller allocations, and some
   experimentation showed that the /proc/meminfo parameter "MemFree" was the
   critical limit.  
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(int argc, char **argv) {
  char *block = NULL;
  unsigned long long int Attempt = 8LL * 1024LL * 1024LL * 1024LL;
  unsigned long long int Half    = 4LL * 1024LL * 1024LL * 1024LL;
  unsigned long long int Best    = 0LL;
  unsigned long long int CalculatedBest = 0LL;
  unsigned long long int Initial = 0LL;
  unsigned long long int LL;
  clock_t BEFORE, AFTER, DURATION;
  
  FILE *meminfo = NULL;

  // First we check for a hint re a good starting point for largest allocation.
  meminfo = fopen("/proc/meminfo", "r");
  if (meminfo) {
    char line[1024];
    for (;;) {
      // later use   int fscanf(FILE *stream, const char *format, ...);
      char *s = fgets(line, 1024, meminfo);
      if (!s) break;
      if (strncmp(line, "MemAvailable:", strlen("MemAvailable:")) == 0) {
        if ((s=strstr(line, "kB")) != NULL) {
          *s = '\0';
        } else continue;
        Initial = Attempt = atoll(line+strlen("MemAvailable:")) * 1024LL;
        Half = Attempt/2LL;
        fprintf(stderr, "Using starting free mem estimate of %llu derived from /proc/meminfo\n", Attempt);
      }
    }
  }

  // A binary chop search finds the largest contiguous allocatable block.
  for (;;) {

    if (Half <= 1024LL) {
      break;
    }
    fprintf(stderr, "malloc(%llu): ", Attempt);
    block = malloc(Attempt);
    fprintf(stderr, "%s\n", block ? "OK" : "Failed");
    if (block) {
      Best = Attempt;
      block[0] = 42; block[Attempt-1] = 17;
      free(block);

      // Successful.  Can we increase a little?
      Attempt += Half; Half /= 2LL;

    } else {

      // Unsuccessful.  Decrease.
      Attempt -= Half; Half /= 2LL;

    }

  }

  // Reduce the requested allocation by a smidgeon to leave some room for other processes etc.
  fprintf(stderr, "Largest allocation was %llu (approx %llu Mb", Best, Best/(1024LL*1024LL));
  if (Best/(1024LL*1024LL) > 1) {
    fprintf(stderr, ", or roughly %.2g Gb", (double)Best/(1024.0*1024.0*1024.0));
  }
  fprintf(stderr, ")\n");
  CalculatedBest = Best;
  if (Best > Initial) {
    fprintf(stderr, "However, since this is larger than the recommendation from /proc/meminfo,\n");
    fprintf(stderr, "I'm going to reduce it to %llu (approx %llu Mb", Initial, Initial/(1024LL*1024LL));
    if (Initial/(1024LL*1024LL) > 1) {
      fprintf(stderr, ", or roughly %.2g Gb", (double)Initial/(1024.0*1024.0*1024.0));
    }
    fprintf(stderr, ")\n");
    Best = Initial;
  }
  Best = ((Best * 15LL) / 16LL) / (1024LL*1024LL);
  fprintf(stderr, "I recommend asking for %lluMb for your array to ensure it does not cause swapping.\n", Best);
  block = malloc(Best * 1024LL * 1024LL);
  if (!block) {
    fprintf(stderr, "The 'safe' malloc size actually failed.  Problems with fragmentation in the malloc package?\n");
    exit(EXIT_FAILURE);
  }
  BEFORE = clock();
  for (LL = 0; LL < Best * 1024LL * 1024LL; LL++) {
    block[LL] = (char)(LL&255LL);
  }
  for (LL = 0; LL < Best * 1024LL * 1024LL; LL++) {
    if (block[LL] != (char)(LL&255LL)) {
      fprintf(stderr, "Memory error at offset %llu\n", LL); exit(EXIT_FAILURE);
    }
  }
  AFTER = clock();
  DURATION = AFTER-BEFORE;
  fprintf(stderr, "Memory seems OK on a quick test.  Time=%.3g Secs\n", (double)DURATION/(double)CLOCKS_PER_SEC);
  fprintf(stderr, "Memory was accessed at roughly %.3g Mb/Secs  (a Pi4 should come in around 64Mb/s, or 320Mb/s if compiled -Ofast)\n", ((double)Best)/((double)DURATION/(double)CLOCKS_PER_SEC));
  free(block);

  Best = CalculatedBest / (1024LL*1024LL);
  fprintf(stderr, "BUT... let's compare the speed with the larger calculated size of %lluMb...\n", Best);
  block = malloc(Best * 1024LL * 1024LL);
  if (!block) {
    fprintf(stderr, "The larger malloc size actually failed.\n");
    exit(EXIT_FAILURE);
  }
  BEFORE = clock();
  fprintf(stderr, "There is a possibility that this test may trigger an 'Out of memory' error.\n");
  fprintf(stderr, "which you'll be able to see by typing: dmesg|tail\n");
  // [15284.564856] Out of memory: Killed process 2333 (m) total-vm:8103832kB, anon-rss:7621040kB, file-rss:0kB, shmem-rss:0kB, UID:1000 pgtables:14956kB oom_score_adj:0

  for (LL = 0; LL < Best * 1024LL * 1024LL; LL++) {
    block[LL] = (char)(LL&255LL);
  }
  for (LL = 0; LL < Best * 1024LL * 1024LL; LL++) {
    if (block[LL] != (char)(LL&255LL)) {
      fprintf(stderr, "Memory error at offset %llu\n", LL); exit(EXIT_FAILURE);
    }
  }
  AFTER = clock();
  DURATION = AFTER-BEFORE;
  fprintf(stderr, "Memory was accessed at roughly %.3g Mb/Secs\n", ((double)Best)/((double)DURATION/(double)CLOCKS_PER_SEC));
  exit(EXIT_SUCCESS);
  return EXIT_FAILURE;
}
