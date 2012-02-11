#pragma once

#include <phonon/videowidget.h>

class MediaVideoWidget : public Phonon::VideoWidget
{
    Q_OBJECT

public:
    MediaVideoWidget(QWidget *parent = 0);

public slots:
    void setFullScreen(bool);

signals:
    void fullScreenChanged(bool);
    void decreaseVolume();
    void increaseVolume();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);
};
