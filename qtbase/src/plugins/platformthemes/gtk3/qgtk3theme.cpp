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

#include "qgtk3theme.h"
#include "qgtk3dialoghelpers.h"
#include "qgtk3menu.h"
#include <QVariant>
#include <QGuiApplication>
#include <qpa/qwindowsysteminterface.h>

#undef signals
#include <gtk/gtk.h>

#include <X11/Xlib.h>

QT_BEGIN_NAMESPACE

const char *QGtk3Theme::name = "gtk3";

template <typename T>
static T gtkSetting(const gchar *propertyName)
{
    GtkSettings *settings = gtk_settings_get_default();
    T value;
    g_object_get(settings, propertyName, &value, NULL);
    return value;
}

static QString gtkSetting(const gchar *propertyName)
{
    gchararray value = gtkSetting<gchararray>(propertyName);
    QString str = QString::fromUtf8(value);
    g_free(value);
    return str;
}

void gtkMessageHandler(const gchar *log_domain,
                       GLogLevelFlags log_level,
                       const gchar *message,
                       gpointer unused_data) {
    /* Silence false-positive Gtk warnings (we are using Xlib to set
     * the WM_TRANSIENT_FOR hint).
     */
    if (g_strcmp0(message, "GtkDialog mapped without a transient parent. "
                           "This is discouraged.") != 0) {
        /* For other messages, call the default handler. */
        g_log_default_handler(log_domain, log_level, message, unused_data);
    }
}

bool QGtk3Theme::initialize()
{
    // A failed GTK connection must allow Qt to try the next platform theme.
    if (qEnvironmentVariableIsEmpty("GDK_BACKEND"))
        gdk_set_allowed_backends(QGuiApplication::platformName().startsWith(QLatin1String("wayland"))
                                 ? "wayland" : "x11");
    int (*oldErrorHandler)(Display *, XErrorEvent *) = XSetErrorHandler(nullptr);
    const bool initialized = gtk_init_check(nullptr, nullptr);
    XSetErrorHandler(oldErrorHandler);
    return initialized;
}

QGtk3Theme::QGtk3Theme()
    : m_palette(*QGnomeTheme::palette(SystemPalette)), m_tooltipPalette(m_palette)
{
    g_type_ensure(PANGO_TYPE_FONT_FAMILY);
    g_type_ensure(PANGO_TYPE_FONT_FACE);
    g_log_set_handler("Gtk", G_LOG_LEVEL_MESSAGE, gtkMessageHandler, nullptr);

    const auto desktops = qgetenv("XDG_CURRENT_DESKTOP").toUpper().split(':');
    if (desktops.contains("GNOME") || desktops.contains("UNITY")) {
        GSettingsSchemaSource *source = g_settings_schema_source_get_default();
        GSettingsSchema *schema = source ? g_settings_schema_source_lookup(source,
            "org.gnome.desktop.interface", true) : nullptr;
        if (schema) {
            if (g_settings_schema_has_key(schema, "color-scheme")) {
                m_gnomeSettings = g_settings_new_full(schema, nullptr, nullptr);
                g_signal_connect(m_gnomeSettings, "changed::color-scheme",
                    G_CALLBACK(+[](GSettings *, gchar *, gpointer data) {
                        static_cast<QGtk3Theme *>(data)->updatePalette();
                    }), this);
            }
            g_settings_schema_unref(schema);
        }
    }
    GtkSettings *settings = gtk_settings_get_default();
    for (const char *signal : {"notify::gtk-theme-name", "notify::gtk-application-prefer-dark-theme"})
        g_signal_connect(settings, signal,
            G_CALLBACK(+[](GObject *, GParamSpec *, gpointer data) {
                static_cast<QGtk3Theme *>(data)->updatePalette();
            }), this);
    updatePalette(false);
}

QGtk3Theme::~QGtk3Theme()
{
    g_signal_handlers_disconnect_by_data(gtk_settings_get_default(), this);
    if (m_gnomeSettings) {
        g_signal_handlers_disconnect_by_data(m_gnomeSettings, this);
        g_object_unref(m_gnomeSettings);
    }
}

static QColor qtGtkColor(const GdkRGBA &color)
{
    return QColor::fromRgbF(qBound(0.0, color.red, 1.0), qBound(0.0, color.green, 1.0),
                           qBound(0.0, color.blue, 1.0), qBound(0.0, color.alpha, 1.0));
}

static QColor gtkNamedColor(GtkStyleContext *context, const char *name, const QColor &fallback)
{
    GdkRGBA color;
    return gtk_style_context_lookup_color(context, name, &color) && color.alpha > 0
        ? qtGtkColor(color) : fallback;
}

static QColor gtkForeground(GtkStyleContext *context, GtkStateFlags state)
{
    GdkRGBA color;
    gtk_style_context_get_color(context, state, &color);
    return qtGtkColor(color);
}

