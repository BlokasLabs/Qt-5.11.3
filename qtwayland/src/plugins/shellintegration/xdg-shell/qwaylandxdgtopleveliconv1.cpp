#include "qwaylandxdgtopleveliconv1_p.h"

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandshmbackingstore_p.h>
#include <QtGui/QIcon>
#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE
namespace QtWaylandClient {

QWaylandXdgToplevelIconManagerV1::QWaylandXdgToplevelIconManagerV1(QWaylandDisplay *display,
                                                               uint32_t id, uint32_t version)
    : QtWayland::xdg_toplevel_icon_manager_v1(display->wl_registry(), id, qMin(version, 1u))
    , m_display(display)
{
}

QWaylandXdgToplevelIconManagerV1::~QWaylandXdgToplevelIconManagerV1()
{
    destroy();
}

void QWaylandXdgToplevelIconManagerV1::xdg_toplevel_icon_manager_v1_icon_size(int32_t size)
{
    if (size > 0 && size <= 256 && !m_sizes.contains(size))
        m_sizes.append(size);
}

void QWaylandXdgToplevelIconManagerV1::setWindowIcon(::xdg_toplevel *toplevel,
                                                  const QIcon &icon, int scale)
{
    if (icon.isNull()) {
        set_icon(toplevel, nullptr);
        return;
    }
    if (!m_display->shm())
        return;

    // The compositor may not have delivered its preferred sizes yet on the
    // first window. Supply useful scalable-icon sizes in that case.
    const QVector<int> sizes = m_sizes.isEmpty() ? QVector<int>{32, 64, 128} : m_sizes;
    scale = qBound(1, scale, 4);
    QtWayland::xdg_toplevel_icon_v1 protocolIcon(create_icon());
    QVector<QWaylandShmBuffer *> buffers;
    for (int size : sizes) {
        const int pixels = size * scale;
        QImage image = icon.pixmap(QSize(pixels, pixels)).toImage();
        if (image.isNull())
            continue;
        image.setDevicePixelRatio(1);
        image = image.scaled(pixels, pixels, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        auto *buffer = new QWaylandShmBuffer(m_display, QSize(pixels, pixels),
                                            QImage::Format_ARGB32_Premultiplied, scale);
        if (!buffer->buffer() || buffer->image()->isNull()) {
            delete buffer;
            continue;
        }
        buffer->image()->fill(Qt::transparent);
        {
            QPainter painter(buffer->image());
            const QSizeF logicalSize = QSizeF(image.size()) / scale;
            const QPointF position((size - logicalSize.width()) / 2,
                                   (size - logicalSize.height()) / 2);
            painter.drawImage(QRectF(position, logicalSize), image);
        }
        protocolIcon.add_buffer(buffer->buffer(), scale);
        buffers.append(buffer);
    }
    if (!buffers.isEmpty())
        set_icon(toplevel, protocolIcon.object());

    // set_icon copies the icon state. Destroy the icon before its immutable
    // buffers, as required by the protocol; wl_buffer.release is not used.
    protocolIcon.destroy();
    qDeleteAll(buffers);
}

} // namespace QtWaylandClient
QT_END_NAMESPACE
