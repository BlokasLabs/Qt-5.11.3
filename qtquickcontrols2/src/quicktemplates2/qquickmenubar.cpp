/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickmenubar_p.h"
#include "qquickmenubar_p_p.h"
#include "qquickmenubaritem_p_p.h"
#include "qquickmenu_p.h"
#include "qquickmenu_p_p.h"

#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlengine.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype MenuBar
    \inherits Container
    \instantiates QQuickMenuBar
    \inqmlmodule QtQuick.Controls
    \since 5.10
    \ingroup qtquickcontrols2-menus
    \ingroup qtquickcontrols2-focusscopes
    \brief Provides a window menu bar.

    \image qtquickcontrols2-menubar.png

    MenuBar consists of drop-down menus, and is normally located at the top
    edge of the window.

    \quotefromfile qtquickcontrols2-menubar.qml
    \skipuntil begin
    \printto skipfrom
    \skipuntil skipto
    \printto end

    Typically, menus are statically declared as children of the menu bar, but
    MenuBar also provides API to \l {addMenu}{add}, \l {insertMenu}{insert},
    \l {removeMenu}{remove}, and \l {takeMenu}{take} menus dynamically. The
    menus in a menu bar can be accessed using \l menuAt().

    \sa {Customizing MenuBar}, Menu, MenuBarItem, {Menu Controls},
        {Focus Management in Qt Quick Controls 2}
*/

void QQuickMenuBarPrivate::createItems()
{
    // removeItem() will remove stuff from contentData, so we have to make a copy of it.
    const auto originalContentData = contentData;
    // Sanity check that there aren't any items we don't know about.
    Q_ASSERT(contentModel->count() == 0);

    for (QObject *object : originalContentData) {
        if (QQuickMenu *menu = qobject_cast<QQuickMenu *>(object)) {
            // It's a QQuickMenu; create a QQuickMenuBarItem for it.
            QQuickItem *menuItem = createItem(menu);
            addObject(menuItem);
        } else if (qobject_cast<QQuickMenuBarItem *>(object)) {
            addObject(object);
        }
        // If it's neither, skip it because we don't care about it.
    }
}

QQuickItem *QQuickMenuBarPrivate::beginCreateItem()
{
    Q_Q(QQuickMenuBar);
    if (!delegate)
        return nullptr;

    QQmlContext *creationContext = delegate->creationContext();
    if (!creationContext)
        creationContext = qmlContext(q);
    QQmlContext *context = new QQmlContext(creationContext, q);
    context->setContextObject(q);

    QObject *object = delegate->beginCreate(context);
    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        delete object;
        delete context;
        return nullptr;
    }

    item->setParentItem(q);
    QQml_setParent_noEvent(item, q);

    return item;
}

void QQuickMenuBarPrivate::completeCreateItem()
{
    if (!delegate)
        return;

    delegate->completeCreate();
}

QQuickItem *QQuickMenuBarPrivate::createItem(QQuickMenu *menu)
{
    QQuickItem *item = beginCreateItem();
    if (QQuickMenuBarItem *menuBarItem = qobject_cast<QQuickMenuBarItem *>(item))
        menuBarItem->setMenu(menu);
    completeCreateItem();
    return item;
}

void QQuickMenuBarPrivate::toggleCurrentMenu(bool visible, bool activate)
{
    if (!currentItem || visible == popupMode)
        return;

    QQuickMenu *menu = currentItem->menu();

    triggering = true;
    popupMode = visible;
    if (menu)
        menu->setVisible(visible);
    if (!visible)
        currentItem->forceActiveFocus();
    else if (menu && activate)
        menu->setCurrentIndex(0);
    triggering = false;
}

void QQuickMenuBarPrivate::activateItem(QQuickMenuBarItem *item)
{
    if (currentItem == item)
        return;

    if (currentItem) {
        currentItem->setHighlighted(false);
        if (popupMode) {
            if (QQuickMenu *menu = currentItem->menu())
                menu->dismiss();
        }
    }

    if (item) {
        item->setHighlighted(true);
        if (popupMode) {
            if (QQuickMenu *menu = item->menu())
                menu->open();
        }
    }

    currentItem = item;
}

