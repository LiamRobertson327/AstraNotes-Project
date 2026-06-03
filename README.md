<h1 align="center">AstraNotes-Project</h1>

![logo](ReferenceIcon.png)

[![C++](https://img.shields.io/badge/C++23-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.11.0-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.30-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)

## ⚠️⚠️ Disclaimer & Data Safety⚠️⚠️
* **AstraNotes is currently in active development.**  By using this application, you ackowledge and accept the following:
    * **Alpha Software**: You may encounter bugs or other incomplete features.  Always keep backups of important notes.
    * **Encryption**: Encryption is handled locally using AES-256-GCM.  We do not store your passwords.  If you lose or forget your encryption password, you secured notes are impossible to recover.
    * **Use at your own risk**: This software is provided "as is", without warranty of any kind.  The develoeprs are not responsible for any loss of data or damages resulting from the use of this application.

## 📋 Requirements
* C++23

## Overview
AstraNotes is a plugin-based note taking application built in C++ with the Qt framework. It supports plaintext and markdown notes by default and can be extended with other plugins to include more note formats. Each note can optionally be encrypted using AES-256-GCM using Argon2ID to derive the key. 

## ✨ Features
* Different Note Types
    * Plaintext
    * Markdown
* Encryption to secure Notes
* Autosave feature that runs every 3 seconds following a keystroke
* Manual save of notes
* Search current note content or title
* Search through titles of all loaded in notes
* Dynamic loading of plugins for different note types
* Status indicator of whether the note is saved or not saved

## 🗂️ Repository Structure
```
AstraNotes-Project
├── src/
│   ├── api/
│   │   ├── INote.h
│   │   ├── IPlugin.h
│   │   └── ISnapshot.h
│   ├── app/
│   │   └── main.cpp
│   ├── model/
│   │   ├── Note.h
│   │   ├── Note.cpp
│   │   ├── Snapshot.h
│   │   └── Snapshot.cpp
│   ├── plugins/
│   │   ├── IFormattingAction.h
│   │   ├── MarkdownFormattingPlugin.h
│   │   ├── MarkdownPlugin.h
│   │   ├── MarkdownPlugin.cpp
│   │   ├── PlaintextPlugin.h
│   │   ├── PlaintextPlugin.cpp
│   │   ├── PluginManager.h
│   │   └── PluginManager.cpp
│   ├── repository/
│   │   ├── INoteRepository.h
│   │   └── SqliteNoteRepository.h/.cpp
│   ├── service/
│   │   ├── interfaces/
│   │   │   ├── INoteService.h
│   │   │   ├── ISnapshotService.h
│   │   │   └── ITrashService.h
│   │   └── impl/
│   │       ├── NoteService.h/.cpp
│   │       ├── SnapshotService.h/.cpp
│   │       └── TrashService.h/.cpp
│   ├── crypto/
│   │   ├── EncryptionService.h
│   │   └── EncryptionService.cpp
│   ├── logging/
│   │   ├── AuditLogger.h
│   │   └── AuditLogger.cpp
│   └── ui/
│       ├── mainwindow.ui
│       ├── mainwindow.h
│       ├── mainwindow.cpp
│       ├── NoteListController.h/.cpp
│       ├── AuditLogPanel.h/.cpp
│       └── TrashDialog.h/.cpp
├── tests/
│   ├── unit/
│   │   ├── snapshot_service_tests.cpp
│   │   └── trash_service_tests.cpp
│   └── smoke_tests.cpp
├── docs/
│   ├── PHASE_PROGRESS.md
│   └── PLUGIN_DLL_IMPLEMENTATION_PLAN.md
├── CMakeLists.txt
├── .gitignore
├── LICENSE
└── README.md
```

## 🙋🏻‍♂️ How to Use
Download the latest release found [here](https://github.com/LiamRobertson327/AstraNotes-Project/releases) as a zip file.  Unzip the file and place the folder "AstraNotesTaker" on your desktop.  Inside the folder you will find the executable "AstraNotes.exe".  Simply double click the executable to run it and you are ready to start taking notes.

Note: The application will default to be a markdown note, but you may change this to plaintext by clicking the "+ New Note" button and selecting Plain Text.

## 🔧 Build from source (cross-platform)
These instructions cover building the project from source on Windows, macOS, and Linux. The project uses CMake and Qt6. You can provide Qt via `CMAKE_PREFIX_PATH` or `Qt6_DIR`.

- Install dependencies:
    - Qt 6 (Qt6 Widgets, Core, Sql, Test)
    - CMake >= 3.19
    - A C++23 toolchain (MSVC / clang / gcc)
    - OpenSSL (system or Homebrew on macOS)

- Configure and build (example; use a clean `build/` directory):

```powershell
mkdir build
cd build
cmake -S .. -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
ctest -j 4 --output-on-failure
```

- Notes per platform:
    - Windows: set `CMAKE_PREFIX_PATH` to your Qt install folder or set `Qt6_DIR` to the `lib/cmake/Qt6` path. For MSYS2/MinGW you can set `OPENSSL_ROOT_DIR` to your MSYS2 prefix (e.g. `C:/msys64/ucrt64`).
    - macOS: if you installed OpenSSL via Homebrew, CMake will try `/opt/homebrew/opt/openssl@3` or `/usr/local/opt/openssl@3`; otherwise set `OPENSSL_ROOT_DIR`.
    - Linux: install Qt6 development packages from your distro or provide `CMAKE_PREFIX_PATH` to a Qt installation.

If you need a GUI bundle on macOS or Windows installers, use standard Qt deployment tools (`macdeployqt`, `windeployqt`) or CI packaging workflows. The top-level `CMakeLists.txt` avoids hardcoded Windows-only Qt paths and prefers environment/CMake-provided locations.

## Credits
* **Qt License**: [Obligations of the GPL and LGPL](https://www.qt.io/development/open-source-lgpl-obligations) by Qt.
