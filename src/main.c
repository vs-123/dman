#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct
{
   bool should_dry_run;
   size_t max_age;
   size_t min_size;
   size_t max_depth;
   char *tmpdir;
   char *cachedir;
} policy_t;

void
policy_init (policy_t *p)
{
   /**
    * MAKE THIS CONFIGURABLE LATER
    * DETERMINE =tmpdir= AND =cachedir= DYNAMICALLY LATER
    */
   p->should_dry_run = true;
   p->max_age        = 1;
   p->min_size       = 1;
   p->max_depth      = 5;
   p->tmpdir         = "/tmp/";
   p->cachedir       = "~/.cache/";
}

int
main (void)
{
   policy_t p = { 0 };
   policy_init (&p);

   return 0;
}
