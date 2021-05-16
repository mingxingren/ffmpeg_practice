#ifndef CVIDEOWIDGET_H
#define CVIDEOWIDGET_H

#include <QWidget>

class CVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CVideoWidget(QWidget *parent = nullptr);

    void Save();

protected:
    virtual void paintEvent(QPaintEvent *event);

public slots:
    void SLOT_UpdateFrame(const QImage &oImage);

private:
    QImage m_oImage;
};

#endif // CVIDEOWIDGET_H
