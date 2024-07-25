
/*******************************************************************************************************
 DkThemeManager.cpp
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

#include "DkThemeManager.h"
#include "DkSettings.h"
#include "DkTimer.h"
#include "DkUtils.h"

#pragma warning(push, 0) // no warnings from includes - begin
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleHints>
#include <cmath>
#pragma warning(pop) // no warnings from includes - end

namespace nmc
{

struct Point3f {
    float x, y, z;
};

// we need a uniform colorspace for contrast calculations
static Point3f rgbToUniform(const QColor &c)
{
    // Oklab transform; more recent than Lab but also simple to compute

    // convert 8-bit srgb to linear RGB
    auto srgbToLinear = [](int v) {
        double c = v / 255.0;
        return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
    };

    double r = srgbToLinear(c.red());
    double g = srgbToLinear(c.green());
    double b = srgbToLinear(c.blue());

    double l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b;
    double m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b;
    double s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b;

    l = cbrt(l); // cube root
    m = cbrt(m);
    s = cbrt(s);

    float L = l * +0.2104542553 + m * +0.7936177850 + s * -0.0040720468;
    float A = l * +1.9779984951 + m * -2.4285922050 + s * +0.4505937099;
    float B = l * +0.0259040371 + m * +0.7827717662 + s * -0.8086757660;

    return Point3f{L, A, B};
}

static QColor uniformToRgb(const Point3f &oklab)
{
    // Inverse Oklab transform
    double L = oklab.x;
    double A = oklab.y;
    double B = oklab.z;

    double l = L + A * +0.3963377774 + B * +0.2158037573;
    double m = L + A * -0.1055613458 + B * -0.0638541728;
    double s = L + A * -0.0894841775 + B * -1.2914855480;

    l = l * l * l;
    m = m * m * m;
    s = s * s * s;

    double r = l * +4.0767416621 + m * -3.3077115913 + s * +0.2309699292;
    double g = l * -1.2684380046 + m * +2.6097574011 + s * -0.3413193965;
    double b = l * -0.0041960863 + m * -0.7034186147 + s * +1.7076147010;

    // linear RGB to srgb
    auto linearToSrgb = [](double c) {
        return c >= 0.0031308 ? 1.055 * pow(c, 1 / 2.4) - 0.055 : 12.92 * c;
    };

    r = linearToSrgb(r);
    g = linearToSrgb(g);
    b = linearToSrgb(b);

    int red = r * 255;
    int green = g * 255;
    int blue = b * 255;

    red = qBound(0, red, 255);
    green = qBound(0, green, 255);
    blue = qBound(0, blue, 255);

    return QColor(red, green, blue);
}

static float uniformLuma(const QColor &c)
{
    return rgbToUniform(c).x;
}

static float euclidianDistance(const Point3f &a, const Point3f &b)
{
    float d0 = b.x - a.x;
    float d1 = b.y - a.y;
    float d2 = b.z - a.z;
    return std::sqrt(d0 * d0 + d1 * d1 + d2 * d2);
};

static float colorDistance(const QColor &c1, const QColor &c2)
{
    Point3f a = rgbToUniform(c1);
    Point3f b = rgbToUniform(c2);
    return euclidianDistance(a, b);
};

// point on vector AB at least C distance from A
static Point3f pointOnVector(const Point3f &a, const Point3f &b, float c)
{
    const Point3f ab = {b.x - a.x, b.y - a.y, b.z - a.z};
    float magnitude = std::sqrt(ab.x * ab.x + ab.y * ab.y + ab.z * ab.z);

    if (c >= magnitude)
        return b;

    float scale = c / magnitude;

    return Point3f{a.x + ab.x * scale, a.y + ab.y * scale, a.z + ab.z * scale};
}

// color palette calculations
class DkPalette
{
    DkPalette() = delete;

private:
    const QPalette mIn; // basis palette (usually system theme palette)
    QPalette mOut; // output palette
public:
    DkPalette(const QPalette &p)
        : mIn(p)
    {
        mOut = mIn;
    }

    // bool modified = false;
    QPalette &output()
    {
        return mOut;
    }

    bool modified() const
    {
        return mIn != mOut;
    }

    // set QPalette::Window to user color (bgBase) and compute the other background colors
    void remapBackground(const QColor &bgBase)
    {
        struct {
            QPalette::ColorRole role; // palette index
            float lumaDiff = -1; // role luma - ::Window luma
        } palette[] = {{QPalette::Window},
                       {QPalette::Base},
                       {QPalette::AlternateBase},
                       {QPalette::Button},
                       {QPalette::Light},
                       {QPalette::Midlight},
                       {QPalette::Mid},
                       {QPalette::Dark},
                       {QPalette::Shadow}};

        const auto oldBg = rgbToUniform(mIn.color(QPalette::Window));
        const auto newBg = rgbToUniform(bgBase);

        // to automatically keep decent contrast, invert the palette
        // we will also invert if remapping would otherwise fail
        bool invertPalette = std::abs(oldBg.x - newBg.x) > 0.5;
        if (invertPalette) {
            qWarning("[theme] color palette was inverted to increase contrast, controls may look strange");
            qWarning("[theme] choose a darker/lighter background color or system theme to prevent this");
        }

        // int numLighter = 0; // count colors lighter than ::Window
        // int numDarker = 0; // count colors darker than ::Window
        float minDiff = std::numeric_limits<float>::max(); // minimum diff to ::Window
        float maxDiff = std::numeric_limits<float>::min(); // maximum diff to ::Window

        for (auto &entry : palette) {
            float baseLuma = oldBg.x;
            float luma = uniformLuma(mIn.color(entry.role));
            if (invertPalette) {
                luma = 1.0 - luma;
                baseLuma = 1.0 - baseLuma;
            }
            entry.lumaDiff = luma - baseLuma;
            minDiff = qMin(minDiff, entry.lumaDiff);
            maxDiff = qMax(maxDiff, entry.lumaDiff);
            // if (entry.lumaDiff > 0)
            //     numLighter++;
            // if (entry.lumaDiff < 0)
            //     numDarker++;
        }

        // qDebug("REMAP: minDiff:%f maxDiff:%f diffRange:%f lighter:%d darker:%d", minDiff, maxDiff, maxDiff - minDiff, numLighter, numDarker);

        // keep this, might prove useful
        //
        // bool darkTheme = false;
        // if (numLighter > numDarker) { // this heuristic was correct for all qt5ct color palettes
        //     darkTheme = true;
        //     qDebug("REMAP: guessing DARK theme");
        // }

        //
        // the new luma range, simply by adding differences, won't fit into 0..1 in most cases
        //
        // in these cases, use a modified linear transform to rescale luma
        //     yn = a(xn-x0) + y0 + c where
        //          yn is the new luma
        //          xn is the luma value in system palette
        //          x0 is the luma value of the ::Window role
        //           a is is scale factor (reduces or adds contrast)
        //          y0 is the new bg luma (user color)
        //           c is a shift to compensate for oob condition, not applied if xn-x0==0

        float y0 = newBg.x;
        float a = 1.0;
        float c = 0;

        const float minLuma = a * minDiff + y0;
        const float maxLuma = a * maxDiff + y0;

        // qDebug("REMAP: before remap min: %f max:%f range:%f", minLuma, maxLuma, lumaRange);

        if (minLuma < 0.0) {
            if (maxLuma > 1.0) {
                qWarning("[theme] logic error (min)"); // not possible since inputs are all 0..1
            } else if (maxLuma <= 0.01f) {
                a = -1.0 / (maxDiff - minDiff); // invert
                qWarning("[theme] palette must be inverted (maxLuma <= 0.0)");
            } else {
                c = -minLuma; // make min luma==0
                if (maxLuma + c > 1.0 && maxLuma != y0)
                    a = (1.0 - c) / (maxDiff - minDiff); // rescale+shift 0..1
            }
        } else if (maxLuma > 1.0) {
            if (minLuma < 0.0) {
                qWarning("[theme] logic error (max)"); // not possible since inputs are all 0..1
            } else if (minLuma >= 1.0f) {
                a = -1.0 / (maxDiff - minDiff); // invert
                qWarning("[theme] palette must be inverted (minLuma >= 1.0)");
            } else {
                c = 1.0 - maxLuma; // make max luma == 1.0
                if (minLuma + c > 1.0 && minLuma != y0)
                    a = (1.0 - c) / (maxDiff - minDiff); // rescale+shift 0..1
            }
        }

        // qDebug("REMAP center=%f scale=%f shift=%f min =%f max=%f", b, a, c, a * minDiff + b, a * maxDiff + b);

        for (const auto &entry : palette) {
            float shift = c;
            if (entry.lumaDiff == 0.0f) // do not shift the user color
                shift = 0.0;

            float luma = a * entry.lumaDiff + y0 + shift;
            if (luma < 0 || luma > 1) {
                qWarning() << "[theme] new palette entry is clipping" << bgBase;
                luma = qBound(0.0f, luma, 1.0f);
            }

            QColor newColor = uniformToRgb({luma, newBg.y, newBg.z});
            newColor.setAlpha(mIn.color(entry.role).alpha());

            mOut.setColor(QPalette::Active, entry.role, newColor);
            mOut.setColor(QPalette::Inactive, entry.role, newColor);

            // some backgrounds have disabled color that is different, uncommon but we'll support it
            QColor disabled = newColor;
            const QColor sysActive = mIn.color(QPalette::Active, entry.role);
            const QColor sysDisabled = mIn.color(QPalette::Disabled, entry.role);

            if (sysActive != sysDisabled) {
                // don't rescale the disabled palette, match brightness difference and clip
                float disabledDiff = uniformLuma(sysDisabled) - uniformLuma(sysActive);
                if (invertPalette || a < 0.0) // inverted hueristic or forced
                    disabledDiff = -disabledDiff;

                float disabledLuma = luma + disabledDiff;
                disabledLuma = qBound(0.0f, disabledLuma, 1.0f);

                auto dlab = rgbToUniform(disabled);
                disabled = uniformToRgb({disabledLuma, dlab.y, dlab.z});
            }

            disabled.setAlpha(sysDisabled.alpha());
            mOut.setColor(QPalette::Disabled, entry.role, disabled);
        }

        mOut.setColor(QPalette::Window, bgBase); // ensure it matches user color exactly
    }

    // set a foreground(text) color, optionally matching contrast with system background
    // remapBackground must be called first to establish the new background color
    void setForegroundColor(QPalette::ColorRole role, QPalette::ColorRole bgRole, const QColor &color)
    {
        QColor active = color;
        if (bgRole != QPalette::NoRole) {
            float contrast = colorDistance(mIn.color(role), mIn.color(bgRole));
            active = adjustContrast(contrast, color, mOut.color(bgRole));
        }

        mOut.setColor(QPalette::Active, role, active);
        mOut.setColor(QPalette::Inactive, role, active); // inactive is usually a copy of active
    };

    // set a disabled color besides background colors
    void setDisabledColor(QPalette::ColorRole role, QPalette::ColorRole bgRole)
    {
        QColor disabled = mOut.color(role); // setForeGround()/remapBackground() must come first to get the right color

        const QColor sysActive = mIn.color(QPalette::Active, role);
        const QColor sysDisabled = mIn.color(QPalette::Disabled, role);

        if (sysActive != sysDisabled) {
            const QColor background = mOut.color(QPalette::Disabled, bgRole);

            // guess that blending white or black gives good contrast
            float bgLuma = uniformLuma(background);
            if (bgLuma < 0.5)
                disabled = QColor(255, 255, 255, 100);
            else
                disabled = QColor(0, 0, 0, 100);

            // TODO: should be able to match the old contrast ratio now
            // const QColor sysBackground = sp.color(QPalette::Disabled, bgRole);
            // float sysContrast = colorDistance(sysDisabled, sysBackground);
            // disabled = adjustContrast(sysContrast, sysDisabled, background);
        }

        mOut.setColor(QPalette::Disabled, role, disabled);
    };

    // set contrast of foreground color against background while preserving hue
    QColor adjustContrast(float minContrast, const QColor &foreground, const QColor &background) const
    {
        QColor result = foreground;

        const auto fg = rgbToUniform(foreground);
        const auto bg = rgbToUniform(background);

        auto max = fg, min = fg;
        max.x = 1.0;
        min.x = 0.0;

        auto p1 = pointOnVector(bg, max, minContrast); // towards white
        auto p2 = pointOnVector(bg, min, minContrast); // towards black

        float c1 = euclidianDistance(p1, bg);
        float c2 = euclidianDistance(p2, bg);

        // use the maximum contrast
        if (c1 < c2)
            p1 = p2;

        result = uniformToRgb(p1);
        result.setAlpha(foreground.alpha());

        return result;
    }

    /*
        // adjust brightness of a color, invert if it goes out of bounds
        QColor adjustBrightness(float lumaDelta, const QColor &color, bool noInversion)
        {
    #ifdef USE_HSV
            qreal h, s, v;
            color.getHsvF(&h, &s, &v);

            qreal luma = v + lumaDelta;
            if (!noInversion && (v < 0.0f || luma > 1.0f))
                luma = v - lumaDelta;

            luma = qMax(0.0, qMin(1.0, luma));

            QColor c = QColor::fromHsvF(h, s, luma);
    #else
            const auto bg = rgbToUniform(color);

            float luma = bg.x + lumaDelta;

            // go the other way to ensure contrast is preserved
            // in some cases this is not what we want as it could make sunken widgets look raised
            if (!noInversion && (luma < 0.0f || luma > 1.0f))
                luma = bg.x - lumaDelta;

            // clip
            luma = qMax(0.0f, qMin(1.0f, luma));

            QColor c = uniformToRgb({luma, bg.y, bg.z});
            if (lumaDelta == 0.0f)
                qWarning() << color << c;
    #endif
            c.setAlpha(color.alpha());
            return c;
        }
    */

