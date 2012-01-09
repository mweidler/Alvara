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
#include <string.h>
#include <iostream>

using namespace std;

#ifndef ALVARA_VERSION
  #define ALVARA_VERSION ""
#endif


/*****************************************************************************
 * Print usage information to the console.
 *****************************************************************************/
void usage(const char *prgname)
{
  cout << prgname << " " << ALVARA_VERSION << " - file integrity validation.\n";
  cout << "Usage: " << prgname << " <command> <reference-file> <file-or-path> [<file-or-path>] ...\n";
  cout << "where <command> can be:\n";
  cout << "   create or c:   Create a new reference file with reference information.\n";
  cout << "   validate or v: Validate the given directories or files against the previously created reference information.\n";
  cout << "<reference-file> is a filename, where to store or read the reference information.\n";
  cout << "<file-or-path> is a filename or a directory name to investitage.\n";
}


/********************************************************************************************
 * Main function:
 *
 * TODO
 *
 * create <reference-file> file1 dir1 ...
 * validate <reference-file> file1 dir1 ...
 *******************************************************************************************/ 
int main (int argc, char *argv[])
{
  int rc= RC_OK;
  int mode= 0;
  Alvara alvara;

  // validating large file support
  struct stat meta;
  if (sizeof(meta.st_size) < 8)
  {
    cout << "ATTENTION: " << argv[0] << " was compiled without support for large files.\n";
    cout << "           Checksums can not be generated on files larger 2 GB.\n";
  }


  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "help") == 0))
  {
    usage(argv[0]);
    return RC_OK;
  }

  if (argc < 4)
  {
    cout << argv[0] << ": missing parameter\n";
    cout << "Try " << argv[0] << " -h or help for more information.\n";
    return RC_ERROR;
  }

  if (strcmp(argv[1], "create") == 0 || strcmp(argv[1], "c") == 0) 
  {
    mode= 1;
  }
  else if (strcmp(argv[1], "validate") == 0 || strcmp(argv[1], "v") == 0)
  {
    mode= 2;
  }
  else
  {
    cout << argv[0] << ": missing or unknown command\n";
    cout << "Try " << argv[0] << " -h or help for mor information.\n";
    return RC_ERROR;
  }


  // Generate content list...
  for (int argn = 3; argn < argc; argn++)
  {
    // do not resolve path names; relative paths must be possible!
    string basedir(argv[argn]);
    cout << "Generating content list for '" << basedir << "'..." << flush;
    alvara.Create(basedir);
    cout << " done.\n";
  }


  // and hashes.
  rc|= alvara.computeHashes();

  if (mode == 1)
  {
    rc|= alvara.writeReference(argv[2]);
  }
  else if (mode == 2)
  {
    rc|= alvara.validateContent(argv[2]);
  }

  return rc;
}

