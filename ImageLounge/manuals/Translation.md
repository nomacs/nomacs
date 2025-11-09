# Translating Nomacs

## Using Crowdin

Nomacs uses Crowdin as the primary translation tool, though it is not strictly required (see "not Crowdin" sections here)
If your language is not available on Crowdin, please open an issue on github).
It is obligatory to register at Crowdin to contribute changes.
Your translation will become available when translations are integrated into the next release.
You may expedite this process by opening an issue on github.

## Resources

Current Crowdin Project Page
https://crowdin.com/project/nomacs-neo

Old Crowdin Project
https://crowdin.com/project/nomacs

Qt Linguist
https://doc.qt.io/qt-6/linguist-translators.html

KDE Translation HOWTO
https://l10n.kde.org/docs/translation-howto/

## Hints for Translating Nomacs

First translate “nmc::DkGlobalSettingsWidget” -> “English” to the name of your language
e.g. for german “English” is translated to “Deutsch”.
This string will be displayed in the preferences when choosing the language.

Crowdin will suggest translations from its database of known translations.
Sometime the suggestions of Crowdin are fine, but often you have to change them.
You can filter for untranslated strings, or machine, AI translated strings.

Different special characters are used within the strings:

- “&”: The ampersand is used as mnemonic in the menu.
       This means it is the shortcut when pressing the alt key.
       For example: The “File” menu is translated as “&File”,
       this means the “F” is underlined in the menu and ALT+F can be used as shortcut to access this menu.

- “%1”: This string is always replaced with strings which are needed for a meaningful message.
        In most of the cases these strings are either image names or directories
        e.g. “Could not save: %1″, here %1 will be replaced with the image name.
        Note: due to some reasons we also have to use %1 for the “°” sign like for rotating the image: “9&0%1 Clockwise”

If the string or its context is unclear,
you can use the comment field on Crowdin for questions
and we will try to answer them as fast as possible.

## How to Add a Crowdin Translation to Your Local Machine

You can add a translation without waiting for a nomacs release.
It must be converted from `.ts` to `.qm` format using `lupdate`.

1. Download the language file (.ts) from Crowdin
2. Install Qt Creator (pick the open source installer)
3. Open a command line and cd to the directory with the downloaded translations (*.ts)
4. Then type (assuming you downloaded the Catalan translation):

    - Windows (change the Qt path accordingly)

        ```powershell
        C:\Qt\6.9.1\msvc2022_64\bin\lrelease.exe nomacs_ca_ES.ts
        ```

    - Linux

        ```shell
        lrelease nomacs_ca_ES.ts
        ```

Now you should have a file named `nomacs_ca_ES.qm` in the same directory.
Copy this file to the translations folder of nomacs (on windows: `C:/Program Files/nomacs/bin/translations`).

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
3. Run `update-translation.sh --keep-line-numbers` to get new untranslated strings
4. Edit the translation .ts file with your tool of choice
5. Run `update-translation.sh` to remove line numbers from .ts
6. Compile nomacs to test your changes
7. Create a pull request with your changes
8. Crowdin Managers: After merging, update the new translation in Crowdin

## Adding a New Language (not Crowdin)

1. Run `update-translation.sh --keep-line-numbers` to make `nomacs.ts` current
2. Copy `ImageLounge/translations/src/nomacs.ts` 
  to `ImageLounge/translations/nomacs_lang.ts`,
  where `lang` is the correct language code of the localization.
3. Add the correct "language" and "sourcelanguage" to the file header
```xml
<TS version="2.1" language="ko" sourcelanguage="en">
```
4. Edit the file using Qt `linguist` program or your editor of choice
5. Run `update-translation.sh` to drop line numbers from .ts file
6. Recompile nomacs
7. Submit a pull request with your changes

## Updating Translation Files for Source Code Changes

.ts files must be periodically updated to reflect latest changes to source code.
This should be done before any import procedure and committed separately.
The `lupdate` command looks for `tr()` macros and adds them to the .ts file specified.

Note that whenever `nomacs.ts` changes, it is ideal to update it on Crowdin as it
is the source document. It is not necessary to update all translation files on Crowdin.

We have a script to simplify this process. It should always be used because
it does extra steps to ensure the cleanest possible diff.

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

- Run `update-translation.sh` and commit any changes
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