#ifdef QT_DEBUG
    static QString ttySwatch(const QColor &color)
    {
        int r, g, b, a;
        color.getRgb(&r, &g, &b, &a);
        return QString::asprintf("\033[48;2;%d;%d;%dm    \033[0m:%3d", r, g, b, a);
    }

    static void printColor(const QString &name, const QColor &color, bool defaultColor = true)
    {
        QString line = "    " + ttySwatch(color);
        line += " " + color.name(QColor::HexArgb);
        line += " " + name;
        if (!defaultColor)
            line += "***";
        qDebug("%s", qPrintable(line));
    }

    void printRole(QPalette::ColorRole role, const char *name) const
    {
        QColor color = mOut.color(role);
        QString line;
        line += "   ";
        line += " " + ttySwatch(mIn.color(QPalette::Active, role));
        line += " " + ttySwatch(mOut.color(QPalette::Active, role));
        line += " |";
        line += " " + ttySwatch(mIn.color(QPalette::Disabled, role));
        line += " " + ttySwatch(mOut.color(QPalette::Disabled, role));
        line += " | ";
        line += name;
        qDebug("%s", qPrintable(line));
    }

    void print() const
    {
        qDebug("    active              disabled");
        qDebug("    in       out        in       out");
#define PRINT(x) printRole(x, #x);
        PRINT(QPalette::Window);
        PRINT(QPalette::Base);
        PRINT(QPalette::AlternateBase);
        PRINT(QPalette::Button);
        PRINT(QPalette::Light);
        PRINT(QPalette::Midlight);
        PRINT(QPalette::Mid);
        PRINT(QPalette::Dark);
        PRINT(QPalette::Shadow);
        PRINT(QPalette::Text);
        PRINT(QPalette::WindowText);
        PRINT(QPalette::ButtonText);
        PRINT(QPalette::BrightText);
        PRINT(QPalette::Highlight);
        PRINT(QPalette::HighlightedText);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        PRINT(QPalette::Accent);
#endif
        PRINT(QPalette::Link);
        PRINT(QPalette::LinkVisited);
        PRINT(QPalette::ToolTipBase);
        PRINT(QPalette::ToolTipText);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        PRINT(QPalette::PlaceholderText);
#endif
#undef PRINT
    }
