#include <QTimer>
#include <QSettings>
#include <QPainter>
#include <QMenu>
#include <QPaintEvent>
#include <math.h>
#include <stdlib.h>
#include <qmmp/qmmp.h>

#include "fft.h"
#include "inlines.h"
#include "plusblurxrays.h"
#include "colorwidget.h"

PlusBlurXRays::PlusBlurXRays (QWidget *parent)
    : Visual(parent)
{
    m_intern_vis_data = nullptr;
    m_running = false;
    m_rows = 0;
    m_cols = 0;
    m_image_size = 0;
    m_image = nullptr;
    m_corner = nullptr;

    setWindowTitle(tr("Plus Blur XRays Widget"));
    setMinimumSize(2*300-30, 105);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->setInterval(QMMP_VISUAL_INTERVAL);

    m_screenAction = new QAction(tr("Fullscreen"), this);
    m_screenAction->setCheckable(true);
    connect(m_screenAction, SIGNAL(triggered(bool)), this, SLOT(changeFullScreen(bool)));

    clear();
    readSettings();
}

PlusBlurXRays::~PlusBlurXRays()
{
    if(m_intern_vis_data)
    {
        delete[] m_intern_vis_data;
    }
    if(m_image)
    {
        delete[] m_image;
    }
}

void PlusBlurXRays::start()
{
    m_running = true;
    if(isVisible())
    {
        m_timer->start();
    }
}

void PlusBlurXRays::stop()
{
    m_running = false;
    m_timer->stop();
    clear();
}

void PlusBlurXRays::clear()
{
    m_rows = 0;
    m_cols = 0;
    update();
}

void PlusBlurXRays::timeout()
{
    if(takeData(m_left_buffer, m_right_buffer))
    {
        process();
        update();
    }
}

void PlusBlurXRays::readSettings()
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("PlusBlurXRays");
    m_colors = ColorWidget::readColorConfig(settings.value("colors").toString());
    settings.endGroup();
}

void PlusBlurXRays::writeSettings()
{
    QSettings settings(Qmmp::configFile(), QSettings::IniFormat);
    settings.beginGroup("PlusBlurXRays");
    settings.setValue("colors", ColorWidget::writeColorConfig(m_colors));
    settings.endGroup();
}

void PlusBlurXRays::changeColor()
{
    ColorWidget c;
    c.setColors(m_colors);
    if(c.exec())
    {
        m_colors = c.getColors();
    }
}

void PlusBlurXRays::hideEvent(QHideEvent *)
{
    m_timer->stop();
}

void PlusBlurXRays::showEvent(QShowEvent *)
{
    if(m_running)
    {
        m_timer->start();
    }
}

void PlusBlurXRays::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(e->rect(), Qt::black);
    draw(&painter);
}

void PlusBlurXRays::contextMenuEvent(QContextMenuEvent *)
{
    QMenu menu(this);
    connect(&menu, SIGNAL(triggered(QAction*)), SLOT(writeSettings()));
    connect(&menu, SIGNAL(triggered(QAction*)), SLOT(readSettings()));

    menu.addAction(m_screenAction);
    menu.addSeparator();
    menu.addAction(tr("Color"), this, SLOT(changeColor()));
    menu.exec(QCursor::pos());
}

void PlusBlurXRays::blur()
{
    for(int y = 0; y < m_rows; y ++)
    {
        uint32_t * p = m_corner + m_cols * y;
        uint32_t * end = p + m_cols;
        uint32_t * plast = p - m_cols;
        uint32_t * pnext = p + m_cols;

        /* We do a quick and dirty average of four color values, first masking
         * off the lowest two bits.  Over a large area, this masking has the net
         * effect of subtracting 1.5 from each value, which by a happy chance
         * is just right for a gradual fade effect. */
        for(; p < end; p ++)
        {
            *p = ((*plast ++ &0xFCFCFC) + (p[-1] & 0xFCFCFC) + (p[1] & 0xFCFCFC) + (*pnext ++ &0xFCFCFC)) >> 2;
        }
    }
}

void PlusBlurXRays::process()
{
    static fft_state *state = nullptr;
    if(!state)
    {
        state = fft_init();
    }

    const int rows = height();
    const int cols = width();

    if(m_rows != rows || m_cols != cols)
    {
        m_rows = rows;
        m_cols = cols;

        if(m_intern_vis_data)
        {
            delete[] m_intern_vis_data;
        }

        m_intern_vis_data = new int[m_cols]{0};

        m_image_size = (m_cols << 2) * (m_rows + 2);
        if(m_image)
        {
            delete[] m_image;
        }
        m_image = new unsigned int[m_image_size]{0};
        m_corner = m_image + m_cols + 1;
    }

    const int step = (QMMP_VISUAL_NODE_SIZE << 8) / m_cols;
    int pos = 0;

    for(int i = 0; i < m_cols; ++i)
    {
        pos += step;
        m_intern_vis_data[i] = int(m_left_buffer[pos >> 8] * m_rows / 2);
        m_intern_vis_data[i] = qBound(-m_rows / 2, m_intern_vis_data[i], m_rows / 2);
    }
}

void PlusBlurXRays::drawLine(int x, int y1, int y2)
{
    int y, h;

    if(y1 < y2)
    {
        y = y1 + 1;
        h = y2 - y1;
    }
    else if(y2 < y1)
    {
        y = y2;
        h = y1 - y2;
    }
    else
    {
        y = y1;
        h = 1;
    }

    unsigned int *p = m_corner + y * m_cols + x;

    for(; h--; p += m_cols)
    {
        *p = m_colors.isEmpty() ? 0xFF3F7F : m_colors.first().rgba();
    }
}

void PlusBlurXRays::draw(QPainter *p)
{
    if(m_cols == 0)
    {
        return;
    }

    blur();

    const float maxed = takeMaxRange();
    int value = m_rows / 2 - m_intern_vis_data[0] * maxed;
    value = qBound(0, value, m_rows - 1);

    for(int i = 0; i < m_cols; i++)
    {
        int y = m_rows / 2 - m_intern_vis_data[i] * maxed;
        y = qBound(0, y, m_rows - 1);
        drawLine(i, value, y);
        value = y;
    }

    p->drawImage(0, 0, QImage((unsigned char *)m_image, m_cols, m_rows, QImage::Format_RGB32));
}
