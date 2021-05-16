#include "videowidget.h"

#include <QDebug>
#include <QPainter>

CVideoWidget::CVideoWidget(QWidget *parent) : QWidget(parent)
{

}

void CVideoWidget::Save()
{
    m_oImage.save("testFrame.jpg");
}

void CVideoWidget::paintEvent(QPaintEvent *event)
{
    QPainter oPaint(this);
    oPaint.drawImage(this->rect(), m_oImage);
}

void CVideoWidget::SLOT_UpdateFrame(const QImage &oImage)
{
    m_oImage = oImage;
    update();
}
