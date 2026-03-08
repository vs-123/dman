#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <unistd.h>

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

static policy_t *g_policy = NULL;

int
prune_worker (const char *filepath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
   if (ftwbuf->level > g_policy->max_depth)
      {
         return 0;
      }

   if (typeflag != FTW_F)
      {
         return 0;
      }

   time_t now      = time (NULL);
   double age_days = difftime (now, sb->st_mtime) / (60 * 60 * 24);

   bool is_beyond_max_age  = age_days > g_policy->max_age;
   bool is_beyond_min_size = sb->st_size > g_policy->min_size;

   if (is_beyond_max_age || is_beyond_min_size)
      {
         if (g_policy->should_dry_run)
            {
               printf ("[DRY RUN] DELETE: '%s'   (AGE: %.1f DAYS, SIZE: %lld "
                       "BYTES)",
                       filepath, age_days, sb->st_size);
            }
         else
            {
               if (unlink (filepath) == 0)
                  {
                     printf (stdout, "[SUCCESS] DELETED '%s'", filepath);
                  }
               else
                  {
                     fprintf (stderr, "[ERROR] COULD NOT DELETE '%s'", filepath);
                  }
            }
      }

   return 0;
}

int
main (void)
{
   /**
      [TODO]
      - ASSIGN =&p= TO =g_policy=
      - INVOKE =nftw= FOR BOTH =tmpdir= AND =cachedir= WITH =FTW_PHYS=
      - USE =FTW_PHYS= TO AVOID FOLLOWING SYMLINKS
    */

   return 0;
}
