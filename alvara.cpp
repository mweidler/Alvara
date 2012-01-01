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


#include "ContentList.hpp"
#include "sha1.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>

using namespace std;

#define HASH_LENGTH 20
#define RC_OK       0
#define RC_MODIFIED 1
#define RC_DELETED  2
#define RC_ADDED    4
#define RC_ERROR    16


/*****************************************************************************
 * Validate all entries in the reference list against the actual content list.
 *****************************************************************************/
int validate(ContentList &referenceList, ContentList &contentList)
{
  int rc= RC_OK;

  for (ContentListIterator iter= referenceList.begin(); iter != referenceList.end(); iter++)
  {
    ContentEntry *reference = iter->second;
    ContentEntry *content = contentList.Find(iter->first);

    if (content)
    {
      if (reference->sha1 != content->sha1)
      {
        cout << "'" << iter->first << "' has different content.\n";
        rc|= RC_MODIFIED;
      }
      if (reference->meta.st_size != content->meta.st_size)
      {
        cout << "'" << iter->first << "' has different size.\n";
        rc|= RC_MODIFIED;
      }
      if (reference->meta.st_mtime != content->meta.st_mtime)
      {
        cout << "'" << iter->first << "' has different modification time.\n";
        rc|= RC_MODIFIED;
      }
      if (reference->meta.st_mode != content->meta.st_mode)
      {
        cout << "'" << iter->first << "' has different file mode flags.\n";
        rc|= RC_MODIFIED;
      }

      contentList.erase(iter->first);
    }
    else
    {
      cout << "'" << iter->first << "' has been deleted.\n";
      rc|= RC_DELETED;
    }
  }


  // all remaining entries in the content list were not in the reference list
  // and therefore newly created/added.
  for (ContentListIterator iter= contentList.begin(); iter != contentList.end(); iter++)
  {
    cout << "'" << iter->first << "' has been added.\n";
    rc|= RC_ADDED;
  }

  return rc;
}


/*****************************************************************************
 * Compute all hashes for files in the list.
 *****************************************************************************/
int computeHashes(ContentList &contentList)
{
  unsigned char sha1_output[HASH_LENGTH];
  char bytehex[2+1];
  int  rc= RC_OK;
  int  n=1;
  
  for (ContentListIterator iter= contentList.begin(); iter != contentList.end(); iter++, n++)
  {
    ContentEntry *entry= iter->second;
    entry->sha1.clear();
    
    cout << "\rGenerating hashes... (" << n << "/" << contentList.size() << ")   " << flush;
	
    if (S_ISREG(entry->meta.st_mode))
    {
      if (sha1_file( iter->first.c_str(), sha1_output) == 0)
      {
        for (int i = 0; i < HASH_LENGTH; i++)
        {
          snprintf(bytehex, sizeof(bytehex), "%02x", sha1_output[i]);
          entry->sha1.append(bytehex);
        }
      }
      else
      {
        entry->sha1.append("no hash (");
        entry->sha1.append(strerror(errno));
        entry->sha1.append(")");
        cerr << "Error on creating hash for '" << iter->first << "' (" << strerror(errno) << ").\n";
        rc= RC_ERROR;
      }
    }
    else
    {
      entry->sha1.append("no hash (Not a file)");
    }

    entry->sha1.resize(2*HASH_LENGTH, '-');
  }
  
  if (contentList.size() > 0)
  {
    cout << "\n";
  }

  return rc;
}


/*****************************************************************************
 * 
 *****************************************************************************/
int writeReference(const char *filename, ContentList &contentList)
{
  int rc= RC_OK;
  ofstream outputfile;

  outputfile.open(filename, ios::out);
  if (outputfile.is_open())
  {
    cout << "Writing reference file '" << filename << "'..." << flush;
    contentList.Dump(outputfile);
    cout << " done.\n";
  }
  else
  {
    cerr << "Could not create reference file '" << filename << "'.\n";
    rc|= RC_ERROR;
  }
  
  return rc;
}


/*****************************************************************************
 * 
 *****************************************************************************/
int validateContent(const char *filename, ContentList &contentList)
{
  int rc= RC_OK;
  ifstream inputfile;

  inputfile.open(filename, ios::in);
  if (inputfile.is_open())
  {
    ContentList referenceList;
    cout << "Reading reference file '" << filename << "'..." << flush;
    referenceList.Import(inputfile);
    cout << " done.\n";

    cout << "Validating content...\n";
    rc|= validate(referenceList, contentList);
    if (rc == 0)
    {
       cout << "File integrity validated, no modifications detected.\n";
    }
  }
  else
  {
    cerr << "Could not open reference file '" << filename << "'.\n";
    rc|= RC_ERROR;
  }
  
  return rc;
}


/*****************************************************************************
 * Print usage information to the console.
 *****************************************************************************/
void usage(const char *prgname)
{
  cout << prgname << ", file integrity validation.\n";
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
  ContentList contentList;
  for (int argn = 3; argn < argc; argn++)
  {
    // do not resolve path names; relative paths must be possible!
    string basedir(argv[argn]);
    cout << "Generating content list for '" << basedir << "'..." << flush;
    contentList.Create(basedir);
    cout << " done.\n";
  }


  // and hashes.
  rc|= computeHashes(contentList);

  if (mode == 1)
  {
    rc|= writeReference(argv[2], contentList);
  }
  else if (mode == 2)
  {
    rc|= validateContent(argv[2], contentList);
  }

  return rc;
}
