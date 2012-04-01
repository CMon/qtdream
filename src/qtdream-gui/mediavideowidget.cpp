#include "mediavideowidget.h"

#include <log.h>

#include <QKeyEvent>
#include <QWheelEvent>
#include <QTimer>

namespace {
int MaxVolume = 100;
int MinVolume = 0;
}

// basicly the exampleis taken from: http://wiki.videolan.org/LibVLC_SampleCode_Qt

MediaVideoWidget::MediaVideoWidget(QWidget *parent) :
#ifdef Q_WS_X11
    QX11EmbedContainer(parent),
#else
    QFrame(parent),
#endif
    vlcInstance_(NULL),
    mediaPlayer_(NULL),
    media_(NULL)
{
    //preparation of the vlc command
    const char * const vlc_args[] = {
              "-I", "dummy", /* Don't use any interface */
              "--ignore-config", /* Don't use VLC's config */
              "--extraintf=logger", //log anything
              "--verbose=2", //be much more verbose then normal for debugging purpose
              "--plugin-path=C:\\vlc-0.9.9-win32\\plugins\\"
    };

    isPlaying_ = false;
    poller_ = new QTimer(this);

    //create a new libvlc instance
    vlcInstance_ = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);  //tricky calculation of the char space used

    // Create a media player playing environement
    mediaPlayer_ = libvlc_media_player_new (vlcInstance_);

    //connect the two sliders to the corresponding slots (uses Qt's signal / slots technology)
    connect(poller_, SIGNAL(timeout()), this, SLOT(updateInterface()));

    poller_->start(100); //start timer to trigger every 100 ms the updateInterface slot
}

MediaVideoWidget::~MediaVideoWidget()
{
    if (media_) libvlc_media_release(media_);

    // Stop playing
    if (mediaPlayer_) libvlc_media_player_stop (mediaPlayer_);
    // Free the media_player
    if (mediaPlayer_) libvlc_media_player_release (mediaPlayer_);
    // release vlcInstance
    if (vlcInstance_) libvlc_release (vlcInstance_);
}

void MediaVideoWidget::setFullScreen(bool enabled)
{
    LOG_DEBUG("TODO: IMPLEMENT ME");
    isFullScreen_ = enabled;
}

//the file has to be in one of the following formats /perhaps a little bit outdated)
/**
    * @brief playback Start playback of the given url
    * @param url The URL has to be one of the following:
    * [file://]filename              Plain media file
    * http://ip:port/file            HTTP URL
    * ftp://ip:port/file             FTP URL
    * mms://ip:port/file             MMS URL
    * screen://                      Screen capture
    * [dvd://][device][@raw_device]  DVD device
    * [vcd://][device]               VCD device
    * [cdda://][device]              Audio CD device
    * udp:[[<source address>]@[<bind address>][:<bind port>]]
*/
void MediaVideoWidget::playback(const QString & url)
{
    if (url.isEmpty()) return;
    LOG_DEBUG("Starting playback of: %1", url);

    // Create a new LibVLC media descriptor
    if (media_) libvlc_media_release(media_);
    media_ = libvlc_media_new_path(vlcInstance_, url.toAscii());

    // stop playback
    libvlc_media_player_stop(mediaPlayer_);

    // assigne media to mediaplayer
    libvlc_media_player_set_media (mediaPlayer_, media_);

    // Get our media instance to use our window
#if defined(Q_OS_WIN)
    libvlc_media_player_set_hwnd(mediaPlayer_, reinterpret_cast<unsigned int>(winId()));
#elif defined(Q_OS_MAC)
    libvlc_media_player_set_agl(mediaPlayer_, winId());
#else //Linux
    libvlc_media_player_set_xwindow (mediaPlayer_, winId());
#endif

    // start playback
    libvlc_media_player_play (mediaPlayer_);

    isPlaying_ = true;
}

void MediaVideoWidget::mouseDoubleClickEvent(QMouseEvent * /*e*/)
{
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
}

void MediaVideoWidget::wheelEvent(QWheelEvent *e)
{
     if (e->orientation() == Qt::Vertical) {
         if (e->delta() < 0)       changeVolume(currentVolume_--);
         else if (e->delta() > 0 ) changeVolume(currentVolume_++);
     }
     e->accept();
}

void MediaVideoWidget::updateInterface()
{
    // no playback => no update
    if(!isPlaying_)
        return;

    libvlc_media_t * curMedia = libvlc_media_player_get_media (mediaPlayer_);
    if (!curMedia) return;

    currentVolume_ = libvlc_audio_get_volume (mediaPlayer_);
}

void MediaVideoWidget::changeVolume(int newVolume)
{
    // limit volume MinVolume <= newVolume <= MaxVolume
    newVolume = qMax(qMin(newVolume, MaxVolume), MinVolume);

    LOG_DEBUG("Setting volume to: %1", newVolume);

    libvlc_audio_set_volume (mediaPlayer_, newVolume);
}