void QQuickMenuBarPrivate::activateNextItem()
{
    int index = currentItem ? contentModel->indexOf(currentItem, nullptr) : -1;
    if (index >= contentModel->count() - 1)
        index = -1;
    activateItem(qobject_cast<QQuickMenuBarItem *>(itemAt(++index)));
}

void QQuickMenuBarPrivate::activatePreviousItem()
{
    int index = currentItem ? contentModel->indexOf(currentItem, nullptr) : contentModel->count();
    if (index <= 0)
        index = contentModel->count();
    activateItem(qobject_cast<QQuickMenuBarItem *>(itemAt(--index)));
}

void QQuickMenuBarPrivate::onItemHovered()
{
    Q_Q(QQuickMenuBar);
    QQuickMenuBarItem *item = qobject_cast<QQuickMenuBarItem *>(q->sender());
    if (!item || item == currentItem || !item->isHovered() || QQuickMenuBarItemPrivate::get(item)->touchId != -1)
        return;

    activateItem(item);
}

void QQuickMenuBarPrivate::onItemTriggered()
{
    Q_Q(QQuickMenuBar);
    QQuickMenuBarItem *item = qobject_cast<QQuickMenuBarItem *>(q->sender());
    if (!item)
        return;

    if (item == currentItem) {
        toggleCurrentMenu(!popupMode, false);
    } else {
        popupMode = true;
        activateItem(item);
    }
}

void QQuickMenuBarPrivate::onMenuAboutToHide()
{
    if (triggering || !currentItem || currentItem->isHovered() || !currentItem->isHighlighted())
        return;

    popupMode = false;
    activateItem(nullptr);
}

void QQuickMenuBarPrivate::updateContentSize()
{
    Q_Q(QQuickMenuBar);
    if (hasContentWidth && hasContentHeight)
        return;

    const int count = contentModel->count();
    if (count <= 0 || !contentItem)
        return;

    qreal maxHeight = 0;
    qreal totalWidth = qMax(0, count - 1) * spacing;

    for (int i = 0; i < count; ++i) {
        QQuickItem *item = q->itemAt(i);
        if (item) {
            totalWidth += item->width();
            maxHeight = qMax(maxHeight, item->implicitHeight());
        }
    }

    bool contentWidthChange = false;
    if (!hasContentWidth && !qFuzzyCompare(contentWidth, totalWidth)) {
        contentWidth = totalWidth;
        contentWidthChange = true;
    }

    bool contentHeightChange = false;
    if (!hasContentHeight && !qFuzzyCompare(contentHeight, maxHeight)) {
        contentHeight = maxHeight;
        contentHeightChange = true;
    }

    if (contentWidthChange)
        emit q->contentWidthChanged();
    if (contentHeightChange)
        emit q->contentHeightChanged();
}

void QQuickMenuBarPrivate::itemGeometryChanged(QQuickItem *, QQuickGeometryChange change, const QRectF &)
{
    if ((change.widthChange() && !hasContentWidth) || (change.heightChange() && !hasContentHeight))
        updateContentSize();
}

void QQuickMenuBarPrivate::contentData_append(QQmlListProperty<QObject> *prop, QObject *obj)
{
    QQuickMenuBar *menuBar = static_cast<QQuickMenuBar *>(prop->object);
    QQuickMenuBarPrivate *menuBarPrivate = QQuickMenuBarPrivate::get(menuBar);
    if (!menuBarPrivate->componentComplete) {
        // Don't add items until we're complete, as the delegate could change in the meantime.
        // We'll add it to contentData and create it when we're complete.
        menuBarPrivate->contentData.append(obj);
        return;
    }

    if (QQuickMenu *menu = qobject_cast<QQuickMenu *>(obj)) {
        QQuickItem *menuItem = menuBarPrivate->createItem(menu);
        menuBarPrivate->addObject(menuItem);
    }
}

