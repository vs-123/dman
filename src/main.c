#include <ctype.h>
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
   off_t min_size;
   int max_depth;
   char *dir;
} policy_t;

static policy_t *g_policy = NULL;

int
prune_worker (const char *filepath, const struct stat *sb, int typeflag,
              struct FTW *ftwbuf)
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
                     printf ("[SUCCESS] DELETED '%s'\n", filepath);
                  }
               else
                  {
                     fprintf (stderr, "[WARNING] COULD NOT DELETE '%s'\n",
                              filepath);
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
         fprintf (stderr, "[ERROR] FAILED TO ACCESS '%s' : %s\n", path,
                  strerror (errno));
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
strcmpci (char const *str1, char const *str2)
{
   for (;;)
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

typedef enum
{
   FLAG_NONE,
   FLAG_HELP,
   FLAG_INFO,
   FLAG_DRY_RUN,
   FLAG_UNKNOWN
} flag_type_t;

typedef struct
{
   const char *short_flag;
   const char *long_flag;
   flag_type_t type;
} flag_map_t;

const flag_map_t flag_table[] = { { "-h", "--help", FLAG_HELP },
                                  { "-i", "--info", FLAG_INFO },
                                  { "-n", "--dry-run", FLAG_DRY_RUN } };

void
print_usage (const char *program_name)
{
#define popt(opt, desc) printf ("   %-45s %s\n", opt, desc)
   printf (
       "[USAGE] %s [FLAGS] [<DIRECTORY_PATH> <MAX_AGE_IN_DAYS> <MIN_SIZE>]\n",
       program_name);
   printf ("[FLAGS]\n");
   popt ("--HELP, -H, -?", "PRINT THIS HELP MESSAGE AND EXIT");
   popt ("--INFO, -I", "PRINT INFORMATION ABOUT THIS PROGRAM AND EXIT");
   popt ("--DRY-RUN, -N",
         "PRINTS TO STDOUT WHAT WOULD BE DELETED WITHOUT ACTUALLY UNLINKING");
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
   pinfo ("[LICENSE]",
          "GNU AFFERO GENERAL PUBLIC LICENSE VERSION 3.0 OR LATER");
#undef pinfo
}

bool
is_valid_number (const char *str, long *result)
{
   char *endptr;
   errno    = 0;
   long val = strtol (str, &endptr, 10);

   if (*endptr != '\0' || errno != 0 || val < 0)
      {
         return false;
      }

   *result = val;
   return true;
}

flag_type_t
get_flag (const char *arg)
{
   for (size_t i = 0; i < sizeof (flag_table) / sizeof (flag_map_t); i++)
      {
         if (strcmpci (arg, flag_table[i].short_flag) == 0
             || strcmpci (arg, flag_table[i].long_flag) == 0)
            {
               return flag_table[i].type;
            }
      }
   return FLAG_UNKNOWN;
}

int
parse_arguments (int argc, char **argv, policy_t *p)
{
   int data_idx = 1;

   if (argc > 1 && argv[1][0] == '-')
      {
         flag_type_t type = get_flag (argv[1]);

         switch (type)
            {
            case FLAG_HELP:
               print_usage (argv[0]);
               return 0;
            case FLAG_INFO:
               print_info ();
               return 0;
            case FLAG_DRY_RUN:
               p->should_dry_run = true;
               data_idx          = 2;
               break;
            default:
               fprintf (stderr, "[ERROR] UNRECOGNISED FLAG, USE --HELP\n");
               return -1;
            }
      }

   if (argc < (data_idx + 3))
      {
         fprintf (stderr, "[ERROR] BAD ARGUMENTS, USE --HELP\n");
         return -1;
      }

   long age, size;

   if (!is_valid_number (argv[data_idx + 1], &age)
       || !is_valid_number (argv[data_idx + 2], &size))
      {
         fprintf (stderr, "[ERROR] <MAX_AGE_IN_DAYS> AND <MIN_SIZE> MUST BE "
                          "NON-NEGATIVE INTEGERS\n");
         return -1;
      }

   p->dir      = argv[data_idx];
   p->max_age  = age;
   p->min_size = size;

   return 1;
}

int
main (int argc, char **argv)
{
   policy_t p = { 0 };
   int result = parse_arguments (argc, argv, &p);

   if (result <= 0)
      {
         return (result == 0) ? 0 : 1;
      }

   char expanded_dir[65536];
   expand_tilde (p.dir, expanded_dir, sizeof (expanded_dir));

   g_policy = &p;

   printf ("[STATUS] PRUNING %s...\n", expanded_dir);
   prune_run (expanded_dir);

   return 0;
}
