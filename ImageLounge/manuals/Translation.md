# Translating Nomacs

## Hints for Translating Qt Applications

## Resources

Current Crowdin Project Page
https://crowdin.com/project/nomacs-neo

Old Crowdin Project
https://crowdin.com/project/nomacs

Qt Linguist
https://doc.qt.io/qt-6/linguist-translators.html

KDE Translation HOWTO
https://l10n.kde.org/docs/translation-howto/

## Modifying a Translation (Crowdin)

1. Create an account on Crowdin and make your changes
2. Open an issue on github
3. A project contributor will create a pull request with the updated translation

## Adding a New Language (Crowdin)

1. Open an issue on github
2. A Crowdin manager will add the language to Crowdin
3. Translate on Crowdin
4. Update your github issue when completed
5. A project contributor will create a pull request with the translation 

## Modifying a Translation (not Crowdin)

We recommend using Crowdin, but it is not absolutely necessary if you prefer a different workflow.

1. Create a new feature branch in git for your changes
2. We recommend pulling the current Crowdin translation if it is newer
3. Run `update-translation.sh` to get new untranslated strings
4. Edit the translation .ts file with your tool of choice
5. Compile nomacs to test your changes
6. Create a pull request with your changes
7. Crowdin Managers: After merging, update the new translation in Crowdin

## Adding a New Language (not Crowdin)

1. Run `update-translation.sh` to make `nomacs.ts` current
2. Copy `ImageLounge/translations/src/nomacs.ts` 
  to `ImageLounge/translations/nomacs_lang.ts`,
  where `lang` is the correct language code of the localization.
3. Add the correct "language" and "sourcelanguage" to the file header
```xml
<TS version="2.1" language="ko" sourcelanguage="en">
```
4. Edit the file using Qt `linguist` program or your editor of choice
5. Recompile nomacs
6. Submit a pull request with your changes

## Updating Translation Files for Source Code Changes

.ts files must be periodically updated to reflect latest changes to source code.
This should be done before any import procedure and committed separately.
The `lupdate` command looks for `tr()` macros and adds them to the .ts file specified.

Note that whenever `nomacs.ts` changes, it is ideal to update it on Crowdin as it
is the source document. It is not necessary to update all translation files on Crowdin.

We have a script to simplify this process:
```
# this needs to be the working directory
cd ImageLounge/translations/src

# update .ts files for changes to source code (tr()) macros
./update-translations.sh

# check the log for problems
less update.log

# if there are any problems (missing Q_OBJECT etc), go fix them

# repeat until no more errors are logged

# commit changes
```

## Importing Translations from Crowdin

We have a script that downloads from Crowdin and merges
the changes. It requires the official python api for Crowdin
and a Crowdin API token which you can obtain with an account.

Note you should update translation files before/after pulling Crowdin changes,
this will give the smallest changeset.

```
export CROWDIN_TOKEN="<crowdin-api-token>"
cd ImageLounge/translations/src
./pull-crowdin-translations.sh
```

## Importing Translations from Other Sources

If this is desired, the best thing is to upload the translation
to Crowdin, resolve any issues on that side and then do the normal
Crowdin import.

Nevertheless, it is possible to do this should Crowdin be shut down
or nomacs changes to another service/workflow:

- Place .ts files to import into a new folder
- Ensure names match the nomacs filenames
- Ensure Unix line endings

```
# from ImageLounge/translations/src
./import-translations.sh <path-to-new-folder>

# cleanup; after commit update-translations should not modify anything
./update-translation.sh
```

## Updating/Adding External Translations to Crowdin

For translation files edited outside of Crowdin or if setting up a new Crowdin
account. The file should already be updated (`update-translation.sh`) and committed

1. Add target language(s) to project settings

2. Translations->Upload Translations

- Add .ts file(s)
- Source file should be nomacs.ts
- Check correct target lanaguage is associated
- Import settings:
  + Allow target to match source [ON]
  + Approve added translations [ON]
  + Translate hidden strings [OFF]
  + Add translations to TM [ON]
