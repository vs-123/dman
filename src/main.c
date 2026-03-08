#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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
                       "BYTES)\n",
                       filepath, age_days, sb->st_size);
            }
         else
            {
               if (unlink (filepath) == 0)
                  {
                     printf (stdout, "[SUCCESS] DELETED '%s'\n", filepath);
                  }
               else
                  {
                     fprintf (stderr, "[WARNING] COULD NOT DELETE '%s'\n", filepath);
                  }
            }
      }

   return 0;
}

int
prune_run (const char *path)
{
   if (nftw (path, prune_worker, 16, FTW_PHYS | FTW_MOUNT) == -1)
      {
         fprintf (stderr, "[ERROR] FAILED TO ACCESS '%s' : %s\n", path, strerror (errno));
         return -1;
      }
   return 0;
}

void
expand_tilde (char *path, char *buf, size_t bufsize)
{
   if (path[0] == '~')
      {
         const char *homedir = getenv ("HOME");
         if (homedir)
            {
               snprintf (buf, bufsize, "%s%s", homedir, path + 1);
               return;
            }
      }
   
   strncpy (buf, path, bufsize);
}

int
main (void)
{
   policy_t p = { 0 };
   policy_init (&p);
   g_policy = &p;

   char expanded_tmpdir[65536];
   char expanded_cachedir[65536];

   expand_tilde (p.tmpdir, expanded_tmpdir, sizeof (expanded_tmpdir));
   expand_tilde (p.cachedir, expanded_cachedir, sizeof (expanded_cachedir));

   printf ("[INFO] PRUNING %s...\n", expanded_tmpdir);
   prune_run (expanded_tmpdir);
   
   printf ("[INFO] PRUNING %s...\n", expanded_cachedir);
   prune_run (expanded_cachedir);

   return 0;
}