static QColor gtkBackground(GtkStyleContext *context, GtkStateFlags state, const QColor &fallback)
{
    GdkRGBA color;
    gtk_style_context_get_background_color(context, state, &color);
    if (color.alpha <= 0)
        return fallback;
    // GTK backgrounds can be translucent; Qt's base brushes must be opaque.
    return QColor::fromRgbF(color.red * color.alpha + fallback.redF() * (1 - color.alpha),
                           color.green * color.alpha + fallback.greenF() * (1 - color.alpha),
                           color.blue * color.alpha + fallback.blueF() * (1 - color.alpha));
}

void QGtk3Theme::updatePalette(bool notify)
{
    if (m_updatingPalette)
        return;
    m_updatingPalette = true;
    if (m_gnomeSettings) {
        gchar *scheme = g_settings_get_string(static_cast<GSettings *>(m_gnomeSettings), "color-scheme");
        g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme",
                     g_strcmp0(scheme, "prefer-dark") == 0, nullptr);
        g_free(scheme);
    }

    // Unmapped widgets provide CSS contexts without creating visible GTK windows.
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);
    GtkWidget *entry = gtk_entry_new();
    GtkWidget *button = gtk_button_new();
    GtkWidget *view = gtk_text_view_new();
    for (GtkWidget *widget : {entry, button, view})
        gtk_container_add(GTK_CONTAINER(box), widget);
    GtkStyleContext *wc = gtk_widget_get_style_context(window);
    GtkStyleContext *ec = gtk_widget_get_style_context(entry);
    GtkStyleContext *bc = gtk_widget_get_style_context(button);
    GtkStyleContext *vc = gtk_widget_get_style_context(view);
    GtkWidget *tooltip = gtk_window_new(GTK_WINDOW_POPUP);
    GtkStyleContext *tc = gtk_widget_get_style_context(tooltip);
    gtk_style_context_add_class(tc, GTK_STYLE_CLASS_TOOLTIP);

    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        const auto state = group == QPalette::Disabled ? GTK_STATE_FLAG_INSENSITIVE
            : group == QPalette::Inactive ? GTK_STATE_FLAG_BACKDROP : GTK_STATE_FLAG_NORMAL;
        for (GtkStyleContext *context : {wc, ec, bc, vc, tc})
            gtk_style_context_set_state(context, state);
        const QColor windowText = gtkForeground(wc, state);
        const QColor windowColor = gtkBackground(wc, state,
            gtkNamedColor(wc, "theme_bg_color", m_palette.color(group, QPalette::Window)));
        const QColor base = gtkBackground(ec, state, gtkNamedColor(ec, "theme_base_color", windowColor));
        const QColor text = gtkForeground(ec, state);
        const QColor buttonColor = gtkBackground(bc, state, windowColor);
        const QColor buttonText = gtkForeground(bc, state);
        const QColor highlight = gtkNamedColor(vc,
            group == QPalette::Inactive ? "theme_unfocused_selected_bg_color" : "theme_selected_bg_color",
            gtkNamedColor(vc, "theme_selected_bg_color", m_palette.color(group, QPalette::Highlight)));
        const QColor highlightedText = gtkNamedColor(vc,
            group == QPalette::Inactive ? "theme_unfocused_selected_fg_color" : "theme_selected_fg_color",
            gtkNamedColor(vc, "theme_selected_fg_color", m_palette.color(group, QPalette::HighlightedText)));
        m_palette.setColor(group, QPalette::Window, windowColor);
        m_palette.setColor(group, QPalette::WindowText, windowText);
        m_palette.setColor(group, QPalette::Base, base);
        m_palette.setColor(group, QPalette::Text, text);
        m_palette.setColor(group, QPalette::Button, buttonColor);
        m_palette.setColor(group, QPalette::ButtonText, buttonText);
        m_palette.setColor(group, QPalette::Highlight, highlight);
        m_palette.setColor(group, QPalette::HighlightedText, highlightedText);
        m_palette.setColor(group, QPalette::AlternateBase, QColor::fromRgbF(
            (base.redF() + windowColor.redF()) / 2, (base.greenF() + windowColor.greenF()) / 2,
            (base.blueF() + windowColor.blueF()) / 2));
        m_palette.setColor(group, QPalette::Light, windowColor.lighter(150));
        m_palette.setColor(group, QPalette::Midlight, windowColor.lighter(120));
        m_palette.setColor(group, QPalette::Mid, windowColor.darker(130));
        m_palette.setColor(group, QPalette::Dark, windowColor.darker(160));
        m_palette.setColor(group, QPalette::Shadow, windowColor.darker(200));
        m_palette.setColor(group, QPalette::Link, gtkNamedColor(vc, "link_color", text));
        m_palette.setColor(group, QPalette::LinkVisited, gtkNamedColor(vc, "visited_link_color", text));
        const QColor tooltipBase = gtkBackground(tc, state,
            gtkNamedColor(tc, "theme_tooltip_bg_color", windowColor));
        const QColor tooltipText = gtkForeground(tc, state);
        m_palette.setColor(group, QPalette::ToolTipBase, tooltipBase);
        m_palette.setColor(group, QPalette::ToolTipText, tooltipText);
    }
    m_tooltipPalette = m_palette;
    for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        const auto background = m_palette.color(group, QPalette::ToolTipBase);
        const auto foreground = m_palette.color(group, QPalette::ToolTipText);
        m_tooltipPalette.setColor(group, QPalette::Window, background);
        m_tooltipPalette.setColor(group, QPalette::WindowText, foreground);
        m_tooltipPalette.setColor(group, QPalette::Base, background);
        m_tooltipPalette.setColor(group, QPalette::Text, foreground);
    }
    gtk_widget_destroy(tooltip);
    gtk_widget_destroy(window);
    m_updatingPalette = false;
    if (notify)
        QWindowSystemInterface::handleThemeChange(nullptr);
}

