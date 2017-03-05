#include <QGuiApplication>
#include <QCommandLineParser>
#include <QStringList>
#include "oledrenderer.h"
#include "ssd1306driver.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qApp->setApplicationName("QML OLED Renderer");

    QCommandLineParser parser;
    parser.setApplicationDescription("Renders QML applications to a SSD1306 OLED display");
    parser.addHelpOption();
    parser.addPositionalArgument("source", "QML source file");
    parser.addOptions({
                          {{"w", "width"}, "OLED screen width", "weight"},
                          {{"h", "height"}, "OLED screen height", "height"},
                          {{"b", "bus"}, "I2C bus to which the OLED is connected", "bus"},
                          {{"a", "address"}, "I2C address of the OLED screen", "address"},
                          {{"f", "fps"}, "Number of frames to render per second", "fps"}
                      });

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.length() < 1) {
        qCritical() << "please specify a source file";
        return -1;
    }
    QString sourceFile = args.first();

    int width = parser.isSet("w") ? parser.value("w").toInt() : 128;
    int height = parser.isSet("h") ? parser.value("h").toInt() : 64;
    int bus = parser.isSet("b") ? parser.value("b").toInt() : 2;
    int address = parser.isSet("a") ? parser.value("a").toInt() : 0x3c;
    int fps = parser.isSet("f") ? parser.value("f").toInt() : 10;

    Ssd1306Driver driver;
    if (!driver.openDevice(QSize(width, height), bus, address)) {
        qCritical() << "cannot open OLED display";
        return -1;
    }
    QObject::connect(qApp, &QGuiApplication::aboutToQuit, &driver, &Ssd1306Driver::clearScreen);

    OledRenderer renderer;
    QObject::connect(&renderer, &OledRenderer::imageRendered, [&driver](const QImage &image) {
       const auto mono = image.convertToFormat(QImage::Format_Mono, Qt::MonoOnly | Qt::ThresholdDither);
       driver.writeImage(mono);
    });
    renderer.loadQmlFile(sourceFile, QSize(width, height), 1.0, fps);

    return app.exec();
}
