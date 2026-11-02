#include "i18n.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace I18n {

const char *const SETTINGS_KEY = "generalSettings/language";

QString normalize(const QString &posixLocale)
{
    QString l = posixLocale.trimmed();
    int cut = l.indexOf(QLatin1Char('.'));
    if (cut >= 0)
        l.truncate(cut);
    cut = l.indexOf(QLatin1Char('@'));
    if (cut >= 0)
        l.truncate(cut);
    l.replace(QLatin1Char('-'), QLatin1Char('_'));

    // Generic Norwegian is translated as Bokmal.
    if (l == QLatin1String("no"))
        return QStringLiteral("nb");
    if (l.startsWith(QLatin1String("no_")))
        return QStringLiteral("nb") + l.mid(2);
    return l;
}

static bool isCLocale(const QString &l)
{
    return l.isEmpty() || l == QLatin1String("C") || l == QLatin1String("POSIX")
        || l.startsWith(QLatin1String("C."));
}

QStringList languagesFromEnvironment(const QProcessEnvironment &env)
{
    QString locale;
    for (const char *var : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
        locale = env.value(QLatin1String(var));
        if (!locale.isEmpty())
            break;
    }
    if (isCLocale(locale))
        return QStringList();

    QStringList result;
    for (const QString &entry : env.value(QStringLiteral("LANGUAGE")).split(QLatin1Char(':'))) {
        if (!isCLocale(entry))
            result << normalize(entry);
    }
    result << normalize(locale);
    return result;
}

QStringList expandFallbacks(const QStringList &languages)
{
    QStringList result;
    for (const QString &l : languages) {
        if (l.isEmpty())
            continue;
        if (!result.contains(l))
            result << l;
        QString base = l.section(QLatin1Char('_'), 0, 0);
        if (!result.contains(base))
            result << base;
    }
    return result;
}

QString match(const QStringList &preferred, const QStringList &available)
{
    for (const QString &l : expandFallbacks(preferred)) {
        // The sources are written in English, so English needs no translation
        // and ends the search, exactly as gettext does.
        if (l == QLatin1String("en") || l.startsWith(QLatin1String("en_")))
            return QStringLiteral("en");
        if (available.contains(l))
            return l;
        for (const QString &a : available) {
            if (a.startsWith(l + QLatin1Char('_')))
                return a;
        }
    }
    return QStringLiteral("en");
}

QStringList availableTranslations()
{
    QStringList result;
    const QStringList files = QDir(QStringLiteral(":/translations"))
                                  .entryList({QStringLiteral("openambit_*.qm")}, QDir::Files, QDir::Name);
    for (const QString &f : files)
        result << f.mid(10, f.length() - 13);  // "openambit_" ... ".qm"
    return result;
}

static void installTranslator(const QString &name, const QString &dir)
{
    QTranslator *t = new QTranslator(qApp);
    if (t->load(name, dir))
        qApp->installTranslator(t);
    else
        delete t;
}

QString install(const QString &forced)
{
    QStringList preferred;
    if (!forced.isEmpty())
        preferred << normalize(forced);
    const QString chosen = QSettings().value(QLatin1String(SETTINGS_KEY)).toString();
    if (!chosen.isEmpty())
        preferred << normalize(chosen);
    const bool explicitChoice = !preferred.isEmpty();
    preferred << languagesFromEnvironment(QProcessEnvironment::systemEnvironment());

    const QString lang = match(preferred, availableTranslations());

    // An explicit choice also sets the number and date formats; otherwise
    // those keep following LC_NUMERIC/LC_TIME, which Qt reads by itself.
    if (explicitChoice)
        QLocale::setDefault(QLocale(lang));

    if (lang != QLatin1String("en")) {
        // Qt's own strings (standard buttons, file dialogs) come from its
        // catalogue, which uses the language part only for most languages.
        const QString qtDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        installTranslator(QStringLiteral("qt_") + lang, qtDir);
        installTranslator(QStringLiteral("openambit_") + lang, QStringLiteral(":/translations"));
    }
    return lang;
}

QString nativeName(const QString &language)
{
    QLocale locale(language);
    QString name = locale.nativeLanguageName();
    if (name.isEmpty())
        return language;
    return name.left(1).toUpper() + name.mid(1);
}

} // namespace I18n
