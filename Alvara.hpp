/*
 * Alvara.hpp
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

#ifndef HEADER_ALVARA_INC
#define HEADER_ALVARA_INC

#include "ContentList.hpp"
#include <list>

#define RC_OK       0
#define RC_MODIFIED 1
#define RC_DELETED  2
#define RC_ADDED    4
#define RC_ERROR    16

#define HASH_LENGTH 20

#define VERBOSITY_QUIET    0
#define VERBOSITY_INFO     1
#define VERBOSITY_PROGRESS 2

#define IGNORE_CONTENT     1
#define IGNORE_SIZE        2
#define IGNORE_TIME        4
#define IGNORE_FLAGS       8
#define IGNORE_DELETION   16
#define IGNORE_ADDED      32

using namespace std;


/*****************************************************************************
 * Class definition for Alvara
 *****************************************************************************/
class Alvara
{
public:
  void Scan(int prefix, string &basedir);
  int  ComputeHashes();
  int  VerifyContent();

  int  WriteReference(const char *filename);
  int  ReadReference(const char *filename);

  void SetVerbosity(int verbosity);
  void SetIgnorance(int ignorance);
  void AddExclude(const char *exclude);
  
protected:
  bool isExcluded(const char *filename);
  void ReadDirectory(string &basedir, string &key);

private:
  ContentList referenceList;
  ContentList contentList;

  int verbosity;
  int ignorance;
  list<string> excludesList;
};

#endif // ! HEADER_ALVARA_INC
