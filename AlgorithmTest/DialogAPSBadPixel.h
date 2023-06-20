#pragma once
#include <qdialog.h>
#include "ui_DialogAPSBadPixel.h"
#include "AlpMPAlgoInterface.h"
#include "WidgetTableView.h"
#include "WidgetImageView.h"

class CDialogAPSBadPixel :
    public QDialog
{
    Q_OBJECT
public:
    CDialogAPSBadPixel(QDialog* parent = nullptr, CAlpAPSMPAlgoInterface* pAPSAlgoInterface = nullptr, CAlpDVSMPAlgoInterface* pDVSAlgoInterface = nullptr);
private slots:
    virtual void Export();
    virtual void BadPixel();
private:
    Ui::DialogAPSBadPixel ui;
    CAlpAPSMPAlgoInterface* m_pAPSAlgoInterface;
    CAlpDVSMPAlgoInterface* m_pDVSAlgoInterface;
    APSBadpixelType m_BadPixel;
    CWidgetTableView m_widgetTableView;
    CWidgetImageView m_widgetImageView[SubFrameIndex::All];
};
