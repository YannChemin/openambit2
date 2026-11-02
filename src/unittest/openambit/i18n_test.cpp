#include <openambit/i18n.h>
#include "doctest.h"

TEST_SUITE_BEGIN("I18n");

static QProcessEnvironment env(std::initializer_list<std::pair<const char *, const char *>> vars)
{
    QProcessEnvironment e;
    for (const auto &v : vars)
        e.insert(v.first, v.second);
    return e;
}

TEST_CASE("normalize strips codeset and modifier") {
    CHECK(I18n::normalize("de_DE.UTF-8@euro") == "de_DE");
    CHECK(I18n::normalize("fr_FR.UTF-8") == "fr_FR");
    CHECK(I18n::normalize("pt-BR") == "pt_BR");
    CHECK(I18n::normalize("no_NO") == "nb_NO");
    CHECK(I18n::normalize("no") == "nb");
}

TEST_CASE("gettext precedence: LANGUAGE, then LC_ALL, LC_MESSAGES, LANG") {
    CHECK(I18n::languagesFromEnvironment(env({{"LANG", "fr_FR.UTF-8"}})) == QStringList{"fr_FR"});
    CHECK(I18n::languagesFromEnvironment(env({{"LANG", "fr_FR.UTF-8"}, {"LC_MESSAGES", "de_DE.UTF-8"}}))
          == QStringList{"de_DE"});
    CHECK(I18n::languagesFromEnvironment(env({{"LANG", "fr_FR.UTF-8"}, {"LC_ALL", "it_IT.UTF-8"}}))
          == QStringList{"it_IT"});
    CHECK(I18n::languagesFromEnvironment(env({{"LANG", "fr_FR.UTF-8"}, {"LANGUAGE", "sv:de"}}))
          == (QStringList{"sv", "de", "fr_FR"}));
}

TEST_CASE("the C locale disables LANGUAGE") {
    CHECK(I18n::languagesFromEnvironment(env({{"LANG", "C"}, {"LANGUAGE", "fr"}})).isEmpty());
    CHECK(I18n::languagesFromEnvironment(env({{"LC_ALL", "C.UTF-8"}, {"LANG", "fr_FR"}})).isEmpty());
    CHECK(I18n::languagesFromEnvironment(env({})).isEmpty());
}

TEST_CASE("matching falls back to the language and stops at English") {
    const QStringList available = {"de", "fr", "nb_NO", "sv"};
    CHECK(I18n::match({"fr_BE"}, available) == "fr");
    CHECK(I18n::match({"nb"}, available) == "nb_NO");
    CHECK(I18n::match({"es_ES", "de_AT"}, available) == "de");
    CHECK(I18n::match({"en_GB", "fr_FR"}, available) == "en");
    CHECK(I18n::match({"es_ES"}, available) == "en");
    CHECK(I18n::match({}, available) == "en");
}
