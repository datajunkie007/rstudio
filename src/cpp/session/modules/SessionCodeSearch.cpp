/*
 * SessionCodeSearch.cpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

#include "SessionCodeSearch.hpp"

#include <iostream>
#include <vector>
#include <set>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <core/Error.hpp>
#include <core/Exec.hpp>
#include <core/FilePath.hpp>
#include <core/FileSerializer.hpp>
#include <core/SafeConvert.hpp>

#include <core/r_util/RSourceIndex.hpp>

#include <R_ext/rlocale.h>

#include <session/SessionUserSettings.hpp>
#include <session/SessionModuleContext.hpp>

#include <session/projects/SessionProjects.hpp>

#include "SessionSource.hpp"

using namespace core ;

namespace session {  
namespace modules {
namespace code_search {

namespace {

std::vector<boost::shared_ptr<core::r_util::RSourceIndex> > s_sourceIndexes;

class SourceFileIndexer : boost::noncopyable
{
public:
   SourceFileIndexer()
      : encoding_(projects::projectContext().defaultEncoding()),
        projectRootDir_(projects::projectContext().directory()),
        dirIter_(projectRootDir_)
   {
   }

   virtual ~SourceFileIndexer()
   {
   }

public:
   bool indexNextFile()
   {
      // see if we are finished
      if (dirIter_.finished())
         return false;

      // get next file
      FilePath filePath;
      Error error = dirIter_.next(&filePath);
      if (error)
      {
         LOG_ERROR(error);
         return false;
      }

      // index
      indexProjectFile(filePath);

      // return status
      return !dirIter_.finished();
   }

private:

   void indexProjectFile(const FilePath& filePath)
   {
      if (isRSourceFile(filePath))
      {
         // read the file
         std::string code;
         Error error = module_context::readAndDecodeFile(filePath,
                                                         encoding_,
                                                         true,
                                                         &code);
         if (error)
         {
            error.addProperty("src-file", filePath.absolutePath());
            LOG_ERROR(error);
            return;
         }

         // compute project relative directory (used for context)
         std::string context = filePath.relativePath(projectRootDir_);

         // index the source
         s_sourceIndexes.push_back(boost::shared_ptr<r_util::RSourceIndex>(
                                       new r_util::RSourceIndex(context, code)));
      }
   }

   static bool isRSourceFile(const FilePath& filePath)
   {
      return !filePath.isDirectory() && (filePath.extensionLowerCase() == ".r");
   }

private:
   std::string encoding_;
   FilePath projectRootDir_;
   RecursiveDirectoryIterator dirIter_;
};

void indexProjectFiles()
{
   // create indexer
   boost::shared_ptr<SourceFileIndexer> pIndexer(new SourceFileIndexer());

   // schedule indexing to occur up front + at idle time
   module_context::scheduleIncrementalWork(
         boost::posix_time::milliseconds(200),
         boost::posix_time::milliseconds(20),
         boost::bind(&SourceFileIndexer::indexNextFile, pIndexer));
}


void searchSourceDatabase(const std::string& term,
                          std::size_t maxResults,
                          bool prefixOnly,
                          std::vector<r_util::RSourceItem>* pItems,
                          std::set<std::string>* pContextsSearched)
{


   // get all of the source indexes
   std::vector<boost::shared_ptr<r_util::RSourceIndex> > rIndexes =
                                             modules::source::rIndexes();

   BOOST_FOREACH(boost::shared_ptr<r_util::RSourceIndex>& pIndex, rIndexes)
   {
      // get file path
      FilePath docPath = module_context::resolveAliasedPath(pIndex->context());

      // bail if the file isn't in the project
      std::string projRelativePath =
            docPath.relativePath(projects::projectContext().directory());
      if (projRelativePath.empty())
         continue;

      // record that we searched this path
      pContextsSearched->insert(projRelativePath);

      // scan the source index
      pIndex->search(term,
                     projRelativePath,
                     prefixOnly,
                     false,
                     std::back_inserter(*pItems));

      // return if we are past maxResults
      if (pItems->size() >= maxResults)
      {
         pItems->resize(maxResults);
         return;
      }
   }
}

void searchProject(const std::string& term,
                   std::size_t maxResults,
                   bool prefixOnly,
                   const std::set<std::string>& excludeContexts,
                   std::vector<r_util::RSourceItem>* pItems)
{
   BOOST_FOREACH(const boost::shared_ptr<core::r_util::RSourceIndex>& pIndex,
                 s_sourceIndexes)
   {
      // bail if this is an exluded context
      if (excludeContexts.find(pIndex->context()) != excludeContexts.end())
         continue;

      // scan the next index
      pIndex->search(term,
                     prefixOnly,
                     false,
                     std::back_inserter(*pItems));

      // return if we are past maxResults
      if (pItems->size() >= maxResults)
      {
         pItems->resize(maxResults);
         return;
      }
   }
}

void searchSource(const std::string& term,
                  std::size_t maxResults,
                  bool prefixOnly,
                  std::vector<r_util::RSourceItem>* pItems,
                  bool* pMoreAvailable)
{
   // default to no more available
   *pMoreAvailable = false;

   // first search the source database
   std::set<std::string> srcDBContexts;
   searchSourceDatabase(term, maxResults, prefixOnly, pItems, &srcDBContexts);

   // we are done if we had >= maxResults
   if (pItems->size() > maxResults)
   {
      *pMoreAvailable = true;
      pItems->resize(maxResults);
      return;
   }

   // now search the project (excluding contexts already searched in the source db)
   std::vector<r_util::RSourceItem> projItems;
   searchProject(term, maxResults, prefixOnly, srcDBContexts, &projItems);

   // add project items to the list
   BOOST_FOREACH(const r_util::RSourceItem& sourceItem, projItems)
   {
      // add the item
      pItems->push_back(sourceItem);

      // bail if we've hit the max
      if (pItems->size() > maxResults)
      {
         *pMoreAvailable = true;
         pItems->resize(maxResults);
         break;
      }
   }
}


void searchFiles(const std::string& term,
                 std::size_t maxResults,
                 bool prefixOnly,
                 json::Array* pNames,
                 json::Array* pPaths,
                 bool* pMoreAvailable)
{
   // default to no more available
   *pMoreAvailable = false;

   // search from project root
   FilePath rootDir = projects::projectContext().directory();

    // create wildcard pattern if the search has a '*'
   bool hasWildcard = term.find('*') != std::string::npos;
   boost::regex pattern;
   if (hasWildcard)
      pattern = regex_utils::wildcardPatternToRegex(term);

   // iterate over project files
   FilePath filePath;
   RecursiveDirectoryIterator dirIter(rootDir);
   while (!dirIter.finished())
   {
      // get the next file
      Error error = dirIter.next(&filePath);
      if (error)
      {
         LOG_ERROR(error);
         return;
      }

      // filter directories
      if (filePath.isDirectory())
         continue;

      // filter file extensions
      std::string ext = filePath.extensionLowerCase();
      if (ext != ".r")
         continue;

      // get name for comparison
      std::string name = filePath.filename();

      // compare for match (wildcard or standard)
      bool matches = false;
      if (hasWildcard)
      {
         matches = regex_utils::textMatches(name,
                                            pattern,
                                            prefixOnly,
                                            false);
      }
      else
      {
         if (prefixOnly)
            matches = boost::algorithm::istarts_with(name, term);
         else
            matches = boost::algorithm::icontains(name, term);
      }

      // add the file if we found a match
      if (matches)
      {
         // name and project relative directory
         pNames->push_back(filePath.filename());
         pPaths->push_back(filePath.relativePath(rootDir));

         // return if we are past max results
         if (pNames->size() > maxResults)
         {
            *pMoreAvailable = true;
            pNames->resize(maxResults);
            pPaths->resize(maxResults);
            return;
         }
      }

   }
}


template <typename TValue, typename TFunc>
json::Array toJsonArray(
      const std::vector<r_util::RSourceItem> &items,
      TFunc memberFunc)
{
   json::Array col;
   std::transform(items.begin(),
                  items.end(),
                  std::back_inserter(col),
                  boost::bind(json::toJsonValue<TValue>,
                                 boost::bind(memberFunc, _1)));
   return col;
}

bool compareItems(const r_util::RSourceItem& i1, const r_util::RSourceItem& i2)
{
   return i1.name() < i2.name();
}

Error searchCode(const json::JsonRpcRequest& request,
                 json::JsonRpcResponse* pResponse)
{
   // get params
   std::string term;
   int maxResultsInt;
   Error error = json::readParams(request.params, &term, &maxResultsInt);
   if (error)
      return error;
   std::size_t maxResults = safe_convert::numberTo<std::size_t>(maxResultsInt,
                                                                20);

   // object to return
   json::Object result;

   // search files
   json::Array names;
   json::Array paths;
   bool moreFilesAvailable = false;
   searchFiles(term,
               maxResults,
               true,
               &names,
               &paths,
               &moreFilesAvailable);
   json::Object files;
   files["filename"] = names;
   files["path"] = paths;
   result["file_items"] = files;

   // search source (sort results by name)
   std::vector<r_util::RSourceItem> items;
   bool moreSourceItemsAvailable = false;
   searchSource(term, maxResults, true, &items, &moreSourceItemsAvailable);
   std::sort(items.begin(), items.end(), compareItems);

   // see if we need to do src truncation
   bool truncated = false;
   if ( (names.size() + items.size()) > maxResults )
   {
      // truncate source items
      std::size_t srcItems = maxResults - names.size();
      items.resize(srcItems);
      truncated = true;
   }

   // return rpc array list (wire efficiency)
   json::Object src;
   src["type"] = toJsonArray<int>(items, &r_util::RSourceItem::type);
   src["name"] = toJsonArray<std::string>(items, &r_util::RSourceItem::name);
   src["context"] = toJsonArray<std::string>(items, &r_util::RSourceItem::context);
   src["line"] = toJsonArray<int>(items, &r_util::RSourceItem::line);
   src["column"] = toJsonArray<int>(items, &r_util::RSourceItem::column);
   result["source_items"] = src;

   // set more available bit
   result["more_available"] =
         moreFilesAvailable || moreSourceItemsAvailable || truncated;

   pResponse->setResult(result);

   return Success();
}

void onDeferredInit()
{
   // initialize source index
   if (code_search::enabled())
      indexProjectFiles();
}
   
} // anonymous namespace


bool enabled()
{
   return projects::projectContext().hasProject();
}
   
Error initialize()
{
   // sign up for deferred init
   module_context::events().onDeferredInit.connect(onDeferredInit);


   using boost::bind;
   using namespace module_context;
   ExecBlock initBlock ;
   initBlock.addFunctions()
      (bind(registerRpcMethod, "search_code", searchCode));
   ;

   return initBlock.execute();
}


} // namespace agreement
} // namespace modules
} // namespace session
