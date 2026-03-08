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
   char *dir;
} policy_t;

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
strncmpci (char const *str1, char const *str2, unsigned int n)
{
   for (unsigned int i = 0; i < n; i++)
      {
         int cmp = tolower (*str1) - tolower (*str2);
         if (!*str1 || !*str2 || cmp != 0)
            {
               return cmp;
            }
         str1++, str2++;
      }

   return 0;
}

void
print_usage (const char *program_name)
{
#define popt(opt, desc) printf ("   %-45s %s\n", opt, desc)
   printf ("[USAGE] %s [FLAGS] [<DIRECTORY_PATH> <MAX_AGE_IN_DAYS> <MIN_SIZE>]\n", program_name);
   printf ("[FLAGS]\n");
   popt ("--HELP, -H, -?", "PRINT THIS HELP MESSAGE AND EXIT");
   popt ("--INFO, -I", "PRINT INFORMATION ABOUT THIS PROGRAM AND EXIT");
   popt ("--DRY-RUN, -N", "PRINTS TO STDOUT WHAT WOULD BE DELETED WITHOUT ACTUALLY UNLINKING");
   printf ("[NOTE] FLAGS ARE NOT CASE-SENSITIVE\n");
#undef popt
}

void
print_info (void)
{
#define pinfo(aspect, detail) printf ("   * %-17s %s\n", aspect, detail)
   printf ("[INFO]\n");
   printf ("   DMAN -- TODO\n");
   printf ("\n");
   pinfo ("[AUTHOR]", "vs-123 @ https://github.com/vs-123");
   pinfo ("[REPOSITORY]", "https://github.com/vs-123/dman");
   pinfo ("[LICENSE]", "GNU AFFERO GENERAL PUBLIC LICENSE VERSION 3.0 OR LATER");
#undef pinfo
}

int
main (int argc, char **argv)
{
   const char *program_name = argv[0];
   policy_t p               = { 0 };
   g_policy                 = &p;

   if (argc < 2)
      {
         print_usage (program_name);
         return 1;
      }

   if (argv[1][0] == '-')
      {
         if (strncmpci (argv[1], "-h", 2) == 0 || strncmpci (argv[1], "--help", 6) == 0)
            {
               print_usage (program_name);
               return 0;
            }
         else if (strncmpci (argv[1], "-i", 2) == 0 || strncmpci (argv[1], "--info", 6) == 0)
            {
               print_info (program_name);
               return 0;
            }
         else if (strncmpci (argv[1], "-n", 2) == 0 || strncmpci (argv[1], "--dry-run", 9) == 0)
            {
               p.should_dry_run = true;
            }
         else
            {
               fprintf (stderr, "[ERROR] UNRECOGNISED FLAG, USE --HELP\n");
               return 1;
            }
      }

   if (argc < 4)
      {
         fprintf (stderr,
                  "[ERROR] EXPECTED <DIRECTORY_PATH> <MAX_AGE_IN_DAYS> <MIN_SIZE>, USE --HELP\n");
      }

   char *dirpath;
   char *max_age_days;
   char *min_size;

   if (p.should_dry_run)
      {
         dirpath      = argv[2];
         max_age_days = argv[3];
         min_size     = argv[4];
      }
   else
      {
         dirpath      = argv[1];
         max_age_days = argv[2];
         min_size     = argv[3];
      }

   p.dir = dirpath;
   p.max_age = max_age_days;
   p.min_size = min_size;

   expand_tilde (p.dir, expanded_dir, sizeof (expanded_dir));

   printf ("[INFO] PRUNING %s...\n", expanded_dir);
   prune_run (expanded_dir);

   return 0;
}
