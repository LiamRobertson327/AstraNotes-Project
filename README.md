<h1 style="text-align: center">AstraNotes-Project</h1>

![logo](ReferenceIcon.png)

[![C++](https://img.shields.io/badge/C++23-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.11.0-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.30-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)

## ⚠️⚠️ Disclaimer & Data Safety⚠️⚠️
* **AstraNotes is currently in active development.**  By using this application, you acknowledge and accept the following:
    * **Alpha Software**: You may encounter bugs or other incomplete features.  Always keep backups of important notes.
    * **Encryption**: Encryption is handled locally using AES-256-GCM.  We do not store your passwords.  If you lose or forget your encryption password, you secured notes are impossible to recover.
    * **Use at your own risk**: This software is provided "as is", without warranty of any kind.  The developers are not responsible for any loss of data or damages resulting from the use of this application.
    * The application does not ship as an installer.  When the project was built and compiled individual dll's were pulled to make the application run.  While working at the time of release, depending on your individual system there is a small possibility a dll is missing.  If this occurs report this to the GitHub repo.  An official installer is planned for a future release.

## 📋 Requirements
- C++23 toolchain
- CMake 3.19 or newer
- Qt 6 with Widgets, Core, SQL, and Test modules
- OpenSSL 3.x
- SQLite runtime and headers if your platform does not provide them with the toolchain

## Overview
AstraNotes is a plugin-based note taking application built in C++ with the Qt framework. It supports plaintext and markdown notes by default and can be extended with other plugins to include more note formats. Each note can optionally be encrypted using AES-256-GCM using Argon2ID to derive the key. 

## ✨ Features
* Different Note Types
    * Plaintext
    * Markdown
* Optional local encryption to secure notes
* Autosave after a short configurable debounce window
* Manual save of notes
* Search current note content or title
* Search through titles of all loaded notes
* Dynamic plugin loading for different note types
* Saved-status indicator and trash workflows

## 🗂️ Repository Structure
The most important folders are:

- `src/app` - application entry point and composition root
- `src/ui` - Qt widgets, dialogs, and controllers
- `src/service` - service interfaces and implementations
- `src/repository` - SQLite persistence
- `src/crypto` - encryption helpers
- `src/logging` - audit logging
- `tests` - smoke, unit, integration, and performance tests
- `docs` - traceability, validation, architecture, and UML references

For a current implementation map, see `docs/architecture/PROJECT_STRUCTURE.md`.

## 🙋🏻‍♂️ How to Use
Depending on your platform download the latest release found [here](https://github.com/LiamRobertson327/AstraNotes-Project/releases) as a zip file (Mac and Windows) or AppImage (Linux).
- **Mac Users**: Unzip the file and place it on your desktop or another folder of your choosing.  Since this application was not uploaded to the App Store, Apple will list it as untrusted.  When you try to open it for the first time, it will not open.  To fix this go to System Settings -> Privacy & Security and scroll down to the security section and click on the **Allow** pop-up for AstraNotes.  You should now be able to use the application.
- **Windows Users**: Unzip the file and place it on your desktop or another folder of your choosing.  Inside the folder you will see the file `AstraNotes.exe`.  Double click on the application to open and use it.
- **Linux Users**: Download the AppImage file and place it on your desktop or in another folder.  Right click on the application, select properties, and click "Allow executing as a program".  For CLI users open a terminal in the same directory as the AppImage file and use the command '''chmod +x AstraNotes.AppImage'''.  After making the AppImage executable, you will be able to use AstraNotes.
- **Developers**: You are free to clone the repository and you can run the application natively in the IDE or code editor such as VsCode.

Note: The application will default to be a markdown note, but you may change this to plaintext by clicking the "+ New Note" button and selecting Plain Text.

## 💻 Keyboard Shortcuts
- **Save Note**: Ctrl + S
- **Find in Note**: Ctrl + F
- **Select Multiple Notes**: Ctrl + Click
- **Cycle to Next Highlighted Search Match**: Enter

## 📝 User Guide
- To create a new note
    - Click on the `+ New Note` button.  A dropdown box will appear with the supported note types.  Clicking on one of them will open up a fresh note of that type.  A title is required, but the body of the note is not to be saved.
- To Save a Note
    - By default auto-save will be enabled, but this can be toggled off via the `Auto-save` checkbox in the top right corner.  To manually save a note, either click the save button or use the shortcut.
- To Edit a note
    - All your saved notes will appear on the left hand side of the application.  You are able to click on any of the notes and they will automatically open.  
- To Delete a note
    - If you want to delete one or multiple notes, you can select them from the list on the left hand side and then click the `Delete` button.
- To Recover a deleted note
    - When a note is deleted it is moved to the trash can.  By default notes will stay in the trashcan and can be recovered for 14 days, after which they will be automatically deleted.  To recover the note, click on the `Trash Can` button and there you will either be able to select one or multiple notes and restore them or purge (permanently delete) them.
- Snapshots
    - Snapshots are created every time a note is manually saved by the user, when a user switches to another note, and when the application closes.  Only two snapshots can exist per note at a time and the user has the option of either deleting the snapshot(s) or restoring the current note to either of them.  This can be accessed by clicking the `History` button.
- To Encrypt a Note
    - When you have a note open, clicking the `Secure` checkbox in the top right corner will change the note state to unsaved.  Upon saving the note, the system will prompt you for a password to encrypt the note with.  This password does not need to be the same as the password used for other notes.  After entering the password, the note is encrypted and every time it is opened you must enter in the correct password.  To remove the encryption, once the note is open clicking the same `Secure` checkbox will remove the encryption.
    - **Warning: If you forget your password for an encrypted note, that specific note is no longer recoverable unless you remember the password.**
- Settings
    - The `Settings` button has 4 user-configurable attributes.
        - **Retention**: The number of days until a trashed note is automatically deleted.  This is configurable from 1 to 60 days.
        - **Enable Automatic purge**: If enabled, every day trashed notes will be permanently deleted.
        - **Enable default encryption**: If enabled, the first time a note is saved the system will also try to encrypt it.  This setting will not affect notes that already exist within the system.
        - **Auto-save debounce interval**: The amount of time after a change to a note is made and the system automatically saving the changes.  This is configurable from 5-10000ms or 0.5-10 seconds.

## 🔧 Build from Source (cross-platform)
The project builds on Windows, macOS, and Linux with CMake and Qt 6.  The CMakeLists.txt used to build the project is included.  Alternatively:

```powershell
mkdir build
cd build
cmake -S .. -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
ctest -j 4 --output-on-failure
```

Platform notes:

- Windows: set `CMAKE_PREFIX_PATH` to your Qt install folder. If you use MSYS2 or another non-default OpenSSL install, set `OPENSSL_ROOT_DIR` as needed.
- macOS: Homebrew OpenSSL is supported through the standard `/opt/homebrew/opt/openssl@3` or `/usr/local/opt/openssl@3` prefixes.
- Linux: install Qt 6 development packages or point `CMAKE_PREFIX_PATH` at a Qt install that includes the Widgets, Sql, and Test modules.

If you need a GUI bundle on macOS or Windows installers, use standard Qt deployment tools (`macdeployqt`, `windeployqt`) or CI packaging workflows. The top-level `CMakeLists.txt` avoids hardcoded Windows-only Qt paths and prefers environment/CMake-provided locations.

## Documentation
- Validation matrix: `docs/validation/TEST_VALIDATION_TRACEABILITY.md`
- Test inventory: `tests/TESTS.md`
- Traceability chain: `docs/validation/TRACEABILITY_CHAIN.md`
- UML reference: `docs/legacy/UML_STARTER_DIAGRAMS.md`
- UML Files: `docs/UML diagrams`
- AI oversight: `docs/validation/AI_HUMAN_OVERSIGHT.md`
- Operations notes: `docs/operations/OPERATIONS_NOTES.md`

## Credits
* **Qt License**: [Obligations of the GPL and LGPL](https://www.qt.io/development/open-source-lgpl-obligations) by Qt.
