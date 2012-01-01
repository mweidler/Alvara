//
// ContentList.cpp
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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

using namespace std;


/*****************************************************************************
 * Constructor
 *****************************************************************************/
ContentList::ContentList()
{
  clear();
}


/*****************************************************************************
 * Destructor
 *****************************************************************************/
ContentList::~ContentList()
{
  clear();
}


/*****************************************************************************
 * Empty the entier map.
 *****************************************************************************/
void ContentList::clear()
{
  for (ContentListIterator iter= this->begin(); iter != this->end(); iter++)
  {
    delete iter->second;
  }
  
  BaseContentList::clear();
}


/*****************************************************************************
 * Deletes the name and meta information from the map.
 *****************************************************************************/
void ContentList::erase(string val)
{
  delete this->at(val);
  BaseContentList::erase(val);
}


/*****************************************************************************
 * Finds a existing file (and meta) in the list and returns the data,
 * or otherwise NULL.
 *****************************************************************************/
ContentEntry *ContentList::Find(string val)
{
  if (this->find(val) == this->end())
  {
    return NULL;
  }
  else
  {
    return this->at(val);
  }
}


/*****************************************************************************
 * Writes time,sha1,size,... for later comparison to a file.
 *****************************************************************************/
void ContentList::Dump(ostream &outputfile)
{
  for (ContentListIterator iter= this->begin(); iter != this->end(); iter++)
  {
    const string &name= iter->first;
    ContentEntry *entry= iter->second;

    outputfile << entry->meta.st_mode  << ";"
               << entry->meta.st_mtime << ";"
               << entry->sha1          << ";"
               << name                 << ";"
               << entry->meta.st_size  << "\n";
  }
}


/*****************************************************************************
 * Reads the time,sha1,size,... for reference information from a file.
 *****************************************************************************/
void ContentList::Import(istream &inputfile)
{
  string mode;
  string mtime;
  string name;
  string size;
  
  while(!std::getline(inputfile, mode, ';').eof())
  {
    ContentEntry *entry= new ContentEntry();

    std::getline(inputfile, mtime, ';');       // read thru simicolon
    std::getline(inputfile, entry->sha1, ';'); // read thru simicolon
    std::getline(inputfile, name, ';');        // read thru simicolon
    std::getline(inputfile, size);             // read thru newline

    entry->meta.st_mode  = (mode_t)atol(mode.c_str());
    entry->meta.st_mtime = (time_t)atol(mtime.c_str());
    entry->meta.st_size  = (off_t)atol(size.c_str());

    this->insert(pair<string,ContentEntry *>(name,entry));
  }
}


/*****************************************************************************
 * Read a single file or contents of a directory recursively.
 *****************************************************************************/
void ContentList::Create(string &basedir)
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
  
  if (basedir != ".." && basedir != ".")
  {
    this->insert(pair<string,ContentEntry *>(basedir,entry));
  }
  else
  {
    delete entry;
  }
}


/*****************************************************************************
 * Read contents of a directory recursively.
 *****************************************************************************/
void ContentList::ReadDirectory(string &dirname)
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
      this->insert(pair<string,ContentEntry *>(nextdirname,entry));

      if (S_ISDIR(entry->meta.st_mode) && !S_ISLNK(entry->meta.st_mode))
      {
        ReadDirectory(nextdirname);
      }
    }

    closedir(pDirectory);
  }
  else
  {
    cerr << "Error during opening directory '" << dirname << "'.\n";
  }
}