const QPalette *QGtk3Theme::palette(Palette type) const
{
    if (type == SystemPalette)
        return &m_palette;
    if (type == ToolTipPalette)
        return &m_tooltipPalette;
    return QGnomeTheme::palette(type);
}

static inline QVariant gtkGetLongPressTime()
{
    const char *gtk_long_press_time = "gtk-long-press-time";
    static bool found = g_object_class_find_property(G_OBJECT_GET_CLASS(gtk_settings_get_default()), gtk_long_press_time);
    if (!found)
        return QVariant();
    return QVariant(gtkSetting<guint>(gtk_long_press_time));  // Since 3.14, apparently we support >= 3.6
}

QVariant QGtk3Theme::themeHint(QPlatformTheme::ThemeHint hint) const
{
    switch (hint) {
    case QPlatformTheme::CursorFlashTime:
        return QVariant(gtkSetting<gint>("gtk-cursor-blink-time"));
    case QPlatformTheme::MouseDoubleClickDistance:
        return QVariant(gtkSetting<gint>("gtk-double-click-distance"));
    case QPlatformTheme::MouseDoubleClickInterval:
        return QVariant(gtkSetting<gint>("gtk-double-click-time"));
    case QPlatformTheme::MousePressAndHoldInterval: {
        QVariant v = gtkGetLongPressTime();
        if (!v.isValid())
            v = QGnomeTheme::themeHint(hint);
        return v;
    }
    case QPlatformTheme::PasswordMaskDelay:
        return QVariant(gtkSetting<guint>("gtk-entry-password-hint-timeout"));
    case QPlatformTheme::StartDragDistance:
        return QVariant(gtkSetting<gint>("gtk-dnd-drag-threshold"));
    case QPlatformTheme::SystemIconThemeName:
        return QVariant(gtkSetting("gtk-icon-theme-name"));
    case QPlatformTheme::SystemIconFallbackThemeName:
        return QVariant(gtkSetting("gtk-fallback-icon-theme"));
    default:
        return QGnomeTheme::themeHint(hint);
    }
}

QString QGtk3Theme::gtkFontName() const
{
    QString cfgFontName = gtkSetting("gtk-font-name");
    if (!cfgFontName.isEmpty())
        return cfgFontName;
    return QGnomeTheme::gtkFontName();
}

bool QGtk3Theme::usePlatformNativeDialog(DialogType type) const
{
    switch (type) {
    case ColorDialog:
        return true;
    case FileDialog:
        return useNativeFileDialog();
    case FontDialog:
        return true;
    default:
        return false;
    }
}

QPlatformDialogHelper *QGtk3Theme::createPlatformDialogHelper(DialogType type) const
{
    switch (type) {
    case ColorDialog:
        return new QGtk3ColorDialogHelper;
    case FileDialog:
        if (!useNativeFileDialog())
            return nullptr;
        return new QGtk3FileDialogHelper;
    case FontDialog:
        return new QGtk3FontDialogHelper;
    default:
        return 0;
    }
}

QPlatformMenu* QGtk3Theme::createPlatformMenu() const
{
    return new QGtk3Menu;
}

QPlatformMenuItem* QGtk3Theme::createPlatformMenuItem() const
{
    return new QGtk3MenuItem;
}

bool QGtk3Theme::useNativeFileDialog()
{
    /* Require GTK3 >= 3.15.5 to avoid running into this bug:
     * https://bugzilla.gnome.org/show_bug.cgi?id=725164
     *
     * While this bug only occurs when using widget-based file dialogs
     * (native GTK3 dialogs are fine) we have to disable platform file
     * dialogs entirely since we can't avoid creation of a platform
     * dialog helper.
     */
    return gtk_check_version(3, 15, 5) == 0;
}

QT_END_NAMESPACE
