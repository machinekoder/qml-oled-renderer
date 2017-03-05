#ifndef OLEDRENDERER_H
#define OLEDRENDERER_H

#include <QFutureWatcher>
#include <QObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QOpenGLFunctions>
#include <QTimer>
#include "animationdriver.h"

class OledRenderer : public QObject
{
    Q_OBJECT
public:
    enum Status {
        NotRunning,
        Running
    };

    explicit OledRenderer(QObject *parent = 0);

    ~OledRenderer();

    void loadQmlFile(const QString &qmlFile, const QSize &size, qreal devicePixelRatio = 1.0, int fps = 24);

    bool isRunning();

    QQuickItem * rootItem();

signals:
    void imageRendered(const QImage &image);

private slots:
    void start();
    void cleanup();

    void createFbo();
    void destroyFbo();
    bool loadQml(const QString &qmlFile, const QSize &size);

    void renderNext();

private:
    QOpenGLContext *m_context;
    QOffscreenSurface *m_offscreenSurface;
    QQuickRenderControl *m_renderControl;
    QQuickWindow *m_quickWindow;
    QQmlEngine *m_qmlEngine;
    QQmlComponent *m_qmlComponent;
    QQuickItem *m_rootItem;
    QOpenGLFramebufferObject *m_fbo;
    qreal m_dpr;
    QSize m_size;
    AnimationDriver *m_animationDriver;

    Status m_status;
    int m_fps;
    QTimer *m_renderTimer;
};

#endif // OLEDRENDERER_H
