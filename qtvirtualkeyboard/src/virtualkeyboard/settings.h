/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QUrl>

namespace QtVirtualKeyboard {

class SettingsPrivate;

class Settings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Settings)
    Q_DECLARE_PRIVATE(Settings)

    Settings(QObject *parent = 0);

public:
    static Settings *instance();

    QString style() const;
    void setStyle(const QString &style);

    QString styleName() const;
    void setStyleName(const QString &name);

    QString locale() const;
    void setLocale(const QString &locale);

    QStringList availableLocales() const;
    void setAvailableLocales(const QStringList &availableLocales);

    QStringList activeLocales() const;
    void setActiveLocales(const QStringList &activeLocales);

    QUrl layoutPath() const;
    void setLayoutPath(const QUrl &layoutPath);

    int wclAutoHideDelay() const;
    void setWclAutoHideDelay(int wclAutoHideDelay);

    bool wclAlwaysVisible() const;
    void setWclAlwaysVisible(bool wclAlwaysVisible);

    bool wclAutoCommitWord() const;
    void setWclAutoCommitWord(bool wclAutoCommitWord);

    bool fullScreenMode() const;
    void setFullScreenMode(bool fullScreenMode);

signals:
    void styleChanged();
    void styleNameChanged();
    void localeChanged();
    void availableLocalesChanged();
    void activeLocalesChanged();
    void layoutPathChanged();
    void wclAutoHideDelayChanged();
    void wclAlwaysVisibleChanged();
    void wclAutoCommitWordChanged();
    void fullScreenModeChanged();
};

} // namespace QtVirtualKeyboard

#endif // SETTINGS_H