void QQuickMenuBarPrivate::menus_append(QQmlListProperty<QQuickMenu> *prop, QQuickMenu *obj)
{
    QQuickMenuBar *menuBar = static_cast<QQuickMenuBar *>(prop->object);
    menuBar->addMenu(obj);
}

int QQuickMenuBarPrivate::menus_count(QQmlListProperty<QQuickMenu> *prop)
{
    QQuickMenuBar *menuBar = static_cast<QQuickMenuBar *>(prop->object);
    return menuBar->count();
}

QQuickMenu *QQuickMenuBarPrivate::menus_at(QQmlListProperty<QQuickMenu> *prop, int index)
{
    QQuickMenuBar *menuBar = static_cast<QQuickMenuBar *>(prop->object);
    return menuBar->menuAt(index);
}

void QQuickMenuBarPrivate::menus_clear(QQmlListProperty<QQuickMenu> *prop)
{
    QQuickMenuBar *menuBar = static_cast<QQuickMenuBar *>(prop->object);
    QQuickMenuBarPrivate::get(menuBar)->contentModel->clear();
}

QQuickMenuBar::QQuickMenuBar(QQuickItem *parent)
    : QQuickContainer(*(new QQuickMenuBarPrivate), parent)
{
    Q_D(QQuickMenuBar);
    d->changeTypes |= QQuickItemPrivate::Geometry;
    setFlag(ItemIsFocusScope);
    setFocusPolicy(Qt::ClickFocus);
}

/*!
    \qmlproperty Component QtQuick.Controls::MenuBar::delegate

    This property holds the component that is used to create menu bar
    items to present menus in the menu bar.

    \sa MenuBarItem
*/
QQmlComponent *QQuickMenuBar::delegate() const
{
    Q_D(const QQuickMenuBar);
    return d->delegate;
}

void QQuickMenuBar::setDelegate(QQmlComponent *delegate)
{
    Q_D(QQuickMenuBar);
    if (d->delegate == delegate)
        return;

    d->delegate = delegate;
    emit delegateChanged();
}

/*!
    \qmlmethod Menu QtQuick.Controls::MenuBar::menuAt(int index)

    Returns the menu at \a index, or \c null if it does not exist.
*/
QQuickMenu *QQuickMenuBar::menuAt(int index) const
{
    Q_D(const QQuickMenuBar);
    QQuickMenuBarItem *item = qobject_cast<QQuickMenuBarItem *>(d->itemAt(index));
    if (!item)
        return nullptr;
    return item->menu();
}

/*!
    \qmlmethod void QtQuick.Controls::MenuBar::addMenu(Menu menu)

    Adds \a menu to the end of the list of menus.
*/
void QQuickMenuBar::addMenu(QQuickMenu *menu)
{
    Q_D(QQuickMenuBar);
    addItem(d->createItem(menu));
}

/*!
    \qmlmethod void QtQuick.Controls::MenuBar::insertMenu(int index, Menu menu)

    Inserts \a menu at \a index.
*/
void QQuickMenuBar::insertMenu(int index, QQuickMenu *menu)
{
    Q_D(QQuickMenuBar);
    insertItem(index, d->createItem(menu));
}

/*!
    \qmlmethod void QtQuick.Controls::MenuBar::removeMenu(Menu menu)

    Removes and destroys the specified \a menu.
*/
void QQuickMenuBar::removeMenu(QQuickMenu *menu)
{
    Q_D(QQuickMenuBar);
    if (!menu)
        return;

    const int count = d->contentModel->count();
    for (int i = 0; i < count; ++i) {
        QQuickMenuBarItem *item = qobject_cast<QQuickMenuBarItem *>(itemAt(i));
        if (!item || item->menu() != menu)
            continue;

        removeItem(item);
        break;
    }

    menu->deleteLater();
}

