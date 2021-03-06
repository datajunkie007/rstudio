/*
 * ChangelistTablePresenter.java
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
package org.rstudio.studio.client.workbench.views.vcs;

import com.google.gwt.core.client.JsArray;
import com.google.inject.Inject;
import org.rstudio.core.client.cellview.ColumnSortInfo;
import org.rstudio.core.client.js.JsObject;
import org.rstudio.studio.client.common.SimpleRequestCallback;
import org.rstudio.studio.client.common.vcs.StatusAndPath;
import org.rstudio.studio.client.common.vcs.VCSServerOperations;
import org.rstudio.studio.client.server.Void;
import org.rstudio.studio.client.workbench.model.ClientState;
import org.rstudio.studio.client.workbench.model.Session;
import org.rstudio.studio.client.workbench.model.helper.JSObjectStateValue;
import org.rstudio.studio.client.workbench.views.vcs.events.StageUnstageEvent;
import org.rstudio.studio.client.workbench.views.vcs.events.StageUnstageHandler;
import org.rstudio.studio.client.workbench.views.vcs.events.VcsRefreshEvent;
import org.rstudio.studio.client.workbench.views.vcs.events.VcsRefreshHandler;
import org.rstudio.studio.client.workbench.views.vcs.model.VcsState;

import java.util.ArrayList;

public class ChangelistTablePresenter
{
   @Inject
   public ChangelistTablePresenter(VCSServerOperations server,
                                   ChangelistTable view,
                                   Session session,
                                   VcsState vcsState)
   {
      server_ = server;
      view_ = view;
      session_ = session;
      vcsState_ = vcsState;

      view_.addStageUnstageHandler(new StageUnstageHandler()
      {
         @Override
         public void onStageUnstage(StageUnstageEvent event)
         {
            ArrayList<String> paths = new ArrayList<String>();
            for (StatusAndPath path : event.getPaths())
               paths.add(path.getPath());

            if (event.isUnstage())
            {
               server_.vcsUnstage(paths,
                                  new SimpleRequestCallback<Void>());
            }
            else
            {
               server_.vcsStage(paths,
                                new SimpleRequestCallback<Void>());
            }
         }
      });

      vcsState_.addVcsRefreshHandler(new VcsRefreshHandler()
      {
         @Override
         public void onVcsRefresh(VcsRefreshEvent event)
         {
            JsArray<StatusAndPath> status = vcsState_.getStatus();
            ArrayList<StatusAndPath> list = new ArrayList<StatusAndPath>();
            for (int i = 0; i < status.length(); i++)
               list.add(status.get(i));
            view_.setItems(list);
         }
      });
   }

   public void initializeClientState()
   {
      new JSObjectStateValue(MODULE_VCS, KEY_SORT_ORDER, ClientState.PERSISTENT,
                             session_.getSessionInfo().getClientState(),
                             false) {
         @Override
         protected void onInit(JsObject value)
         {
            if (value != null)
            {
               view_.setSortOrder(value.<JsArray<ColumnSortInfo>>cast());
               lastHashCode_ = view_.getSortOrderHashCode();
            }
         }

         @Override
         protected JsObject getValue()
         {
            return view_.getSortOrder().cast();
         }

         @Override
         protected boolean hasChanged()
         {
            if (lastHashCode_ != view_.getSortOrderHashCode())
            {
               lastHashCode_ = view_.getSortOrderHashCode();
               return true;
            }
            return false;
         }

         public int lastHashCode_;
      };
   }

   public ChangelistTable getView()
   {
      return view_;
   }

   private final VCSServerOperations server_;
   private final ChangelistTable view_;
   private final Session session_;
   private final VcsState vcsState_;

   private static final String KEY_SORT_ORDER = "sortOrder";
   private static final String MODULE_VCS = "vcs";
}
