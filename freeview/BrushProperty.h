/**
 * @file  BrushProperty.h
 * @brief Class to hold brush properties for voxel editing
 *
 * Simpleclass for use with the Listener class so text
 * messages with a pointer data can be sent to a list of listeners.
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2017/01/11 21:05:23 $
 *    $Revision: 1.16 $
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
 *
 */


#ifndef BrushProperty_h
#define BrushProperty_h

#include <QObject>
#include <QVariantMap>

class LayerVolumeBase;
class Layer;

class BrushProperty : public QObject
{
  Q_OBJECT
public:
  BrushProperty(QObject* parent=0);
  virtual ~BrushProperty ();

  int  GetBrushSize();

  int  GetBrushTolerance();

  LayerVolumeBase*  GetReferenceLayer();

  double* GetDrawRange();
  void  SetDrawRange( double* range );
  void  SetDrawRange( double low, double high );

  bool GetDrawRangeEnabled();

  double* GetExcludeRange();
  void SetExcludeRange( double* range );
  void  SetExcludeRange( double low, double high );

  bool GetExcludeRangeEnabled();

  bool GetDrawConnectedOnly();

  double* GetEraseRange();
  void  SetEraseRange( double* range );
  void  SetEraseRange( double low, double high );

  bool GetEraseRangeEnabled();

  double* GetEraseExcludeRange();
  void SetEraseExcludeRange( double* range );
  void  SetEraseExcludeRange( double low, double high );

  bool GetEraseExcludeRangeEnabled();

  bool GetFill3D()
  {
    return m_bFill3D;
  }

  double GetFillValue()
  {
    return m_dFillValue;
  }

  double GetEraseValue()
  {
    return m_dEraseValue;
  }

  bool GetCloning()
  {
    return m_bIsCloning;
  }

  QVariantMap GetGeosSettings()
  {
    return m_mapGeos;
  }

  void SetGeosSettings(const QString& name, const QVariant& val)
  {
    m_mapGeos[name] = val;
  }

signals:
  void FillValueChanged(double);
  void EraseValueChanged(double);
  void BrushSizeChanged(int);

public slots:
  void SetFillValue(double val);
  void SetEraseValue(double val);
  void SetBrushSize( int nSize );
  void SetBrushTolerance( int nTolerance );
  void SetReferenceLayer( LayerVolumeBase* layer );
  void SetDrawRangeEnabled( bool bEnable );
  void SetExcludeRangeEnabled( bool bEnable );
  void SetDrawConnectedOnly( bool bEnable );
  void OnLayerRemoved(Layer* layer);
  void SetFill3D(bool bVal)
  {
    m_bFill3D = bVal;
  }
  void SetCloning(bool bVal)
  {
    m_bIsCloning = bVal;
  }

  void SetEraseRangeEnabled( bool bEnable );
  void SetEraseExcludeRangeEnabled( bool bEnable );

protected:
  int  m_nBrushSize;
  int  m_nBrushTolerance;
  double m_dDrawRange[2];
  bool m_bEnableDrawRange;
  double m_dExcludeRange[2];
  bool m_bEnableExcludeRange;
  double m_dEraseRange[2];
  bool m_bEnableEraseRange;
  double m_dEraseExcludeRange[2];
  bool m_bEnableEraseExcludeRange;
  bool m_bDrawConnectedOnly;
  bool  m_bFill3D;
  bool m_bIsCloning;

  double m_dFillValue;
  double m_dEraseValue;

  QVariantMap m_mapGeos;

  LayerVolumeBase* m_layerRef;
};


#endif
