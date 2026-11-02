#ifndef I18N_H
#define I18N_H

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

// User interface language selection.
//
// The language follows the GNU gettext rules that every Linux desktop uses,
// GNOME included (GNOME writes the user's choice from Settings > Region &
// Language into these same variables when the session starts, so reading them
// covers GNOME without linking any GNOME library):
//
//   1. If the effective locale (the first set of LC_ALL, LC_MESSAGES, LANG) is
//      "C" or "POSIX", the interface stays in English and LANGUAGE is ignored.
//   2. Otherwise LANGUAGE, a colon-separated priority list ("fr_BE:fr:en"),
//      comes first, followed by the effective locale.
//
// Each entry falls back from the most to the least specific form
// ("pt_BR.UTF-8@euro" -> "pt_BR" -> "pt"). The first entry that has a
// translation wins; reaching "en" first means English, as in gettext.
//
// An explicit choice (the "-locale:xx" argument, then the "language" setting)
// overrides the environment.
namespace I18n {

// Settings key holding the user's explicit choice; empty means "system".
extern const char *const SETTINGS_KEY;

// Strips the codeset and modifier ("de_DE.UTF-8@euro" -> "de_DE") and maps
// aliases to the names the translations use ("no" -> "nb").
QString normalize(const QString &posixLocale);

// Ordered language list derived from the environment, by the rules above.
QStringList languagesFromEnvironment(const QProcessEnvironment &env);

// Adds the language-only fallback after each entry and removes duplicates.
QStringList expandFallbacks(const QStringList &languages);

// Picks the first entry of `preferred` available in `available` (e.g. "nb"
// matches "nb_NO"). Returns "en" when English comes first or nothing matches.
QString match(const QStringList &preferred, const QStringList &available);

// Languages that have a compiled translation in the resources.
QStringList availableTranslations();

// Selects and installs the translators. `forced` is the "-locale:" argument,
// if any. Returns the language that was installed ("en" when none was).
QString install(const QString &forced = QString());

// Name of a language in that language, for the settings dialog.
QString nativeName(const QString &language);

} // namespace I18n

#endif // I18N_H
