/*******************************************************************************************************
 DkThemeManager.h
 Created on:	18.11.2024

nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

Copyright (C) 2011-2013 Markus Diem <markus@nomacs.org>
Copyright (C) 2011-2013 Stefan Fiel <stefan@nomacs.org>
Copyright (C) 2011-2013 Florian Kleber <florian@nomacs.org>
This file is part of nomacs.

nomacs is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

nomacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#pragma once

#pragma warning(push, 0) // no warnings from includes - begin
#include <QColor>
#include <QObject>
#include <QPalette>
#pragma warning(pop) // no warnings from includes - end

class QStyle;

#ifndef DllCoreExport
#ifdef DK_CORE_DLL_EXPORT
#define DllCoreExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllCoreExport Q_DECL_IMPORT
#else
#define DllCoreExport Q_DECL_IMPORT
#endif
#endif

namespace nmc
{

/**
 * @brief Sets global style properties
 * @related DkPreferenceManager contains UI for changing settings affecting the theme
 *
 * @details Nomacs uses at least two Qt-flavored CSS stylesheets, a Qt style plugin,
 *          and user-specified colors to configure the theme.
 *
 *          The base sylesheet ("stylesheet.css") is a linked resource and cannot be user modified. It contains
 *          CSS for custom widgets such as preferences sidebar and manipulators. In general, it does not
 *          contain CSS to style the platform widget theme.
 *
 *          The user stylesheets ("Dark-Theme.css",etc) are packaged with the distribution and modifyable
 *          text files. User stylesheets override the base stylesheet and optionally override the platform widget theme.
 *
 *          There is a special empty stylesheet "System.css" which has no overrides or definitions,
 *          this indicates to the theme manager it must do specific actions such as modifying the application palette.
 *
 *          Stylesheets may define a specific set of color roles that bind to DkSettings, which are referenced
 *          throughout nomacs UI as well as replacing those names in the stylesheet itself. For a list of roles
 *          and their binding to settings, see cssColors().
 *
 *          Users may override certain color settings by setting a "default*Color" setting to false, and then setting
 *          the associated color setting to something else. If the "System" theme is active, this will result
 *          in remapping of the platform palette using a contrast-preserving heuristic (see DkPalette)
 *
 *          The "theme*Color" setting is set automatically and is not saved in settings, it always contains the
 *          theme color definition or the platform palette default.
 *
 *          When the "System" stylesheet is active, the platform palette will be used to establish certain colors,
 *          such as the viewport background.
 *
 */
class DllCoreExport DkThemeManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DkThemeManager)

public:
    /**
     * @brief singleton pattern
     * @return global instance
     */
    static DkThemeManager &instance();

    /**
     * @return name of theme for setCurrentTheme() etc
     * @note unique name of theme, usually the file name
     * @note not human readable, use getCleanThemeName() etc
     */
    QStringList getAvailableThemes() const;

    /**
     * @return name of stored/saved theme
     */
    QString getCurrentThemeName() const;

    /**
     * @brief set the current theme but do not load or apply it
     * @param themeName see:getAvailableThemes()
     */
    void setCurrentTheme(const QString &themeName) const;

    /**
     * @brief reload current theme and make it active
     */
    void applyTheme();

    /**
     * @param themeName internal theme name (no spaces, file extension)
     * @return human-readable theme name (spaces, no file extension)
     */
    QString cleanThemeName(const QString &themeName) const;

    /**
     * @return names of Qt style plugins, for setWidgetStyle()
     */
    QStringList getStylePlugins() const;

    /**
     * @brief set the Qt style plugin
     * @param name style plugin name from getStylePlugins()
     */
    void setStylePlugin(const QString &name);

signals:
    /**
     * @brief emit this signal after applyTheme()
     * @note for widgets that want to display current theme properties (like colors)
     * @note this will also fire when desktop/platform theme changes
     */
    void themeApplied();

protected:
    DkThemeManager();

    struct Color {
        const char *name;
        const QColor color;
    };
    typedef QVector<Color> ColorBinding;

    /**
     * @return dir to scan for external theme files
     */
    QString themeDir() const;

    /**
     * @brief read a stylesheet and preprocess it
     * @param path path to .css file
     * @return stylesheet with line/block macros removed
     */
    QString readFile(const QString &path) const;

    /**
     * @brief do line-preprocessing of nomacs CSS file
     * @param cssString nomacs-flavoured CSS from disk or resource
     * @return stylesheet ready for macro replacement
     * @note adds color defs to settings
     * @note result contains unprocessed macros and constants
     */
    QString preprocess(const QString &cssString) const;

    /**
     * @return names and current values of theme colors
     */
    ColorBinding cssColors() const;

    /**
     * @brief replace function macros (compute colors etc)
     * @param cssString
     * @param binding result of bindColors()
     * @return stylesheet ready for color replacement
     */
    QString replaceMacros(const QString &cssString, const ColorBinding &colors) const;

    /**
     * @brief replace color constants with nomacs settings
     * @param cssString stylesheet
     * @param binding result of bindColors()
     * @return stylesheet ready for Qt
     */
    QString replaceColors(const QString &cssString, const ColorBinding &colors) const;

    /**
     * @return true if stylesheet is a placeholder for an empty/system/unstyled widget theme
     */
    bool isSystemTheme() const;

    /**
     * @return true if current theme seems to be dark or light
     */
    bool isDarkColorScheme() const;

    /**
     * @brief intercept palette change events
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief compress app palette change events
     */
    void timerEvent(QTimerEvent *event) override;

    QString mDefaultStyle; // style plugin at startup
    bool mOurPaletteChange = false; // if true, ignore palette change events
    int mTimerId = -1;
};
}
