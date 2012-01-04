//
// StreamPersistence.cpp
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


#include "StreamPersistence.hpp"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;


/*****************************************************************************
 * Writes time,sha1,size,... for later comparison to a file.
 *****************************************************************************/
void StreamPersistence::Save(ContentList &contentList, ostream &outputfile)
{
  for (ContentListIterator iter= contentList.begin(); iter != contentList.end(); iter++)
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
void StreamPersistence::Load(ContentList &contentList, istream &inputfile)
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

    entry->meta.st_mode  = (mode_t)atoll(mode.c_str());
    entry->meta.st_mtime = (time_t)atoll(mtime.c_str());
    entry->meta.st_size  = (off_t)atoll(size.c_str());

    contentList.insert(pair<string,ContentEntry *>(name,entry));
  }
}
