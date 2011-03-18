/*
 * FileDialogStyles.java
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
package org.rstudio.core.client.files.filedialog;

import com.google.gwt.resources.client.CssResource;

public interface FileDialogStyles extends CssResource
{

   String contents();

   String filenamePanel();
   String filenameLabel();
   String filename();

   String breadcrumbFrame();
   String breadcrumb();
   String path();
   String home();
   String last();
   String fade();
   String goUp();
   String browse();

   String columnIcon();
   String columnName();
   String columnSize();
   String columnDate();
   
   String setwdButton();
}