#endif // QT_DEBUG
};

DkThemeManager::DkThemeManager()
    : mSystemPalette(qApp->palette())
{
    // we'd like to know the default style for settings, also to avoid
    // constructing a style at startup for faster launching
    // there is no way to find the name of the default style besides having this table,
    // you would think QStyle would advertise this, but no
    static constexpr const struct {
        const char *className; // class name of the QStyle subclass
        const char *pluginName; // style name in the plugin .json file
    } knownStylePlugins[] = {
        {"QFusionStyle", "Fusion"},
        {"Breeze::Style", "Breeze"},
        {"QWindowsVistaStyle", "windowsvista"},
        {"QWindows11Style", "windows11"},
        {"QMacStyle", "macOS"},
        {"QWindowsStyle", "Windows"},
        {"QAndroidStyle", "android"},
        {"Qt5CTProxyStyle", "qt5ct-style"},
        {"Qt6CTProxyStyle", "qt6ct-style"},
        {"Oxygen::Style", "Oxygen"},
        {"QtCurve::Style", "QtCurve"},
        {"Kvantum::Style", "kavantum"}, // kvantum-dark is in the same class, I guess we ignore it
        {"QCDEStyle", "cde"},
        {"QCleanlooksStyle", "cleanlooks"},
        {"QMotifStyle", "motif"},
        {"QPlastiqueStyle", "plastique"},
        {"QGtkStyle", "gtk2"},
        {"QBB10DarkStyle", "bb10dark"},
        {"QBB10BrightStyle", "bb10bright"},
        {"QHaikuStyle", "haiku"},
    };

    const QString className = qApp->style()->metaObject()->className();
    for (auto &plugin : knownStylePlugins)
        if (plugin.className == className) {
            mDefaultStyle = plugin.pluginName;
            break;
        }

    if (mDefaultStyle.isEmpty()) {
        mDefaultStyle = qEnvironmentVariable("QT_STYLE_OVERRIDE");
        if (mDefaultStyle.isEmpty()) {
            mDefaultStyle = getStylePlugins().first();
            qWarning() << "[theme] unknown style plugin:" << className;
            qWarning() << "[theme] assuming default style is:" << mDefaultStyle;
        }
    }

    qInfo() << "[theme] default widget style/name:" << className << mDefaultStyle;

    QString name = DkSettingsManager::param().display().stylePlugin;
    if (name == "Default") {
#ifdef Q_OS_WIN
        // windowsvista style is the default on <=10, doesn't support dark windows theme
        // windows11 style is the default on >=11 and it looks weird
        name = "Fusion";
#else
        name = mDefaultStyle;
#endif
        DkSettingsManager::param().display().stylePlugin = name;
    }

    if (name != mDefaultStyle) {
        setStylePlugin(name);
        mSystemPalette = qApp->palette();
    }

    // we connect this after setStylePlugin, it can change the palette before we're ready
    QObject::connect(qApp, &QApplication::paletteChanged, [this]() {
        qDebug() << "[theme] platform palette changed";
        mSystemPalette = qApp->palette();
        applyTheme();
    });
}

