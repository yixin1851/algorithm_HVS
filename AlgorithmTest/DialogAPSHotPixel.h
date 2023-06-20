#pragma once
#include <qdialog.h>
#include "ui_DialogAPSHotPixel.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetImageView.h"

class CDialogAPSHotPixel :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSHotPixel(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void HotPixel();
private:
    Ui::DialogAPSHotPixel ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSBadpixelType m_HotPixel;
    CWidgetTableView m_widgetTableView;
    CWidgetImageView m_widgetImageView[SubFrameIndex::All];
};
