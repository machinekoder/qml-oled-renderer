#ifndef SSD1306DRIVER_H
#define SSD1306DRIVER_H

#include <QObject>
#include <QSize>
#include <QImage>

class Ssd1306Driver : public QObject
{
    Q_OBJECT
public:
    explicit Ssd1306Driver(QObject *parent = 0);

    bool openDevice(QSize size, int bus_id = 2, int address = 0x3c);
    void close();

public slots:
    void writeImage(const QImage &image);
    void clearScreen();

private:
    QSize m_size;
    int m_file;
};

#endif // SSD1306DRIVER_H