DkThemeManager &DkThemeManager::instance()
{
    static auto *inst = new DkThemeManager; // no destructor will be called, but we don't need to
    return *inst;
}

QStringList DkThemeManager::getAvailableThemes() const
{
    QDir dir(themeDir());
    dir.setNameFilters(QStringList("*.css"));
    return dir.entryList(QDir::Files, QDir::Name);
}

QString DkThemeManager::getCurrentThemeName() const
{
    return DkSettingsManager::param().display().themeName;
}

void DkThemeManager::setCurrentTheme(const QString &themeName) const
{
    DkSettingsManager::param().display().themeName = themeName;
}

void DkThemeManager::applyTheme()
{
    DkTimer dt;
    QString cssString;

    cssString += preprocess(readFile(":/nomacs/stylesheet.css"));

    const QFileInfo themeFileInfo(themeDir(), getCurrentThemeName());
    cssString += preprocess(readFile(themeFileInfo.absoluteFilePath()));

    const ColorBinding colors = cssColors(); // must follow css preprocessing (or you get the defaults)
    cssString = replaceMacros(cssString, colors);
    cssString = replaceColors(cssString, colors); // must follow macro replacement

    const DkSettings::Display &d = DkSettingsManager::param().display();

    // we always modify the palette, either for system theme, fallback for broken CSS,
    // and some missing CSS capabilities
    DkPalette palette(mSystemPalette);

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QStyleHints *hints = qApp->styleHints();
    Qt::ColorScheme scheme = Qt::ColorScheme::Unknown;
#endif

    if (isSystemTheme()) {
        //
        // Change palette to recolor all widgets. Attempt to follow the
        // existing palette contrast ratios.
        //
        // Results depend on the style plugin since the usage of palette can differ,
        // styles can also draw native widgets and ignore the palette completely.
        //
        // TODO: unit test?
        // test all possible color combinations
        // for (int r = 0; r < 256; r++) {
        //     for (int g = 0; g < 256; g++)
        //         for (int b = 0; b < 256; b++)
        //             remapBackground(QColor(r, g, b));
        // }
        // ::exit(0);
        // return;

        if (!d.defaultForegroundColor) {
            palette.setForegroundColor(QPalette::Text, QPalette::NoRole, d.fgColor);
            palette.setForegroundColor(QPalette::WindowText, QPalette::NoRole, d.fgColor);
            palette.setForegroundColor(QPalette::ButtonText, QPalette::NoRole, d.fgColor);
        }

        if (!d.defaultBackgroundColor) {
            auto &sp = DkThemeManager::instance().systemPalette();

            palette.remapBackground(d.bgColor);

            // other colors related to background but different hue (probably)
            palette.setForegroundColor(QPalette::Link, QPalette::Base, sp.color(QPalette::Link));
            palette.setForegroundColor(QPalette::LinkVisited, QPalette::Base, sp.color(QPalette::LinkVisited));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            palette.setForegroundColor(QPalette::PlaceholderText, QPalette::Base, sp.color(QPalette::PlaceholderText));
#endif
            // set disabled text colors; active color is controlled by user
            // must follow remapBackground() to pickup the modified background color
            palette.setDisabledColor(QPalette::Text, QPalette::Base);
            palette.setDisabledColor(QPalette::WindowText, QPalette::Window);
            palette.setDisabledColor(QPalette::ButtonText, QPalette::Button);
            palette.setDisabledColor(QPalette::Link, QPalette::Base);
            palette.setDisabledColor(QPalette::LinkVisited, QPalette::Base);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            palette.setDisabledColor(QPalette::PlaceholderText, QPalette::Base);
#endif
        }

        if (!d.defaultIconColor) {
            // TODO: maybe this should be applied to ::BrightText ??
        }

        if (palette.modified()) {
            qWarning() << "Modifying system theme is experimental. Use non-system theme for best results";

            // disabled this for now since it changes the window title bar
            // #if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
            //             // auto-select light/dark system palette for custom colors
            //             if (d.fgColor.lightness() > d.bgColor.lightness())
            //                 scheme = Qt::ColorScheme::Dark;
            //             else
            //                 scheme = Qt::ColorScheme::Light;
            // #endif
        }

    } else {
        // CSS should replace all of these colors, however if it fails
        // to parse, a fallback may prevent UI from being unreadable
        QPalette &p = palette.output();
        p.setColor(QPalette::Window, d.themeBgdColor);
        p.setColor(QPalette::Base, d.themeBgdColor);
        p.setColor(QPalette::WindowText, d.themeFgdColor);
        p.setColor(QPalette::ButtonText, d.themeFgdColor);

        auto &sp = DkThemeManager::instance().systemPalette();

        // the link color can't be changed in css
        palette.setForegroundColor(QPalette::Link, QPalette::Base, sp.color(QPalette::Link));
        palette.setForegroundColor(QPalette::LinkVisited, QPalette::Base, sp.color(QPalette::LinkVisited));

        // TODO: qt5 cannot change placeholder text color in css
        // palette.setForegroundColor(QPalette::PlaceholderText, QPalette::Base, colors["PLACEHOLDER_COLOR"], true);

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        QString theme = getCurrentThemeName().toLower();
        bool dark = theme.contains("dark"); // TODO: pull from theme CSS and another user option?
        bool light = theme.contains("light");
        if (dark)
            scheme = Qt::ColorScheme::Dark;
        else if (light)
            scheme = Qt::ColorScheme::Light;
#endif
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // set a dark/light hint to window manager
    // NOTE: changing the scheme will potentially alter the system palette and cause a recursion
    if (scheme != hints->colorScheme()) {
        const QPalette copy = mSystemPalette;
        hints->setColorScheme(scheme);
        if (copy != mSystemPalette)
            return;
    }
    qInfo() << "[theme] color scheme hint:" << hints->colorScheme();
#endif

#ifdef QT_DEBUG
    qDebug("-------css colors------");
    for (auto &c : colors)
        DkPalette::printColor(c.name, c.color);

    qDebug("-------settings--------");

    DkPalette::printColor("themeBgdColor", d.themeBgdColor);
    DkPalette::printColor("bgColor", d.bgColor, d.defaultBackgroundColor);
    DkPalette::printColor("themeFgdColor", d.themeFgdColor);
    DkPalette::printColor("fgColor", d.fgColor, d.defaultForegroundColor);
    DkPalette::printColor("themeIconColor", d.themeIconColor);
    DkPalette::printColor("iconColor", d.iconColor, d.defaultIconColor);
    DkPalette::printColor("bgColorFrameless", d.bgColorFrameless);
    DkPalette::printColor("highlightColor", d.highlightColor);
    DkPalette::printColor("hudBgColor", d.hudBgColor);
    DkPalette::printColor("hudFgdColor", d.hudFgdColor);
    DkPalette::printColor("slideshowColor", DkSettingsManager::param().slideShow().backgroundColor);

    qDebug("-------palette---------");
    palette.print();
    qDebug();
#endif // QT_DEBUG

    // set the palette for select subclasses since we need qApp->palette() to remain unchanged,
    // so we can track external palette changes like theme switching or accent color changes
    // NOTE: this does not work with windowsvista style plugin unless "-platform windows:darkmode=1" is passed,
    // and even then the result is quite poor. This is a known Qt limitation.
    QPalette newPalette = palette.output();
    qApp->setPalette(newPalette, "QObject");
    qApp->setPalette(newPalette, "QMenu");

    qApp->setStyleSheet(cssString);

    // force palette change in Qt even if it did not change (for dynamic switching)
    qApp->setAttribute(Qt::AA_SetPalette, true);

    qInfo() << "theme applied in:" << dt;

    emit themeApplied();
}

QString DkThemeManager::cleanThemeName(const QString &themeName) const
{
    QString name = themeName;
    name = name.replace(".css", "");
    name = name.replace("-", " ");

    return name;
}

QStringList DkThemeManager::getStylePlugins() const
{
    QStringList names = QStyleFactory::keys();
    names.sort(Qt::CaseInsensitive);

    // windows11 style does nothing on < 11
    if (QSysInfo::productType() == "windows" && QSysInfo::productVersion().toInt() < 11)
        names.removeOne("windows11");

    return names;
}

void DkThemeManager::setStylePlugin(const QString &name)
{
    QStyle *style = QStyleFactory::create(name); // always allocates; qApp takes ownership
    if (!style) {
        qWarning() << "[theme] widget style" << name << "does not exist or cannot be loaded";
        return;
    }
    qDebug() << "[theme] setting widget style:" << style->metaObject()->className() << name;
    qApp->setStyle(style);
}

QString DkThemeManager::themeDir() const
{
    QStringList paths;
    paths << QCoreApplication::applicationDirPath();
    paths << QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);

    qDebug() << "[theme] looking for css files in:" << paths;

    QDir themeDir;

    for (const QString &p : paths) {
        themeDir = QDir(p + QDir::separator() + "themes");

        if (themeDir.exists())
            break;
    }

    return themeDir.absolutePath();
}

