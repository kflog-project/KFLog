/***********************************************************************
**
**   igcpreview.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef IGCPREVIEW_H
#define IGCPREVIEW_H

#include <q3filedialog.h>
#include <qlabel.h>

/**
 * @short File preview for open file dialog
 *
 * Provides a file-preview in the open-file-dialog. Only the header of
 * a local igc-file is displayed. Non-local files are not used. The
 * preview displays the date, the pilotname, the glider-type and -id and
 * the name of the waypoints.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class IGCPreview : public QLabel, public Q3FilePreview
{
  Q_OBJECT

  public:
    /**
     * Creates a preview-widget.
     *
     * @param  parent  The parent widget of the preview-widget
     */
    IGCPreview(QWidget* parent);
    /**
     * Destructor
     */
    ~IGCPreview();

  public slots:
    /**
     * This slot is called every time the user selects another file in
     * the file dialog. Reimplemented from QFilePreview.
     *
     * @param  url  The url of the selected file
     */
    virtual void previewUrl(const Q3Url &url);

  private:
    /**
     * The text-label in which the preview will be shown.
     */
    QLabel* textLabel;
};

#endif
