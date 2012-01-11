//
// alvara.cpp
//
// COPYRIGHT (C) 2011 AND ALL RIGHTS RESERVED BY
// MARC WEIDLER, ULRICHSTR. 12/1, 71672 MARBACH, GERMANY (MARC.WEIDLER@WEB.DE).
//
// ALL RIGHTS RESERVED. THIS SOFTWARE AND RELATED DOCUMENTATION ARE PROTECTED BY
// COPYRIGHT RESTRICTING ITS USE, COPYING, DISTRIBUTION, AND DECOMPILATION. NO PART
// OF THIS PRODUCT OR RELATED DOCUMENTATION MAY BE REPRODUCED IN ANY FORM BY ANY
// MEANS WITHOUT PRIOR WRITTEN AUTHORIZATION OF MARC WEIDLER OR HIS PARTNERS, IF ANY.
// UNLESS OTHERWISE ARRANGED, THIRD PARTIES MAY NOT HAVE ACCESS TO THIS PRODUCT OR
// RELATED DOCUMENTATION. SEE LICENSE FILE FOR DETAILS.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE. THE ENTIRE RISK AS TO THE QUALITY AND
// PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
// YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.


#include "Alvara.hpp"
#include "version.h"
#include <string.h>
#include <iostream>
#include <getopt.h>

using namespace std;

#ifndef ALVARA_VERSION
  #define ALVARA_VERSION ""
#endif


static int create_flag= 0;
static int verify_flag= 0;
static int verbose_flag= VERBOSITY_INFO;
static int help_flag= 0;

static struct option long_options[] =
{
  // These options set a flag.
  { "create",  no_argument,       &create_flag,  1 },
  { "verify",  no_argument,       &verify_flag,  1 },
  { "quiet",   no_argument,       &verbose_flag, VERBOSITY_QUIET    },
  { "progress",no_argument,       &verbose_flag, VERBOSITY_PROGRESS },
  { "help",    no_argument,       &help_flag,    1 },
  { "usage",   no_argument,       &help_flag,    1 },
        
  // These options don't set a flag, we distinguish them by their indices.
  { "file",    required_argument, 0, 'f' },
  { "exclude", required_argument, 0, 'e' },
  { "ignore",  required_argument, 0, 'i' },
  { 0, 0, 0, 0 }
};


/*****************************************************************************
 * Print usage information to the console.
 *****************************************************************************/
void usage(const char *prgname)
{
  cout << prgname << " " << ALVARA_VERSION << " - file integrity verification.\n";
  cout << "Usage: " << prgname << " <options> <file-or-path> [<file-or-path>] ...\n";
  cout << "where <options> start at least with one '-' and can be:\n";
  cout << "   --create,   -c or c: Create a new reference file with reference information.\n";
  cout << "   --verify,   -v or v: Verify the given directories or files against previously created reference information.\n";
  cout << "   --file,     -f or f: Read or write reference information from this file.\n";
  cout << "   --exclude,  -e or e: Exclude path/file from verification.\n";
  cout << "   --ignore,   -i or i: Ignore differences of (c)ontent, (s)ize, (m)odification, (f)lags, (d)eletion, (a)dded.\n";
  cout << "   --quiet,    -q or q: Be quiet. Print only detected differences.\n";
  cout << "   --progress, -p or p: Show progress and print as many information as possible.\n";
  cout << "   --help,     -h or h: Print this command usage.\n";
  cout << "   --usage,    -u or u: Print this command usage.\n";
  cout << "<file-or-path> filename or a directory name to investitage.\n";
  cout << "\n";
  cout << "Examples:\n";
  cout << "1. Create reference file 'reference' with contents of directory '/media/data' and '/media/music':\n";
  cout << "     " << prgname << " -cf ~/reference /media/data /media/music\n";
  cout << "2. Verify the obove created reference to the current content:\n";
  cout << "     " << prgname << " --verify --file=~/reference /media/data /media/music\n";
  cout << "3. Create reference of home directory, but ignore '.ssh' and '.dbus' directories:\n";
  cout << "     " << prgname << " --create --file=/data/ref --exclude='.ssh' --exclude='.dbus' ~\n";

}


/********************************************************************************************
 * Main function:
 *
 * TODO
 *
 *******************************************************************************************/ 
int main (int argc, char *argv[])
{
  int rc= RC_OK;
  int optcode= 0;
  Alvara alvara;
  const char *reffilename= NULL;
  
  // ensure large file support
  struct stat meta;
  if (sizeof(meta.st_size) < 8)
  {
    cout << "ATTENTION: " << argv[0] << " was compiled without support for large files.\n";
    cout << "           Files larger than 2 GB can not be handled. This program is not usable.\n";
    return RC_ERROR;
  }

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
    int option_index = 0;
     
    optcode = getopt_long_only(argc, argv, "cvqphuf:x:i:", long_options, &option_index);
     
    /* Detect the end of the options. */
    if (optcode == -1)
      break;
     
    switch (optcode)
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
	   
        cout << "option " << long_options[option_index].name;
        if (optarg)
             cout << " with arg " << optarg;
        cout << "\n";
        break;
     
      case 'q':
        verbose_flag= VERBOSITY_QUIET;
        break;

      case 'p':
        verbose_flag= VERBOSITY_PROGRESS;
        break;
     
      case 'c':
        create_flag= 1;
        break;
     
      case 'v':
        verify_flag= 1;
        break;
     
      case 'f':
        reffilename= optarg;
        break;

      case 'u':
      case 'h':
        help_flag=1;
        break;

      case '?':
       /* getopt_long already printed an error message. */
       break;
     
      default:
       cout << argv[0] << ": program bug " << optcode << "\n";
       return RC_ERROR;
    }
  }

  if (help_flag)
  {
    usage(argv[0]);
    return RC_OK;
  }
     
  if ((create_flag == 0) && (verify_flag == 0))
  {
    cout << argv[0] << ": command '--create' or '--verify' is mandatory\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  if (create_flag && verify_flag)
  {
    cout << argv[0] << ": choose either --create or --verify\n";
    cout << "Try " << argv[0] << " -h or --help for more information.\n";
    return RC_ERROR;
  }

  if (!reffilename)
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

  alvara.SetVerbosity(verbose_flag);

  // Generate content list...
  while (optind < argc)
  {
    // do not resolve path names; relative paths must be possible!
    string basedir(argv[optind++]);
    alvara.Create(basedir);
  }


  // and hashes.
  rc|= alvara.computeHashes();

  if (create_flag)
  {
    rc|= alvara.writeReference(reffilename);
  }
  if (verify_flag)
  {
    rc|= alvara.validateContent(reffilename);
  }

  return rc;
}

