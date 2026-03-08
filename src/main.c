#include <stdio.h>

typedef struct
{
   unsigned int max_age;
   unsigned int min_size;
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
   p->max_age  = 1;
   p->min_size = 1;
   p->tmpdir   = "/tmp/";
   p->cachedir = "~/.cache/";
}

int
main (void)
{
   policy_t p = { 0 };
   policy_init (&p);

   printf ("p.max_age	-->  %d\n", p.max_age);
   printf ("p.min_size	-->  %d\n", p.min_size);
   printf ("p.tmpdir	-->  %s\n", p.tmpdir);
   printf ("p.cachedir	-->  %s\n", p.cachedir);

   return 0;
}
