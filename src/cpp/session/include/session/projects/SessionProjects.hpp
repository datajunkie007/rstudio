/*
 * SessionProjects.hpp
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

#ifndef SESSION_PROJECTS_PROJECTS_HPP
#define SESSION_PROJECTS_PROJECTS_HPP


#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <core/FilePath.hpp>
#include <core/json/Json.hpp>

#include <core/r_util/RProjectFile.hpp>

#include <core/r_util/RSourceIndex.hpp>
 
namespace session {
namespace projects {

class ProjectContext : boost::noncopyable
{
public:
   ProjectContext() {}
   virtual ~ProjectContext() {}

   core::Error startup(const core::FilePath& projectFile,
                       std::string* pUserErrMsg);

   core::Error initialize();

public:
   bool hasProject() const { return !file_.empty(); }

   const core::FilePath& file() const { return file_; }
   const core::FilePath& directory() const { return directory_; }
   const core::FilePath& scratchPath() const { return scratchPath_; }

   const core::r_util::RProjectConfig& config() const { return config_; }
   void setConfig(const core::r_util::RProjectConfig& config)
   {
      config_ = config;
   }

   // code which needs to rely on the encoding should call this method
   // rather than getting the encoding off of the config (because the
   // config could have been created on another system with an encoding
   // not available here -- defaultEncoding reflects (if possible) a
   // local mapping of an unknown encoding and a fallback to UTF-8
   // if necessary
   std::string defaultEncoding() const;

   core::json::Object uiPrefs() const;

public:
   static core::r_util::RProjectConfig defaultConfig();

private:
   core::FilePath file_;
   core::FilePath directory_;
   core::FilePath scratchPath_;
   core::r_util::RProjectConfig config_;
   std::string defaultEncoding_;
};

const ProjectContext& projectContext();

} // namespace projects
} // namesapce session

#endif // SESSION_PROJECTS_PROJECTS_HPP