/*!
    \qmlmethod Menu QtQuick.Controls::MenuBar::takeMenu(int index)

    Removes and returns the menu at \a index.

    \note The ownership of the item is transferred to the caller.
*/
QQuickMenu *QQuickMenuBar::takeMenu(int index)
{
    Q_D(QQuickMenuBar);
    QQuickMenuBarItem *item = qobject_cast<QQuickMenuBarItem *>(itemAt(index));
    if (!item)
        return nullptr;

    QQuickMenu *menu = item->menu();
    if (!menu)
        return nullptr;

    d->removeItem(index, item);
    item->deleteLater();
    return menu;
}

/*!
    \qmlproperty real QtQuick.Controls::MenuBar::contentWidth

    This property holds the content width. It is used for calculating the total
    implicit width of the menu bar.

    Unless explicitly overridden, the content width is automatically calculated
    based on the total implicit width of the items and the \l {Control::}{spacing}
    of the menu bar.

    \sa contentHeight
*/
qreal QQuickMenuBar::contentWidth() const
{
    Q_D(const QQuickMenuBar);
    return d->contentWidth;
}

void QQuickMenuBar::setContentWidth(qreal width)
{
    Q_D(QQuickMenuBar);
    d->hasContentWidth = true;
    if (qFuzzyCompare(d->contentWidth, width))
        return;

    d->contentWidth = width;
    emit contentWidthChanged();
}

void QQuickMenuBar::resetContentWidth()
{
    Q_D(QQuickMenuBar);
    if (!d->hasContentWidth)
        return;

    d->hasContentWidth = false;
    if (isComponentComplete())
        d->updateContentSize();
}

/*!
    \qmlproperty real QtQuick.Controls::MenuBar::contentHeight

    This property holds the content height. It is used for calculating the total
    implicit height of the menu bar.

    Unless explicitly overridden, the content height is automatically calculated
    based on the maximum implicit height of the items.

    \sa contentWidth
*/
qreal QQuickMenuBar::contentHeight() const
{
    Q_D(const QQuickMenuBar);
    return d->contentHeight;
}

void QQuickMenuBar::setContentHeight(qreal height)
{
    Q_D(QQuickMenuBar);
    d->hasContentHeight = true;
    if (qFuzzyCompare(d->contentHeight, height))
        return;

    d->contentHeight = height;
    emit contentHeightChanged();
}

void QQuickMenuBar::resetContentHeight()
{
    Q_D(QQuickMenuBar);
    if (!d->hasContentHeight)
        return;

    d->hasContentHeight = false;
    if (isComponentComplete())
        d->updateContentSize();
}

/*!
    \qmlproperty list<Menu> QtQuick.Controls::MenuBar::menus

    This property holds the list of menus.

    The list contains all menus that have been declared in QML as children
    of the menu bar, and also menus that have been dynamically added or
    inserted using the \l addMenu() and \l insertMenu() methods, respectively.
*/
QQmlListProperty<QQuickMenu> QQuickMenuBar::menus()
{
    return QQmlListProperty<QQuickMenu>(this, nullptr,
                                        QQuickMenuBarPrivate::menus_append,
                                        QQuickMenuBarPrivate::menus_count,
                                        QQuickMenuBarPrivate::menus_at,
                                        QQuickMenuBarPrivate::menus_clear);
}

QQmlListProperty<QObject> QQuickMenuBar::contentData()
{
    return QQmlListProperty<QObject>(this, nullptr,
                                     QQuickMenuBarPrivate::contentData_append,
                                     QQuickContainerPrivate::contentData_count,
                                     QQuickContainerPrivate::contentData_at,
                                     QQuickContainerPrivate::contentData_clear);
}

void QQuickMenuBar::updatePolish()
{
    Q_D(QQuickMenuBar);
    QQuickContainer::updatePolish();
    d->updateContentSize();
}

void QQuickMenuBar::componentComplete()
{
    Q_D(QQuickMenuBar);
    QQuickContainer::componentComplete();
    d->createItems();
    d->updateContentSize();
}

bool QQuickMenuBar::eventFilter(QObject *object, QEvent *event)
{
    return QObject::eventFilter(object, event);
}

