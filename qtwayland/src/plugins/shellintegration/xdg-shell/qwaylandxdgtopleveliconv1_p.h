#ifndef QWAYLANDXDGTOPLEVELICONV1_P_H
#define QWAYLANDXDGTOPLEVELICONV1_P_H

// Private client implementation of xdg-toplevel-icon-v1.
#include "qwayland-xdg-toplevel-icon-v1.h"
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QIcon;

namespace QtWaylandClient {

class QWaylandDisplay;

class QWaylandXdgToplevelIconManagerV1 : public QtWayland::xdg_toplevel_icon_manager_v1
{
public:
    QWaylandXdgToplevelIconManagerV1(QWaylandDisplay *display, uint32_t id, uint32_t version);
    ~QWaylandXdgToplevelIconManagerV1() override;
    void setWindowIcon(::xdg_toplevel *toplevel, const QIcon &icon, int scale);

protected:
    void xdg_toplevel_icon_manager_v1_icon_size(int32_t size) override;

private:
    QWaylandDisplay *m_display;
    QVector<int> m_sizes;
};

} // namespace QtWaylandClient
QT_END_NAMESPACE

#endif
