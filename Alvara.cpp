//
// Alvara.cpp
//
// COPYRIGHT (C) 2012 AND ALL RIGHTS RESERVED BY
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
#include "StreamPersistence.hpp"
#include "sha1.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>

#define INFO     if (verbosity >= VERBOSITY_INFO)
#define PROGRESS if (verbosity >= VERBOSITY_PROGRESS)

using namespace std;


void Alvara::SetVerbosity(int verbosity)
{
  this->verbosity= verbosity;
}


void Alvara::SetIgnorance(int ignorance)
{
  this->ignorance= ignorance;
}


void Alvara::AddExclude(const char *exclude)
{
  excludesList.push_back(exclude);
}


bool Alvara::isExcluded(const char *filename)
{
  for (list<string>::iterator iter=excludesList.begin(); iter != excludesList.end(); iter++)
  {
    if (fnmatch(iter->c_str(), filename, FNM_FILE_NAME) == 0)
      return true;
  }
  
  return false;
}

  
/*****************************************************************************
 * Validate all entries in the reference list against the actual content list.
 *****************************************************************************/
int Alvara::Validate()
{
  int rc= RC_OK;

  for (ContentListIterator iter= referenceList.begin(); iter != referenceList.end(); iter++)
  {
    ContentEntry *reference = iter->second;
    ContentEntry *content = contentList.Find(iter->first);

    if (content)
    {
      if (reference->sha1 != content->sha1 && !(ignorance & IGNORE_CONTENT))
      {
        cout << "'" << iter->first << "' has different content.\n";
        rc|= RC_MODIFIED;
      }
      if (reference->meta.st_size != content->meta.st_size && !(ignorance & IGNORE_SIZE))
      {
        cout << "'" << iter->first << "' has different size.\n";
        rc|= RC_MODIFIED;
      }
      if (reference->meta.st_mtime != content->meta.st_mtime && !(ignorance & IGNORE_TIME))
      {
        cout << "'" << iter->first << "' has different modification time.\n";
        rc|= RC_MODIFIED;
      }
      if (reference->meta.st_mode != content->meta.st_mode && !(ignorance & IGNORE_FLAGS))
      {
        cout << "'" << iter->first << "' has different file mode flags.\n";
        rc|= RC_MODIFIED;
      }

      contentList.erase(iter->first);
    }
    else if (!(ignorance & IGNORE_DELETION))
    {
      cout << "'" << iter->first << "' has been deleted.\n";
      rc|= RC_DELETED;
    }
  }


  // all remaining entries in the content list were not in the reference list
  // and therefore newly created/added.
  if (!(ignorance & IGNORE_ADDED))
  {
    for (ContentListIterator iter= contentList.begin(); iter != contentList.end(); iter++)
    {
      cout << "'" << iter->first << "' has been added.\n";
      rc|= RC_ADDED;
    }
  }

  return rc;
}


/*****************************************************************************
 * Compute all hashes for files in the list.
 *****************************************************************************/
int Alvara::ComputeHashes()
{
  unsigned char sha1_output[HASH_LENGTH];
  char bytehex[2+1];
  int  rc= RC_OK;
  int  n=1;

  if (ignorance & IGNORE_CONTENT)
    return RC_OK;

  
  for (ContentListIterator iter= contentList.begin(); iter != contentList.end(); iter++, n++)
  {
    ContentEntry *entry= iter->second;
    entry->sha1.clear();
    
    PROGRESS cout << "\rGenerating hashes... (" << n << "/" << contentList.size() << ")   " << flush;
	
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
        cerr << "\nError on creating hash for '" << iter->first << "' (" << strerror(errno) << ").\n" << flush;
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
    PROGRESS cout << "\n";
  }

  return rc;
}


/*****************************************************************************
 * 
 *****************************************************************************/
int Alvara::WriteReference(const char *filename)
{
  int rc= RC_OK;
  ofstream outputfile;

  outputfile.open(filename, ios::out);
  if (outputfile.is_open())
  {
    PROGRESS cout << "Writing reference file '" << filename << "'..." << flush;
    StreamPersistence::Save(contentList, outputfile);
    PROGRESS cout << " done.\n";
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
int Alvara::VerifyContent(const char *filename)
{
  int rc= RC_OK;
  ifstream inputfile;

  inputfile.open(filename, ios::in);
  if (inputfile.is_open())
  {
    PROGRESS cout << "Reading reference file '" << filename << "'..." << flush;
    StreamPersistence::Load(referenceList, inputfile);
    PROGRESS cout << " done.\n";

    PROGRESS cout << "Verifying content...\n";
    rc|= Validate();
    if (rc == 0)
    {
       INFO cout << "No differences detected.\n";
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
 * Read contents of a directory recursively.
 *****************************************************************************/
void Alvara::ReadDirectory(string &dirname)
{
  struct dirent *pDirEntry;
  string nextdirname;
  
  DIR *pDirectory= opendir(dirname.c_str());
  if (pDirectory)
  {
    while ((pDirEntry= readdir(pDirectory)))
    {
      if (strcmp(pDirEntry->d_name, "..") == 0 || strcmp(pDirEntry->d_name, ".") == 0)
        continue;

      if (isExcluded(pDirEntry->d_name))
        continue;

      ContentEntry *entry= new ContentEntry();

      nextdirname= dirname;
      nextdirname.append("/");
      nextdirname.append(pDirEntry->d_name);
      lstat(nextdirname.c_str(), &entry->meta);
      
      // directory size may not be compared, it's size depends on the relative path length.
      // Thus, only files get a valid/comparable size meta info.
      if (!S_ISREG(entry->meta.st_mode))
      {
        entry->meta.st_size= 0;
      }
      contentList.insert(pair<string,ContentEntry *>(nextdirname,entry));

      if (S_ISDIR(entry->meta.st_mode) && !S_ISLNK(entry->meta.st_mode))
      {
        ReadDirectory(nextdirname);
      }
    }

    PROGRESS cout << "\rScanning...  (" << contentList.size() << ")       " << flush;
    closedir(pDirectory);
  }
  else
  {
    cerr << "Error during opening directory '" << dirname << "'.\n";
  }
}


/*****************************************************************************
 * Read a single file or contents of a directory recursively.
 *****************************************************************************/
void Alvara::Scan(string &basedir)
{
  ContentEntry *entry= new ContentEntry();

  lstat(basedir.c_str(), &entry->meta);

  // directory size may not be compared, it's size depends on the relative path length.
  // Thus, only files get a valid/comparable size meta info.
  if (!S_ISREG(entry->meta.st_mode))
  {
    entry->meta.st_size= 0;
  }
  if (S_ISDIR(entry->meta.st_mode) && !S_ISLNK(entry->meta.st_mode))
  {
    ReadDirectory(basedir);
  }
  
  if (basedir != ".." && basedir != "." && !isExcluded(basedir.c_str()))
  {
    contentList.insert(pair<string,ContentEntry *>(basedir,entry));
  }
  else
  {
    delete entry;
  }

  PROGRESS cout << "\rScanning '" << basedir << "'...  (" << contentList.size() << ")" << " done.\n";
}

