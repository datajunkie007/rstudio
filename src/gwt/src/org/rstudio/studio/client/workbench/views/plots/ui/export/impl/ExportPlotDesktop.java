/*
 * ExportPlotDesktop.java
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
package org.rstudio.studio.client.workbench.views.plots.ui.export.impl;

import org.rstudio.core.client.widget.OperationWithInput;
import org.rstudio.studio.client.application.Desktop;
import org.rstudio.studio.client.workbench.views.plots.model.ExportPlotOptions;
import org.rstudio.studio.client.workbench.views.plots.model.PlotsServerOperations;
import org.rstudio.studio.client.workbench.views.plots.ui.export.ExportPlot;

public class ExportPlotDesktop extends ExportPlot
{

   @Override
   public void copyPlotToClipboard(
                              PlotsServerOperations server,
                              ExportPlotOptions options,
                              OperationWithInput<ExportPlotOptions> onClose)
   {   
      if (Desktop.getFrame().supportsClipboardMetafile())
      {
         new CopyPlotToClipboardDesktopMetafileDialog(server, 
                                                      options, 
                                                      onClose).showModal();
      }
      else
      {
         new CopyPlotToClipboardDesktopDialog(server, 
                                              options, 
                                              onClose).showModal();
      }
   }

}
