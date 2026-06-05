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

## 🔧 Build from Source (cross-platform)
The project builds on Windows, macOS, and Linux with CMake and Qt 6.

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
- AI oversight: `docs/validation/AI_HUMAN_OVERSIGHT.md`
- Operations notes: `docs/operations/OPERATIONS_NOTES.md`

## Credits
* **Qt License**: [Obligations of the GPL and LGPL](https://www.qt.io/development/open-source-lgpl-obligations) by Qt.