void QQuickMenuBar::keyPressEvent(QKeyEvent *event)
{
    Q_D(QQuickMenuBar);
    QQuickContainer::keyReleaseEvent(event);

    switch (event->key()) {
    case Qt::Key_Up:
        d->toggleCurrentMenu(false, false);
        break;

    case Qt::Key_Down:
        d->toggleCurrentMenu(true, true);
        break;

    case Qt::Key_Left:
    case Qt::Key_Right:
        if (isMirrored() == (event->key() == Qt::Key_Left))
            d->activateNextItem();
        else
            d->activatePreviousItem();
        break;
    case Qt::Key_Escape:
        if (d->currentItem) {
            d->activateItem(nullptr);
            setFocus(false);
        }
        break;
    default:
        break;
    }
}

void QQuickMenuBar::keyReleaseEvent(QKeyEvent *event)
{
    QQuickContainer::keyReleaseEvent(event);

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Escape:
        event->accept();
        break;

    default:
        event->ignore();
        break;
    }
}

void QQuickMenuBar::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QQuickMenuBar);
    QQuickContainer::hoverLeaveEvent(event);
    if (!d->popupMode && d->currentItem)
        d->activateItem(nullptr);
}

bool QQuickMenuBar::isContent(QQuickItem *item) const
{
    return qobject_cast<QQuickMenuBarItem *>(item);
}

void QQuickMenuBar::itemAdded(int index, QQuickItem *item)
{
    Q_D(QQuickMenuBar);
    QQuickContainer::itemAdded(index, item);
    if (QQuickMenuBarItem *menuBarItem = qobject_cast<QQuickMenuBarItem *>(item)) {
        QQuickMenuBarItemPrivate::get(menuBarItem)->setMenuBar(this);
        QObjectPrivate::connect(menuBarItem, &QQuickControl::hoveredChanged, d, &QQuickMenuBarPrivate::onItemHovered);
        QObjectPrivate::connect(menuBarItem, &QQuickMenuBarItem::triggered, d, &QQuickMenuBarPrivate::onItemTriggered);
        if (QQuickMenu *menu = menuBarItem->menu())
            QObjectPrivate::connect(menu, &QQuickPopup::aboutToHide, d, &QQuickMenuBarPrivate::onMenuAboutToHide);
    }
    if (isComponentComplete())
        polish();
    if (isComponentComplete())
        polish();
    emit menusChanged();
}

void QQuickMenuBar::itemMoved(int index, QQuickItem *item)
{
    QQuickContainer::itemMoved(index, item);
    emit menusChanged();
}

void QQuickMenuBar::itemRemoved(int index, QQuickItem *item)
{
    Q_D(QQuickMenuBar);
    QQuickContainer::itemRemoved(index, item);
    if (QQuickMenuBarItem *menuBarItem = qobject_cast<QQuickMenuBarItem *>(item)) {
        QQuickMenuBarItemPrivate::get(menuBarItem)->setMenuBar(nullptr);
        QObjectPrivate::disconnect(menuBarItem, &QQuickControl::hoveredChanged, d, &QQuickMenuBarPrivate::onItemHovered);
        QObjectPrivate::disconnect(menuBarItem, &QQuickMenuBarItem::triggered, d, &QQuickMenuBarPrivate::onItemTriggered);
        if (QQuickMenu *menu = menuBarItem->menu())
            QObjectPrivate::disconnect(menu, &QQuickPopup::aboutToHide, d, &QQuickMenuBarPrivate::onMenuAboutToHide);
    }
    emit menusChanged();
}

QFont QQuickMenuBar::defaultFont() const
{
    return QQuickControlPrivate::themeFont(QPlatformTheme::MenuBarFont);
}

QPalette QQuickMenuBar::defaultPalette() const
{
    return QQuickControlPrivate::themePalette(QPlatformTheme::MenuBarPalette);
}

#if QT_CONFIG(accessibility)
QAccessible::Role QQuickMenuBar::accessibleRole() const
{
    return QAccessible::MenuBar;
}
#endif

QT_END_NAMESPACE
