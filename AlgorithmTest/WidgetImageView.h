#pragma once
#include <qwidget.h>
#include "ui_WidgetImageView.h"
#include <vector>

class CWidgetImageView :
    public QWidget
{
    Q_OBJECT

public:
    CWidgetImageView(QWidget* parent = nullptr);
    void SetGrayData(const uchar* pImage, int32_t nWidth, int32_t nHeight);
    void SetColorData(const uchar* pImage, int32_t nWidth, int32_t nHeight);
    void Clear();
private:
    Ui::ImageViewWidget ui;
};

