/*
 * alvara_main.cpp
 *
 * COPYRIGHT (C) 2012 AND ALL RIGHTS RESERVED BY
 * MARC WEIDLER, ULRICHSTR. 12/1, 71672 MARBACH, GERMANY (MARC.WEIDLER@WEB.DE).
 *
 * ALL RIGHTS RESERVED. THIS SOFTWARE AND RELATED DOCUMENTATION ARE PROTECTED BY
 * COPYRIGHT RESTRICTING ITS USE, COPYING, DISTRIBUTION, AND DECOMPILATION. NO PART
 * OF THIS PRODUCT OR RELATED DOCUMENTATION MAY BE REPRODUCED IN ANY FORM BY ANY
 * MEANS WITHOUT PRIOR WRITTEN AUTHORIZATION OF MARC WEIDLER OR HIS PARTNERS, IF ANY.
 * UNLESS OTHERWISE ARRANGED, THIRD PARTIES MAY NOT HAVE ACCESS TO THIS PRODUCT OR
 * RELATED DOCUMENTATION. SEE LICENSE FILE FOR DETAILS.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE. THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
 * YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 */

#include "Alvara.hpp"
#include "version.h"
#include <string.h>
#include <iostream>
#include <getopt.h>

using namespace std;

#define COMMAND_NONE   0
#define COMMAND_CREATE 1
#define COMMAND_VERIFY 2
#define COMMAND_BOTH   (COMMAND_CREATE|COMMAND_VERIFY)

static int command=  COMMAND_NONE;
static int command2= COMMAND_NONE;
static int verbosity= VERBOSITY_INFO;
static int showhelp= 0;
static int showversion= 0;
static int ignorance= 0;

static char alvara_version[128];

/* options map for getopt() */
static struct option long_options[]=
{
  // These options set a flag.
  { "create",  no_argument, &command,      COMMAND_CREATE     },
  { "verify",  no_argument, &command2,     COMMAND_VERIFY     },
  { "quiet",   no_argument, &verbosity,    VERBOSITY_QUIET    },
  { "progress",no_argument, &verbosity,    VERBOSITY_PROGRESS },
  { "version", no_argument, &showversion,  1                  },
  { "help",    no_argument, &showhelp,     1                  },
  { "usage",   no_argument, &showhelp,     1                  },
        
  // These options don't set a flag, we distinguish them by their indices.
  { "file",    required_argument, 0, 'f' },
  { "exclude", required_argument, 0, 'e' },
  { "ignore",  required_argument, 0, 'i' },
  { 0, 0, 0, 0 }
};


/********************************************************************************************
 * Print usage information to the console.
 *******************************************************************************************/ 
static void usage(const char *prgname)
{
  cout << prgname << " " << alvara_version << " - file integrity verification.\n"

  "Usage: " << prgname << " <options> <file-or-path> [<file-or-path>] ...\n"
  "where <options> start at least with one '-' and can be\n"
  "   --create, -c or c          Create a new reference file with reference information of the given files or directories.\n"
  "   --verify, -v or v          Verify the given files or directories against previously created reference information.\n"
  "   --file=REFERENCE, -f or f  Read or write reference information using file REFERENCE.\n"
  "   --exclude=PATTERN, -e or e Gives you a way to avoid processing files/directories, if they match the PATTERN.\n"
  "                              The whole relative file path is compared against the PATTERN.\n"
  "   --ignore=IGNORE, -i or i   Ignore differences of content.\n"
  "                              You can IGNORE one or multiple of: (c)ontent,(s)ize,(t)ime,(f)lags,(d)eletion,(a)dded.\n"
  "                              Use long (e.g. 'content') or short form (e.g. 'c'), separated with or without comma.\n"
  "   --quiet, -q or q           Be quiet. Print only detected differences.\n"
  "   --progress, -p or p        Show progress and print as many information as possible.\n"
  "   --version                  Print version info and exit.\n"
  "   --help, -h or h            Prints the synopsis and a list of options and exists.\n"
  "   --usage, -u or u           Same as '--help'.\n"
  "and <file-or-path> specifies filename or a directory name to investitage.\n"
  "\n"
  "Return values are\n"
  "   0  no differences were detected\n"
  "   1  content, size, time or flags have been changed\n"
  "   2  files were deleted\n"
  "   4  files were added\n"
  "  16  error occured\n"
  "\n"
  "Examples:\n"
  "   Create reference file 'reference' with contents of directory '/media/data' and '/media/music':\n"
  "     " << prgname << " -cf ~/reference /media/data /media/music\n\n"
  "   Verify the above created reference against the current content, but ignore file's timestamp:\n"
  "     " << prgname << " --verify --file=~/reference --ignore=time /media/data /media/music\n\n"
  "   Create reference of home directory, but exclude '.ssh' and '.dbus' directories:\n"
  "     " << prgname << " --create --file=/data/ref --exclude='.ssh' --exclude=.dbus ~\n";
}


/********************************************************************************************
 * Finds the needle in haystack and removes all occurencies.
 * Return: 1 if needle found, otherwise 0
 *******************************************************************************************/ 
static int findAndWipe(const char *haystack, const char *needle)
{
  int found= 0;
  char *p;

  while ((p=(char *)strstr(haystack, needle)) != NULL)
  {
     strncpy(p, p+strlen(needle), strlen(p));
     found= 1;
  }

  return found;
}