QString DkThemeManager::readFile(const QString &path) const
{
    QString cssString;
    QFile file(path);

    if (file.open(QFile::ReadOnly)) {
        cssString = file.readAll();
        qInfo() << "CSS loaded from: " << path;
    } else
        qInfo() << "CSS failed to load from" << path;

    return cssString;
}

QString DkThemeManager::preprocess(const QString &cssString) const
{
    // the theme is not required to set every color
    // ensure we always set the "theme*Color" settings since
    // the prefs color picker depends on these values
    auto &display = DkSettingsManager::param().display();

    const QPalette &palette = systemPalette();

    display.themeBgdColor = palette.color(QPalette::Window);
    if (display.defaultBackgroundColor)
        display.bgColor = display.themeBgdColor;

    display.themeFgdColor = palette.color(QPalette::WindowText);
    if (display.defaultForegroundColor)
        display.fgColor = display.themeFgdColor;

    display.themeIconColor = palette.color(QPalette::ButtonText);
    if (display.defaultIconColor)
        display.iconColor = display.themeIconColor;

    // assign a parsed theme color to DkSettings
    // the color is not assigned if user modified it in prefs ("default*Color" is false)
    const auto setColor = [](const QStringList &colorDef) {
        if (colorDef.count() != 2) {
            qWarning() << "invalid color def" << colorDef;
            return;
        }

        const QString &name = colorDef[0];
        const QString &colorSpec = colorDef[1];

        QColor color = QColor(colorSpec);
        if (!color.isValid()) {
            qWarning() << "[theme] invalid color value" << name << colorSpec;
            return;
        }

        auto &dpy = DkSettingsManager::param().display();

        if (name == "BACKGROUND_COLOR") {
            dpy.themeBgdColor = color;
            if (dpy.defaultBackgroundColor)
                dpy.bgColor = color;
        } else if (name == "FOREGROUND_COLOR") {
            dpy.themeFgdColor = color;
            if (dpy.defaultForegroundColor)
                dpy.fgColor = color;
        } else if (name == "HIGHLIGHT_COLOR")
            dpy.highlightColor = color;
        else if (name == "HUD_BACKGROUND_COLOR")
            dpy.hudBgColor = color;
        else if (name == "HUD_FOREGROUND_COLOR")
            dpy.hudFgdColor = color;
        else if (name == "ICON_COLOR") {
            dpy.themeIconColor = color;
            if (dpy.defaultIconColor)
                dpy.iconColor = color;
        } else
            qWarning() << "[theme] unknown color name" << name << color;
    };

    const QStringView inCss(cssString);
    QString outCss;

    bool bufferLines = false;
    QList<QStringView> lineBuffer;
    QStringList expression;

    //
    // stylesheet preprocessor macros (one per line)
    // -------------------------------
    //
    // /* nomacs-color NAME [color-spec] */
    //
    // Sets a color in DkSettings and stylesheet
    // - the user may override colors that have a "default*Color" setting
    // - color-spec conforms to QColor::fromString()
    //
    // -------------------------------
    //
    // /* nomacs-if-<property>-<operator>-<expression> */
    //   [css block]
    // /* nomacs-end-if */
    //
    // Conditional block of CSS
    // - nesting is not possible
    //
    const QList<QStringView> lines = inCss.split('\n');
    for (auto &line : lines) {
        if (line.indexOf(QLatin1String("/* nomacs-color")) >= 0) {
            QString str = line.toString();
            str = str.mid(2, str.length() - 4).trimmed();
            const QStringList colorDef = str.split(' ', Qt::SkipEmptyParts).mid(1);
            setColor(colorDef);
        } else if (line.indexOf(QLatin1String("/* nomacs-if-")) >= 0) {
            QString str = line.toString();
            str = str.mid(2, str.length() - 4).trimmed();
            expression = str.split('-').mid(2);
            bufferLines = true;
        } else if (line.indexOf(QLatin1String("/* nomacs-end-if")) >= 0) {
            bool valid = false;
            bool result = false;
            if (expression.count() == 3) {
                const QString &property = expression.at(0);
                const QString &op = expression.at(1);
                const QString &value = expression.at(2);

                int lhs = 0, rhs = 0;

                if (property == "qt") {
                    const auto v = value.split(',');
                    if (v.count() == 3) {
                        lhs = QT_VERSION;
                        rhs = QT_VERSION_CHECK(v[0].toInt(), v[1].toInt(), v[2].toInt());
                        valid = true;
                    }
                }

                if (valid) {
                    if (op == "gte")
                        result = lhs >= rhs;
                    else if (op == "lt")
                        result = lhs < rhs;
                }
            }

            if (valid && result) {
                for (auto &l : qAsConst(lineBuffer)) {
                    outCss += l;
                    outCss += '\n';
                }
            } else if (!valid)
                qWarning() << "invalid CSS conditional:" << expression;

            bufferLines = false;
            lineBuffer.clear();
        } else if (bufferLines) {
            lineBuffer += line;
        } else {
            outCss += line;
            outCss += '\n';
        }
    }

    return outCss;
}

