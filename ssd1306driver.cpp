#include "ssd1306driver.h"
#include <QVector>
#include <QRgb>
#include <stdint.h>

extern "C" {
    int i2c_open(int bus);
    int i2c_select(int file, int addr);
    int ssd1306_init(int file, int col, int line);
    int ssd1306_cls(int file, int col, int line);
    int i2c_write_data(int file, uint8_t data[], size_t len);
}

Ssd1306Driver::Ssd1306Driver(QObject *parent)
    : QObject(parent)
    , m_file(-1)
{

}

bool Ssd1306Driver::openDevice(QSize size, int busId, int address)
{
    m_file = i2c_open(busId);
    if (m_file < 0) {
        return false;
    }

    int res = i2c_select(m_file, address);
    if (res < 0) {
        return false;
    }

    ssd1306_init(m_file, size.width(), size.height());
    ssd1306_cls(m_file, size.width(), size.height()); // SSD1306 may have a SRAM-based GDDRAM, some parts of the graphic are perserved after power cycle.

    m_size = size;

    return true;
}

void Ssd1306Driver::clearScreen()
{
    if (m_file > -1) {
        ssd1306_cls(m_file, m_size.width(), m_size.height());
    }
}

void Ssd1306Driver::close()
{
    m_file = -1; // TODO: close file ?
}

void Ssd1306Driver::writeImage(const QImage &image)
{
    const uint8_t SSD1306_CONT_DATA_HDR = 0x40;

    if (m_file < 0) {
        return;
    }

    int height = m_size.height() / 8;
    int width = m_size.width();
    int len = height * width + 1;

    QVector<uint8_t> vector(len);
    int pos = 0;
    vector[pos] = SSD1306_CONT_DATA_HDR;
    pos++;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t pixel = 0u;
            for (int i = 0; i < 8; ++i) {
                pixel |= static_cast<uint8_t>(image.pixelIndex(x, y * 8 + i) == 1) << i;
            }
            vector[pos] = pixel;
            pos++;
        }
    }

    i2c_write_data(m_file, vector.data(), static_cast<size_t>(vector.size()));
}