/********************************************************************************************
 *
 * Main function
 *
 *******************************************************************************************/ 
int main (int argc, char *argv[])
{
  int rc= RC_OK;
  Alvara alvara;
  const char *ref_filename= NULL;

  strcpy(alvara_version, (strlen(COMMIT_VERSION)>0)?COMMIT_VERSION:ALVARA_VERSION);
   
  // ensure large file support
  struct stat meta;
  if (sizeof(meta.st_size) < 8)
  {
    cout << "ATTENTION: " << argv[0] << " was compiled without support for large files.\n";
    cout << "           Files larger than 2 GB can not be handled. This program is not usable.\n";
    return RC_ERROR;
  }

  // Now check and parse parameters...
  if (argc == 2 && (strcmp(argv[1], "--version") == 0))
  {
    cout << argv[0] << " version " << alvara_version << "\n";
    return RC_OK;
  }

  // Now check and parse parameters...
  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
  {
    usage(argv[0]);
    return RC_OK;
  }

  if (argc < 3)
  {
    cout << argv[0] << ": missing parameter\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  while (1)
  {
    /* getopt_long stores the option index here. */
    int option_index= 0;
    int optcode= getopt_long_only(argc, argv, "cvqphuf:x:i:", long_options, &option_index);
     
    /* Detect the end of the options. */
    if (optcode == -1)
      break;
     
    switch (optcode)
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;

        cout << argv[0] << ": program bug " << optcode << "\n";
        return RC_ERROR;       
        break;
	       
      case 'q':
        verbosity= VERBOSITY_QUIET;
        break;

      case 'p':
        verbosity= VERBOSITY_PROGRESS;
        break;
     
      case 'c':
        command|= COMMAND_CREATE;
        break;
     
      case 'v':
        command|= COMMAND_VERIFY;
        break;
     
      case 'f':
        ref_filename= optarg;
        break;

      case 'e':
        alvara.AddExclude(optarg);
        break;

      case 'i':
        if (findAndWipe(optarg, "content"))  ignorance |= IGNORE_CONTENT;
        if (findAndWipe(optarg, "size"))     ignorance |= IGNORE_SIZE;
        if (findAndWipe(optarg, "time"))     ignorance |= IGNORE_TIME;
        if (findAndWipe(optarg, "flags"))    ignorance |= IGNORE_FLAGS;
        if (findAndWipe(optarg, "deletion")) ignorance |= IGNORE_DELETION;
        if (findAndWipe(optarg, "added"))    ignorance |= IGNORE_ADDED;
        if (findAndWipe(optarg, "c"))        ignorance |= IGNORE_CONTENT;
        if (findAndWipe(optarg, "s"))        ignorance |= IGNORE_SIZE;
        if (findAndWipe(optarg, "t"))        ignorance |= IGNORE_TIME;
        if (findAndWipe(optarg, "f"))        ignorance |= IGNORE_FLAGS;
        if (findAndWipe(optarg, "d"))        ignorance |= IGNORE_DELETION;
        if (findAndWipe(optarg, "a"))        ignorance |= IGNORE_ADDED;
	findAndWipe(optarg, ","); // just delete all "," characters
	if (strlen(optarg) > 0)
	{
          cout << argv[0] << ": unknown options for '--ignore': " << optarg << "\n";
          cout << "Try " << argv[0] << " -h or --help for more information.\n";
          return RC_ERROR;
	}
        break;

      case 'u':
      case 'h':
        showhelp= 1;
        break;

      case '?':
       /* getopt_long already printed an error message. */
       cout << "Try " << argv[0] << " -h or --help for more information.\n";
       return RC_ERROR;
       break;
     
      default:
       cout << argv[0] << ": program bug " << optcode << "\n";
       return RC_ERROR;
    }
  }


  // Now evaluate parameters... 
  if (showhelp)
  {
    usage(argv[0]);
    return RC_OK;
  }

  if (showversion)
  {
    cout << argv[0] << " version " << alvara_version << "\n";
    return RC_OK;
  }

  command|= command2;
  if (command == COMMAND_NONE)
  {
    cout << argv[0] << ": command '--create' or '--verify' is mandatory\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  if (command == COMMAND_BOTH)
  {
    cout << argv[0] << ": choose either --create or --verify\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  if (!ref_filename)
  {
    cout << argv[0] << ": reference filename is mandatory\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  if (optind == argc)
  {
    cout << argv[0] << ": please specify at least one file or directory target\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  alvara.SetVerbosity(verbosity);
  alvara.SetIgnorance(ignorance);

  if (command == COMMAND_VERIFY)
  {
    rc= alvara.ReadReference(ref_filename);
    if (rc != RC_OK)
      return rc;
  }

  int prefix= 0;
  // Generate content list...
  while (optind < argc)
  {
    // do not resolve path names; relative paths must be possible!
    string basedir(argv[optind++]);
    alvara.Scan(prefix, basedir);
    prefix++;
  }

  // and hashes.
  rc|= alvara.ComputeHashes();

  if (command == COMMAND_CREATE)
  {
    rc|= alvara.WriteReference(ref_filename);
  }

  if (command == COMMAND_VERIFY)
  {
    rc|= alvara.VerifyContent();
  }

  return rc;
}

