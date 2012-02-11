#include "mediavideowidget.h"

#include <QKeyEvent>
#include <QWheelEvent>

MediaVideoWidget::MediaVideoWidget(QWidget *parent) :
    Phonon::VideoWidget(parent)
{
}

void MediaVideoWidget::setFullScreen(bool enabled)
{
    Phonon::VideoWidget::setFullScreen(enabled);
    emit fullScreenChanged(enabled);
}

void MediaVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    Phonon::VideoWidget::mouseDoubleClickEvent(e);
    setFullScreen(!isFullScreen());
}

void MediaVideoWidget::keyPressEvent(QKeyEvent *e)
{
    if(!e->modifiers()) {
        if (e->key() == Qt::Key_Escape) {
            setFullScreen(false);
            e->accept();
            return;
        }
    }
    Phonon::VideoWidget::keyPressEvent(e);
}

void MediaVideoWidget::wheelEvent(QWheelEvent *e)
{
     if (e->orientation() == Qt::Vertical) {
         if (e->delta() < 0)       emit decreaseVolume();
         else if (e->delta() > 0 ) emit increaseVolume();
     }
     e->accept();
}