DkThemeManager::ColorBinding DkThemeManager::cssColors() const
{
    DkSettings &settings = DkSettingsManager::param();
    const auto &d = settings.display();
    const auto &s = settings.slideShow();

    QColor highlightAlpha = d.highlightColor;
    highlightAlpha.setAlpha(150);

    return {{"HIGHLIGHT_COLOR", d.highlightColor}, // note the order, BACKGROUND_COLOR must follow *_BACKGROUND_COLOR
            {"HIGHLIGHT_LIGHT", highlightAlpha},
            {"HUD_BACKGROUND_COLOR", d.hudBgColor},
            {"HUD_FOREGROUND_COLOR", d.hudFgdColor},
            {"BACKGROUND_COLOR", d.bgColor},
            {"FOREGROUND_COLOR", d.fgColor},
            {"SLIDESHOW_COLOR", s.backgroundColor},
            {"ICON_COLOR", d.iconColor}};
}

QString DkThemeManager::replaceMacros(const QString &cssString, const ColorBinding &colors) const
{
    const auto findColor = [&colors](const QString &name) {
        for (auto &c : colors)
            if (c.name == name)
                return c.color;
        return QColor();
    };

    const struct Macro {
        const char *name;
        std::function<QString(const QStringList &)> eval;
    } macros[] = {
        // NOTE: macros must be ordered such that shortest common prefix appears last
        //       e.g. "nomacsBlend" must follow "nomacsBlendAndMultiply"
        {"nomacsBlend",
         [&findColor](const QStringList &args) {
             // blend colors using source-alpha blend mode
             //  QFrame {
             //    background-color: nomacsBlend(BACKGROUND_COLOR,255,0,0,128);
             //  };
             if (args.count() != 5) {
                 qWarning() << "nomacsBlend: expected 5 arguments";
                 return QString("#000");
             }
             QColor base = findColor(args[0]);
             if (!base.isValid())
                 qWarning() << "nomacsBlend: invalid color:" << args[0];

             QColor over(args[1].toInt(), args[2].toInt(), args[3].toInt(), args[4].toInt());
             float sa = over.alpha() / 255.0;
             float da = 1.0 - sa;
             int r = base.red() * da + over.red() * sa;
             int g = base.green() * da + over.green() * sa;
             int b = base.blue() * da + over.blue() * sa;
             return DkUtils::colorToString(QColor(r, g, b));
         }},
    };

    QString outCss = cssString;

    for (const Macro &macro : macros) {
        int begin = 0;
        int end = 0;
        QString css;
        do {
            end = outCss.indexOf(macro.name, begin);
            if (end < 0)
                break;

            css += outCss.mid(begin, end - begin); // chunk before macro started

            int argsIn = outCss.indexOf('(', end) + 1;
            int argsOut = outCss.indexOf(')', argsIn);

            QStringList args = outCss.mid(argsIn, argsOut - argsIn).split(',');

            // qDebug() << macro->name << args << macro->eval(args);
            css += macro.eval(args);

            begin = argsOut + 1;

        } while (true);

        css += outCss.mid(begin);
        outCss = css;
    }

    return outCss;
}

QString DkThemeManager::replaceColors(const QString &cssString, const ColorBinding &colors) const
{
    QString outCss = cssString;
    for (auto &c : colors)
        outCss.replace(c.name, DkUtils::colorToString(c.color));
    return outCss;
}

bool DkThemeManager::isSystemTheme() const
{
    return getCurrentThemeName() == "System.css";
}
}
