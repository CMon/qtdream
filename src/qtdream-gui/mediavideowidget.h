#pragma once

#include <vlc/vlc.h>

#ifdef Q_WS_X11
    #include <QX11EmbedContainer>
#else
    #include <QFrame>
#endif

class QTimer;

#ifdef Q_WS_X11
    class MediaVideoWidget : public QX11EmbedContainer
#else
    class MediaVideoWidget : public QFrame
#endif
{
    Q_OBJECT

public:
    MediaVideoWidget(QWidget *parent = 0);
    ~MediaVideoWidget();

    bool isFullScreen() { return isFullScreen_; }

public slots:
    void setFullScreen(bool);
    void playback(const QString & url);
    void changeVolume(int newVolume);

signals:
    void fullScreenChanged(bool);
    void decreaseVolume();
    void increaseVolume();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);

private slots:
    void updateInterface();

private:
#ifdef Q_WS_X11
    QX11EmbedContainer * videoWidget_;
#else
    QFrame * videoWidget_;
#endif
    QTimer * poller_;
    bool     isPlaying_;
    bool     isFullScreen_;
    int      currentVolume_;

    libvlc_instance_t *     vlcInstance_;
    libvlc_media_player_t * mediaPlayer_;
    libvlc_media_t *        media_;
};
