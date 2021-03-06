#
# ModuleTools.R
#
# Copyright (C) 2009-11 by RStudio, Inc.
#
# This program is licensed to you under the terms of version 3 of the
# GNU Affero General Public License. This program is distributed WITHOUT
# ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
# AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
#
#

.rs.addFunction("enqueClientEvent", function(type, data = NULL)
{
   .Call("rs_enqueClientEvent", type, data)
})

.rs.addFunction("scratchPath", function()
{
   .Call("rs_scratchPath")
})

.rs.addFunction("scopedScratchPath", function()
{
   .Call("rs_scopedScratchPath")
})

.rs.addFunction("showErrorMessage", function(title, message)
{
   .Call("rs_showErrorMessage", title, message)
})

.rs.addFunction("logErrorMessage", function(message)
{
   .Call("rs_logErrorMessage", message)
})

.rs.addFunction("logWarningMessage", function(message)
{
   .Call("rs_logWarningMessage", message)
})

.rs.addGlobalFunction("RStudio.version", function()
{
   .Call("rs_rstudioVersion")
})

.rs.addFunction("getSignature", function(obj)
{
   sig = capture.output(print(args(obj)))
   sig = sig[1:length(sig)-1]
   sig = gsub('^\\s+', '', sig)
   paste(sig, collapse='')
})

# Wrap a return value in this to give a hint to the
# JSON serializer that one-element vectors should be
# marshalled as scalar types instead of arrays
.rs.addFunction("scalar", function(obj)
{
   class(obj) <- 'rs.scalar'
   return(obj)
})

.rs.addFunction("validateAndNormalizeEncoding", function(encoding)
{
   iconvList <- toupper(iconvlist())
   encodingUpper <- toupper(encoding)
   if (encodingUpper %in% iconvList)
   {
      return (encodingUpper)
   }
   else
   {
      encodingUpper <- gsub("[_]", "-", encodingUpper)
      if (encodingUpper %in% iconvList)
         return (encodingUpper)
      else
         return ("")
   }
})
