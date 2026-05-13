<h1 align="center">AstraNotes-Project</h1>

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
AstraNotes
├──  src/
│      ├── api
│      │    ├── INote.h
│      │    ├── IPlugin.h
│      │    └── ISnapshot.h
│      ├── app
│      │    └── main.cpp
│      ├── model
│      │    ├── Note.cpp
│      │    ├── Note.h
│      │    ├── Snapshot.cpp
│      │    └── Snapshot.h
│      ├── plugins
│      │    ├── IFormattingAction.h
│      │    ├── MarkdownFormattingPlugin.h
│      │    ├── MarkdownPlugin.cpp
│      │    ├── MarkdownPlugin.h
│      │    ├── PlainTextPlugin.cpp
│      │    ├── PlaintextPlugin.h
│      │    ├── PluginManager.cpp
│      │    └── PluginManager.h
│      ├── repoistory
│      │    ├── INoteRepoistory.h
│      │    ├── SqliteNoteRepoistroy.cpp
│      │    └── SqliteNoteRepoistroy.h
│      ├── service
│      │    ├── EncryptionService.cpp
│      │    ├── EncryptionService.h
│      │    ├── NoteService.cpp
│      │    └── NoteService.h
│      ├── ui
│      │    ├── mainwindow.cpp
│      │    ├── mainwindow.h
│      │    └── mainwindow.ui             
├──  tests/
│      ├──  mainwindow.h
│      └──  Note.h
│
├──  Docs/
│ 
├──  .gitignore
├──  CmakeList.txt
├──  LICENSE
└──  README.md
```

## 🙋🏻‍♂️ How to Use
Download the latest release found [here](https://github.com/LiamRobertson327/AstraNotes-Project/releases) as a zip file.  Unzip the file and place the folder "AstraNotesTaker" on your desktop.  Inside the folder you will find the executable "AstraNotes.exe".  Simply double click the executable to run it and you are ready to start taking notes.

Note: The application will default to be a markdown note, but you may change this to plaintext by clicking the "+ New Note" button and selecting Plain Text.

## Credits
* **Qt License**: [Obligations of the GPL and LGPL](https://www.qt.io/development/open-source-lgpl-obligations) by Qt.
