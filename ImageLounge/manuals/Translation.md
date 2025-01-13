
WIP documentation for translations. The steps used in the past are unknown
due to no documentation and contributors going dormant.

## Hints for Translating Qt Applications

### Resources

https://crowdin.com/project/nomacs
https://doc.qt.io/qt-6/linguist-translators.html
https://l10n.kde.org/docs/translation-howto/


## Contributing on CrowdIN


## Adding a New Language Manually (not Crowdin)

- Copy translation template `ImageLounge/translations/src/nomacs.ts` 
  to `ImageLounge/translations/nomacs_lang.ts`,
  where `lang` is the correct name of the localization.
- Add the correct "language" and "sourcelanguage" to the file header
```xml
<TS version="2.1" language="ko" sourcelanguage="en">
```
- Edit the file using Qt `linguist` program or your editor of choice
- Recompile nomacs

## Updating Translation files for Crowdin Changes

- Crowdin pushes to nomacs/translations_master automatically, and then we can collect them in batches and merge to master:
- turn off crowdin updates temporarily to prevent merge conflicts?
- git branch -b tx upstream/nomacs_translations
- git merge upstream/master
- git rebase upstream/master
- squash commits, title: Translations: German, French, etc
- run "lupdate" on all .ts files (translations/src/update-translations.sh -- WIP)
- git commit -a, title: Update Translations
- git push upstream/nomacs_translations
- git push upstream/master
- git branch --delete tx
- turn crowdin updates back on (should pull new translation files??)Crowdin pushes to nomacs/translations_master automatically, and then we can collect them in batches and merge to master:

- turn off crowdin updates temporarily to prevent merge conflicts?
- git branch -b tx upstream/nomacs_translations
- git merge upstream/master
- git rebase upstream/master
- squash commits, title: Translations: German, French, etc
- run "lupdate" on all .ts files (translations/src/update-translations.sh -- WIP)
- git commit -a, title: Update Translations
- git push upstream/nomacs_translations
- git push upstream/master
- git branch --delete tx
- turn crowdin updates back on (should pull new translation files??)

## Updating Translation Files for Source Code Changes

.ts files need to be updated to reflect latest changes to source code,
the `lupdate` command looks for `tr()` macros and adds them to the .ts file specified.

```
# this needs to be the working directory
cd ImageLounge/translations/src

# update .ts files for changes to source code (tr()) macros
./update-translations.sh

# check the log for problems
less update.log

# if there are any problems (missing Q_OBJECT etc), go fix them

# repeat until no more errors logged
```

