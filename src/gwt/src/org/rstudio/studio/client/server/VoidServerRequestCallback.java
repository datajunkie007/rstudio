/*
 * VoidServerRequestCallback.java
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
package org.rstudio.studio.client.server;

import org.rstudio.core.client.widget.ProgressIndicator;

public class VoidServerRequestCallback extends ServerRequestCallback<Void>
{
   public VoidServerRequestCallback()
   {
      this(null);
   }
   
   public VoidServerRequestCallback(ProgressIndicator progress)
   {
      progress_ = progress ;
   }
   
   public void onResponseReceived(Void response)
   {
      if (progress_ != null)
         progress_.onCompleted();
      
      onSuccess();
   }
   
   public void onError(ServerError error)
   {
      if (progress_ != null)
         progress_.onError(error.getUserMessage());
      
      onFailure();
   }
   
   protected void onSuccess()
   {
      
   }
   
   protected void onFailure()
   {
      
   }
   
   private ProgressIndicator progress_ ;
}
