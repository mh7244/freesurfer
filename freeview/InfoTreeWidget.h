/**
 * @file  InfoTreeWidget.h
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2016/02/09 21:22:39 $
 *    $Revision: 1.10 $
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */
#ifndef INFOTREEWIDGET_H
#define INFOTREEWIDGET_H

#include <QTreeWidget>
#include <QVariantMap>

class QLineEdit;
class QTreeWidgetItem;
class Layer;

class InfoTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  InfoTreeWidget(QWidget* parent = 0);
  void contextMenuEvent(QContextMenuEvent *);

  void SetForCursor(bool bCursor)
  {
    m_bForCursor = bCursor;
  }

signals:
  void RASChangeTriggered(double x, double y, double z);
  void VertexChangeTriggered(int nVertex);

public slots:
  void UpdateTrackVolumeAnnotation(Layer* layer, const QVariantMap& info);
  void UpdateAll();
  void ShowHeaderText();
  void ClearHeaderText();

protected slots:
  void OnMousePositionChanged();
  void OnCursorPositionChanged();
  void OnItemClicked(QTreeWidgetItem * item, int column);
  void OnEditFinished();
  void OnToggleShowInfo(bool bShow);
  void OnToggleSurfaceCurvature(bool show);
  void OnToggleSurfaceNormal(bool show);
  void OnToggleShowTkRegRAS(bool bShow);

protected:
  void showEvent(QShowEvent *);
  void keyPressEvent(QKeyEvent *event);
  void mousePressEvent(QMouseEvent *event);

private:
  double m_dRAS[3];
  bool  m_bShowSurfaceNormal;
  bool  m_bShowSurfaceCurvature;
  bool  m_bShowTkRegRAS;
  QLineEdit*  m_editor;
  QTreeWidgetItem* m_itemEdited;
  bool  m_bForCursor;
};

#endif // INFOTREEWIDGET_H
