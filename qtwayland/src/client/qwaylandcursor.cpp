/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandcursor_p.h"

#include "qwaylanddisplay_p.h"
#include "qwaylandwindow_p.h"
#include "qwaylandinputdevice_p.h"
#include "qwaylandscreen_p.h"
#include "qwaylandshmbackingstore_p.h"

#include <QtGui/QImageReader>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QDebug>

#include <wayland-cursor.h>
#include "wayland-cursor-shape-v1-client-protocol.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandCursor::QWaylandCursor(QWaylandScreen *screen)
    : mDisplay(screen->display()), mScreen(screen)
{
    //TODO: Make wl_cursor_theme_load arguments configurable here
    QByteArray cursorTheme = qgetenv("XCURSOR_THEME");
    if (cursorTheme.isEmpty())
        cursorTheme = QByteArray("default");
    QByteArray cursorSizeFromEnv = qgetenv("XCURSOR_SIZE");
    bool hasCursorSize = false;
    int cursorSize = cursorSizeFromEnv.toInt(&hasCursorSize);
    if (!hasCursorSize || cursorSize <= 0)
        cursorSize = 32;
    mCursorTheme = wl_cursor_theme_load(cursorTheme, cursorSize, mDisplay->shm()->object());
    if (!mCursorTheme)
        qDebug() << "Could not load theme" << cursorTheme;
}

QWaylandCursor::~QWaylandCursor()
{
    if (mCursorTheme)
        wl_cursor_theme_destroy(mCursorTheme);
}

struct wl_cursor_image *QWaylandCursor::cursorImage(Qt::CursorShape newShape)
{
    struct wl_cursor *waylandCursor = nullptr;

    /* Hide cursor */
    if (newShape == Qt::BlankCursor)
    {
        mDisplay->setCursor(nullptr, nullptr);
        return nullptr;
    }

    if (newShape < Qt::BitmapCursor) {
        waylandCursor = requestCursor((WaylandCursor)newShape);
    } else if (newShape == Qt::BitmapCursor) {
        // cannot create a wl_cursor_image for a CursorShape
        return nullptr;
    } else {
        //TODO: Custom cursor logic (for resize arrows)
    }

    if (!waylandCursor) {
        qDebug("Could not find cursor for shape %d", newShape);
        return nullptr;
    }

    struct wl_cursor_image *image = waylandCursor->images[0];
    struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    if (!buffer) {
        qDebug("Could not find buffer for cursor");
        return nullptr;
    }

    return image;
}

QSharedPointer<QWaylandBuffer> QWaylandCursor::cursorBitmapImage(const QCursor *cursor)
{
    if (cursor->shape() != Qt::BitmapCursor)
        return QSharedPointer<QWaylandShmBuffer>();

    const QImage &img = cursor->pixmap().toImage();
    QSharedPointer<QWaylandShmBuffer> buffer(new QWaylandShmBuffer(mDisplay, img.size(), img.format()));
    memcpy(buffer->image()->bits(), img.bits(), img.sizeInBytes());
    return buffer;
}

uint32_t QWaylandCursor::protocolShape(Qt::CursorShape shape)
{
    switch (int(shape)) {
    case ArrowCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    case CrossCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR;
    case WaitCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT;
    case IBeamCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT;
    case SizeVerCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE;
    case SizeHorCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE;
    case SizeBDiagCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE;
    case SizeFDiagCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE;
    case SizeAllCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_SCROLL;
    case SplitVCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ROW_RESIZE;
    case SplitHCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COL_RESIZE;
    case PointingHandCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER;
    case ForbiddenCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED;
    case WhatsThisCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_HELP;
    case BusyCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_PROGRESS;
    case OpenHandCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRAB;
    case ClosedHandCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRABBING;
    case DragCopyCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COPY;
    case DragMoveCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_MOVE;
    case DragLinkCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALIAS;
    case ResizeNorthCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_N_RESIZE;
    case ResizeSouthCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_S_RESIZE;
    case ResizeEastCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_E_RESIZE;
    case ResizeWestCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_W_RESIZE;
    case ResizeNorthWestCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NW_RESIZE;
    case ResizeSouthEastCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SE_RESIZE;
    case ResizeNorthEastCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NE_RESIZE;
    case ResizeSouthWestCursor: return WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SW_RESIZE;
    default: return 0; // Blank, bitmap and shapes without a protocol equivalent.
    }
}

void QWaylandCursor::changeCursor(QCursor *cursor, QWindow *window)
{
    const QCursor arrow(Qt::ArrowCursor);
    QWaylandScreen *screen = window
            ? static_cast<QWaylandScreen *>(window->screen()->handle()) : mScreen;
    for (QWaylandInputDevice *device : mDisplay->inputDevices()) {
        // Restoring an override visits every window. Only the focused window
        // may choose this seat's cursor; another window may still have a hand
        // or drag cursor set on it.
        if (window && !QGuiApplication::overrideCursor()
                && (!device->pointerFocus() || device->pointerFocus()->window() != window))
            continue;
        device->setCursor(cursor ? *cursor : arrow, screen);
    }
}

void QWaylandCursor::pointerEvent(const QMouseEvent &event)
{
    mLastPos = event.globalPos();
}

QPoint QWaylandCursor::pos() const
{
    return mLastPos;
}

void QWaylandCursor::setPos(const QPoint &pos)
{
    Q_UNUSED(pos);
    qWarning() << "QWaylandCursor::setPos: not implemented";
}

