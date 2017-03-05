#include "oledrenderer.h"

#include <QSurfaceFormat>

OledRenderer::OledRenderer(QObject *parent)
    : QObject(parent)
    , m_context(nullptr)
    , m_offscreenSurface(nullptr)
    , m_renderControl(nullptr)
    , m_quickWindow(nullptr)
    , m_qmlEngine(nullptr)
    , m_qmlComponent(nullptr)
    , m_rootItem(nullptr)
    , m_fbo(nullptr)
    , m_animationDriver(nullptr)
    , m_status(NotRunning)
    , m_renderTimer(nullptr)
{
    QSurfaceFormat format;
    // Qt Quick may need a depth and stencil buffer. Always make sure these are available.
    format.setDepthBufferSize(16);
    format.setStencilBufferSize(8);
    format.setSamples(1);

    m_context = new QOpenGLContext;
    m_context->setFormat(format);
    m_context->create();

    m_offscreenSurface = new QOffscreenSurface;
    m_offscreenSurface->setFormat(m_context->format());
    m_offscreenSurface->create();

    m_renderControl = new QQuickRenderControl(this);
    m_quickWindow = new QQuickWindow(m_renderControl);

    m_qmlEngine = new QQmlEngine;
    if (!m_qmlEngine->incubationController())
        m_qmlEngine->setIncubationController(m_quickWindow->incubationController());

    m_context->makeCurrent(m_offscreenSurface);
    m_renderControl->initialize(m_context);
}

OledRenderer::~OledRenderer()
{
    m_context->makeCurrent(m_offscreenSurface);
    delete m_renderControl;
    delete m_qmlComponent;
    delete m_quickWindow;
    delete m_qmlEngine;
    delete m_fbo;

    m_context->doneCurrent();

    delete m_offscreenSurface;
    delete m_context;
    delete m_animationDriver;
    delete m_renderTimer;
}

void OledRenderer::loadQmlFile(const QString &qmlFile, const QSize &size, qreal devicePixelRatio, int fps)
{
    if (m_status != NotRunning) {
        return;
    }

    m_size = size;
    m_dpr = devicePixelRatio;
    m_fps = fps;

    if (!loadQml(qmlFile, size)) {
        return;
    }

    start();
}

void OledRenderer::start()
{
    m_status = Running;
    createFbo();

    if (!m_context->makeCurrent(m_offscreenSurface)) {
        return;
    }

    int renderInterval = 1000 / m_fps;
    // Render each frame of movie
    m_animationDriver = new AnimationDriver(renderInterval);
    m_animationDriver->install();

    // Start the renderer
    m_renderTimer = new QTimer;
    m_renderTimer->setInterval(renderInterval);
    connect(m_renderTimer, &QTimer::timeout, this, &OledRenderer::renderNext);
    m_renderTimer->start();
    renderNext();
}

void OledRenderer::cleanup()
{
    m_animationDriver->uninstall();
    delete m_animationDriver;
    m_animationDriver = nullptr;

    if (m_renderTimer != nullptr) {
        delete m_renderTimer;
        m_renderTimer = nullptr;
    }

    destroyFbo();
}

void OledRenderer::createFbo()
{
    m_fbo = new QOpenGLFramebufferObject(m_size * m_dpr, QOpenGLFramebufferObject::CombinedDepthStencil);
    m_quickWindow->setRenderTarget(m_fbo);
}

void OledRenderer::destroyFbo()
{
    delete m_fbo;
    m_fbo = nullptr;
}

bool OledRenderer::loadQml(const QString &qmlFile, const QSize &size)
{
    if (m_qmlComponent != nullptr) {
        delete m_qmlComponent;
    }
    m_qmlComponent = new QQmlComponent(m_qmlEngine, QUrl(qmlFile), QQmlComponent::PreferSynchronous);

    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            qWarning() << error.url() << error.line() << error;
        return false;
    }

    QObject *rootObject = m_qmlComponent->create();
    if (m_qmlComponent->isError()) {
        const QList<QQmlError> errorList = m_qmlComponent->errors();
        for (const QQmlError &error : errorList)
            qWarning() << error.url() << error.line() << error;
        return false;
    }

    m_rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!m_rootItem) {
        qWarning("run: Not a QQuickItem");
        delete rootObject;
        return false;
    }

    // The root item is ready. Associate it with the window.
    m_rootItem->setParentItem(m_quickWindow->contentItem());

    m_rootItem->setWidth(size.width());
    m_rootItem->setHeight(size.height());

    m_quickWindow->setGeometry(0, 0, size.width(), size.height());

    return true;
}

void OledRenderer::renderNext()
{
    // Polish, synchronize and render the next frame (into our fbo).
    m_renderControl->polishItems();
    m_renderControl->sync();
    m_renderControl->render();

    m_context->functions()->glFlush();

    emit imageRendered(m_fbo->toImage());

    m_animationDriver->advance();
}

bool OledRenderer::isRunning()
{
    return m_status == Running;
}

QQuickItem *OledRenderer::rootItem()
{
    return m_rootItem;
}
