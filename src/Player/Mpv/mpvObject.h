#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include "mpv.hpp"
#include <QByteArray>
#include <QClipboard>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QtQuick/QQuickFramebufferObject>

class MpvRenderer;

class MpvObject : public QQuickFramebufferObject {
  Q_OBJECT

  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
  Q_PROPERTY(qint64 time READ time NOTIFY timeChanged)
  Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(float speed READ speed WRITE setSpeed NOTIFY speedChanged)
  Q_PROPERTY(QSize videoSize READ videoSize NOTIFY videoSizeChanged)
  Q_PROPERTY(bool subVisible READ subVisible WRITE setSubVisible NOTIFY
                 subVisibleChanged)
  Q_PROPERTY(QStringList audioTracks READ audioTracks NOTIFY audioTracksChanged)
  Q_PROPERTY(QStringList subtitles READ subtitles NOTIFY subtitlesChanged)
  friend class MpvRenderer;

public:
  enum State { STOPPED, VIDEO_PLAYING, VIDEO_PAUSED, TV_PLAYING };
  enum Hwdec { AUTO, VAAPI, VDPAU, NVDEC };
  Q_ENUM(State)

  inline static MpvObject *instance() { return s_instance; }

  MpvObject(QQuickItem *parent = nullptr);
  virtual Renderer *createRenderer() const;

  // Access properties
  inline QSize videoSize() {
    return QSize(m_videoWidth, m_videoHeight) /
           window()->effectiveDevicePixelRatio();
  }
  inline State state() { return m_state; }
  inline qint64 duration() { return m_duration; }
  inline qint64 time() { return m_time; }
  inline bool subVisible() { return m_subVisible; }
  inline int volume() { return m_volume; }
  inline float speed() { return m_speed; }
  inline QStringList audioTracks() { return m_audioTracks; }
  inline QStringList subtitles() { return m_subtitles; }
  Q_INVOKABLE void testupdate() {
    if (isVisible()) {
      update();
    }
  }
  void setVolume(int volume);
  void setSubVisible(bool subVisible);
  qint64 OPStart = -1;
  qint64 OPLength = 0;
  qint64 EDStart = -1;
  qint64 EDLength = 0;
  struct {
    std::string modeA;
    std::string modeB;
    std::string modeC;
    std::string modeAA;
    std::string modeBB;
    std::string modeCA;
  } anime4K;
public slots:
  void open(const QUrl &fileUrl, const QUrl &danmakuUrl = QUrl(),
            const QUrl &audioTrackUrl = QUrl());
  void play(void);
  void pause(void);
  void stop(void);
  void mute(void) {
    if (m_volume > 0) {
      m_lastVolume = m_volume;
      setVolume(0);
    } else {
      setVolume(m_lastVolume);
    }
  }
  void setSpeed(float speed);
  void seek(qint64 offset, bool absolute = true);
  void screenshot(void);
  void addAudioTrack(const QUrl &url);
  void addSubtitle(const QUrl &url);
  void setProperty(const QString &name, const QVariant &value);
  void showText(const QByteArray &text);

  void loadAnime4K(int n) {
    std::string cmd = "CTRL+" + std::to_string(n);
    sendKeyPress(cmd.data());
  }
  void sendKeyPress(const char *cmd) {
    const char *args[] = {"keypress", cmd, nullptr};
    m_mpv.command_async(args);
  }
  void pasteOpen() {
    QString clipboardText = QGuiApplication::clipboard()->text();
    qDebug() << clipboardText;
    if (clipboardText.contains(".m3u8") || clipboardText.contains(".mp4"))
      open(clipboardText);
    else if (clipboardText.endsWith(".vtt")) {
      addSubtitle(clipboardText);
      setSubVisible(true);
    }
  }
  void copyVideoLink() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(currentVideoLink);
  }
  void reload() {
    if (currentVideoLink.isEmpty())
      return;
    open(currentVideoLink);
  }

  bool setOPSkipStart(qint64 start) {
    if (start < 0)
      return false;
    OPStart = start;
    return true;
  }
  bool setEDSkipStart(qint64 start) {
    if (start < 0)
      return false;
    EDStart = start;
    return true;
  }
  void setOPSkipLength(qint64 length) { OPLength = length; }
  void setEDSkipLength(qint64 length) { EDLength = length; }

signals:
  void audioTracksChanged(void);
  void stateChanged(void);
  void subtitlesChanged(void);
  void subVisibleChanged(void);
  void durationChanged(void);
  void timeChanged(void);
  void volumeChanged(void);
  void speedChanged(void);
  void videoSizeChanged(void);

  void initialised(void);
  void playNext(void);

private:
  static void on_update(void *ctx);
  Q_INVOKABLE void onMpvEvent(void);
  void handleMpvError(int code);

  Mpv::Handle m_mpv;

  QString currentVideoLink;

  State m_state = STOPPED;
  mpv_end_file_reason m_endFileReason = MPV_END_FILE_REASON_STOP;
  bool m_subVisible = false;
  float m_speed = 1.0;
  int m_volume = 50;
  int m_lastVolume = 0;
  int m_danmakuDisallowMode = 0;
  int64_t m_time;
  int64_t m_duration;
  int64_t m_videoWidth = 0;
  int64_t m_videoHeight = 0;
  double m_reservedArea = 0;
  QUrl m_danmakuUrl;
  QUrl m_audioToBeAdded;
  QStringList m_audioTracks;
  QStringList m_subtitles;
  //    Danmaku2ASS::AssBuilder::Ptr m_danmakuAss;
  std::vector<std::string> m_blockWords;

  static MpvObject *s_instance;
};

#endif
