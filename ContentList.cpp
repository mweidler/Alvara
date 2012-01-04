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
#include <iostream>

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