wl_cursor *QWaylandCursor::requestCursor(WaylandCursor shape)
{
    struct wl_cursor *cursor = mCursors.value(shape, 0);

    //If the cursor has not been loaded already, load it
    if (!cursor) {
        if (!mCursorTheme)
            return nullptr;

        // Alias priority follows declaration order, including aliases for
        // the same shape. QMultiMap would reverse their insertion order.
        static const struct ShapeAndName {
            WaylandCursor shape;
            const char *name;
        } cursorNames[] = {
            {ArrowCursor, "left_ptr"},
            {ArrowCursor, "default"},
            {ArrowCursor, "top_left_arrow"},
            {ArrowCursor, "left_arrow"},

            {UpArrowCursor, "up_arrow"},

            {CrossCursor, "cross"},

            {WaitCursor, "wait"},
            {WaitCursor, "watch"},
            {WaitCursor, "0426c94ea35c87780ff01dc239897213"},

            {IBeamCursor, "ibeam"},
            {IBeamCursor, "text"},
            {IBeamCursor, "xterm"},

            {SizeVerCursor, "size_ver"},
            {SizeVerCursor, "ns-resize"},
            {SizeVerCursor, "v_double_arrow"},
            {SizeVerCursor, "00008160000006810000408080010102"},

            {SizeHorCursor, "size_hor"},
            {SizeHorCursor, "ew-resize"},
            {SizeHorCursor, "h_double_arrow"},
            {SizeHorCursor, "028006030e0e7ebffc7f7070c0600140"},

            {SizeBDiagCursor, "size_bdiag"},
            {SizeBDiagCursor, "nesw-resize"},
            {SizeBDiagCursor, "50585d75b494802d0151028115016902"},
            {SizeBDiagCursor, "fcf1c3c7cd4491d801f1e1c78f100000"},

            {SizeFDiagCursor, "size_fdiag"},
            {SizeFDiagCursor, "nwse-resize"},
            {SizeFDiagCursor, "38c5dff7c7b8962045400281044508d2"},
            {SizeFDiagCursor, "c7088f0f3e6c8088236ef8e1e3e70000"},

            {SizeAllCursor, "size_all"},

            {SplitVCursor, "split_v"},
            {SplitVCursor, "row-resize"},
            {SplitVCursor, "sb_v_double_arrow"},
            {SplitVCursor, "2870a09082c103050810ffdffffe0204"},
            {SplitVCursor, "c07385c7190e701020ff7ffffd08103c"},

            {SplitHCursor, "split_h"},
            {SplitHCursor, "col-resize"},
            {SplitHCursor, "sb_h_double_arrow"},
            {SplitHCursor, "043a9f68147c53184671403ffa811cc5"},
            {SplitHCursor, "14fef782d02440884392942c11205230"},

            {PointingHandCursor, "pointer"},
            {PointingHandCursor, "pointing_hand"},
            {PointingHandCursor, "hand2"},
            {PointingHandCursor, "e29285e634086352946a0e7090d73106"},

            {ForbiddenCursor, "forbidden"},
            {ForbiddenCursor, "not-allowed"},
            {ForbiddenCursor, "crossed_circle"},
            {ForbiddenCursor, "circle"},
            {ForbiddenCursor, "03b6e0fcb3499374a867c041f52298f0"},

            {WhatsThisCursor, "whats_this"},
            {WhatsThisCursor, "help"},
            {WhatsThisCursor, "question_arrow"},
            {WhatsThisCursor, "5c6cd98b3f3ebcb1f9c7f1c204630408"},
            {WhatsThisCursor, "d9ce0ab605698f320427677b458ad60b"},

            {BusyCursor, "left_ptr_watch"},
            {BusyCursor, "half-busy"},
            {BusyCursor, "progress"},
            {BusyCursor, "00000000000000020006000e7e9ffc3f"},
            {BusyCursor, "08e8e1c95fe2fc01f976f1e063a24ccd"},

            {OpenHandCursor, "openhand"},
            {OpenHandCursor, "fleur"},
            {OpenHandCursor, "5aca4d189052212118709018842178c0"},
            {OpenHandCursor, "9d800788f1b08800ae810202380a0822"},

            {ClosedHandCursor, "closedhand"},
            {ClosedHandCursor, "grabbing"},
            {ClosedHandCursor, "208530c400c041818281048008011002"},

            {DragCopyCursor, "dnd-copy"},
            {DragCopyCursor, "copy"},

            {DragMoveCursor, "dnd-move"},
            {DragMoveCursor, "move"},

            {DragLinkCursor, "dnd-link"},
            {DragLinkCursor, "link"},

            {ResizeNorthCursor, "n-resize"},
            {ResizeNorthCursor, "top_side"},

            {ResizeSouthCursor, "s-resize"},
            {ResizeSouthCursor, "bottom_side"},

            {ResizeEastCursor, "e-resize"},
            {ResizeEastCursor, "right_side"},

            {ResizeWestCursor, "w-resize"},
            {ResizeWestCursor, "left_side"},

            {ResizeNorthWestCursor, "nw-resize"},
            {ResizeNorthWestCursor, "top_left_corner"},

            {ResizeSouthEastCursor, "se-resize"},
            {ResizeSouthEastCursor, "bottom_right_corner"},

            {ResizeNorthEastCursor, "ne-resize"},
            {ResizeNorthEastCursor, "top_right_corner"},

            {ResizeSouthWestCursor, "sw-resize"},
            {ResizeSouthWestCursor, "bottom_left_corner"},
        };
        for (const auto &entry : cursorNames) {
            if (entry.shape != shape)
                continue;
            cursor = wl_cursor_theme_get_cursor(mCursorTheme, entry.name);
            if (cursor) {
                mCursors.insert(shape, cursor);
                break;
            }
        }
    }

    //If there still no cursor for a shape, use the default cursor
    if (!cursor && shape != ArrowCursor) {
        cursor = requestCursor(ArrowCursor);
    }

    return cursor;
}



}

QT_END_NAMESPACE
