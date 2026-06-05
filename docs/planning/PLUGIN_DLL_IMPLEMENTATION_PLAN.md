# Dynamic Plugin (DLL) Implementation Plan

> **After Project Completion Note**: This was not completed.  The project has a plugin system, however it uses h/cpp files instead of loading dlls.  Plugin migration to dlls is planned for a future release and it will migrate into the code without major issues or other merge blockers.

Goal
----
Add support for loading runtime plugins (DLL/shared libraries) using Qt's plugin infrastructure (QPluginLoader). This will allow developers to ship new note formats (e.g., voice notes) as separate libraries which AstraNotes can discover and load at runtime.

Assumptions
-----------
- Current codebase exposes `IPlugin` in `src/api/IPlugin.h` and uses `PluginManager` as an in-memory registry.
- Qt is available and the project uses CMake.

High-level approach
-------------------
1. Convert the public plugin contract to a proper Qt plugin interface (if needed).
2. Make plugin classes (MarkdownPlugin) implement `QObject` + the interface and add `Q_PLUGIN_METADATA` / `Q_INTERFACES`.
3. Build plugins as shared libraries (CMake `add_library(... SHARED ...)`).
4. Extend `PluginManager` to discover and load `.dll`/`.so` files from a plugins folder using `QPluginLoader` and register instances.
5. Adjust formatting-action ownership so crossing an ABI boundary is safe (see discussion below).

Concrete tasks
--------------
1) Interface and plugin class changes
   - Option A (minimal): Keep `IPlugin` as-is but ensure plugin classes also use `Q_INTERFACES(IPlugin)` and `Q_PLUGIN_METADATA`. This often works since `IPlugin` already uses `Q_DECLARE_INTERFACE`.
   - Option B (cleaner): Split into a pure abstract interface (e.g. `IPluginInterface`) in `src/api/` (no QObject), then have plugin implementations inherit `QObject` and `IPluginInterface`.
   - Add `Q_PLUGIN_METADATA(IID "com.astranotes.IPlugin/1.0")` and `Q_INTERFACES(IPlugin)` to the plugin class implementation (e.g., `MarkdownPlugin`).

2) Plugin ownership / formatting actions
   - Current: `getFormattingActions()` returns `IFormattingAction*` instances allocated inside the plugin. Across DLL boundaries this can be fragile.
   - Recommended: change `getFormattingActions()` to return a list of simple descriptors (id, name, icon path, tooltip). The host (AstraNotes) will create `QAction` objects and forward a trigger to the plugin by calling `executeFormattingAction(actionId, QTextEdit*)` on the plugin. This keeps small-value data across ABI and keeps object ownership in the host.

3) Plugin build changes (CMake)
   - Add a new CMake target for the plugin, example:

```cmake
add_library(MarkdownPlugin SHARED
    src/plugins/MarkdownPlugin.cpp
    src/plugins/MarkdownFormattingPlugin.cpp
)
target_include_directories(MarkdownPlugin PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(MarkdownPlugin PRIVATE Qt6::Core Qt6::Widgets)
set_target_properties(MarkdownPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
```

   - Remove the plugin .cpp from the `ASTRA_CORE_SOURCES` glob or conditionally exclude the plugin sources when building the app (so you don't link the same code twice).

4) Plugin discovery and loading (PluginManager)
   - At startup, `PluginManager` will:
     - Ensure the `plugins/` directory exists relative to the executable (e.g., `$APP_DIR/plugins`).
     - Iterate files with `QDir("plugins").entryList(QStringList() << "*.dll" << "*.so" << "*.dylib", QDir::Files)`.
     - For each file, call `QPluginLoader loader(path); QObject *inst = loader.instance();` and `auto plugin = qobject_cast<IPlugin*>(inst);`.
     - If successful, store both `loader` (or a pointer to it) and the `IPlugin*` in the manager. Keep the loader alive while plugin is in use.

   - Example minimal code (conceptual):

```cpp
QPluginLoader *loader = new QPluginLoader(path);
QObject *obj = loader->instance();
if (obj) {
    IPlugin *p = qobject_cast<IPlugin*>(obj);
    if (p) {
        // keep loader in a map keyed by formatId
        m_loaders[p->formatId()] = loader;
        registerPlugin(p);
    } else {
        delete loader; // not our plugin
    }
} else {
    delete loader; // failed to instantiate
}
```

5) Backwards compatibility & fallback
   - Keep existing static plugins (PlaintextPlugin) compiled into the core as a fallback if dynamic load fails or if no plugin file found.
   - Plugin discovery should only *add* new plugins; existing in-memory registration can remain for statically linked ones.

6) Packaging & developer workflow
   - CI: build plugin shared target and place the produced library into `build/plugins/` so the app finds it during local testing.
   - For distribution, package the plugins folder with the main app installer.

7) Testing
   - Add a small integration test that runs the app in headless mode, loads the plugins folder (with the MarkdownPlugin built as a shared lib) and verifies `PluginManager::availableFormats()` contains `markdown`.
   - Add a manual test checklist: drop plugin DLL into `plugins/` and verify UI shows the markdown toolbar/actions and rendering.

Risks & caveats
----------------
- ABI and CRT mismatch can create crashes if host and plugin are built with different runtime settings (MSVC CRT mismatch). For local demos, build with the same toolchain and runtime.
- Resource paths inside plugin (icons, resources) should be compiled into the plugin or use remote resource references; ensure `Q_INIT_RESOURCE` or qrc is used in plugin if needed.
- Ownership across DLLs: avoid new/delete of objects across the boundary unless you guarantee CRT alignment; prefer host-owned `QAction` objects and plugin callback invocation.

Example conversion checklist (MarkdownPlugin)
-------------------------------------------
1. Modify `src/plugins/MarkdownPlugin.h`/`.cpp` to add `Q_PLUGIN_METADATA(IID "com.astranotes.IPlugin/1.0")` and `Q_INTERFACES(IPlugin)` in the `.cpp` implementation file.
2. Move `MarkdownPlugin` target into a new `add_library(... SHARED ...)` in CMake, and place output into `build/plugins`.
3. Update `CMakeLists.txt` to no longer include `src/plugins/MarkdownPlugin.cpp` in `ASTRA_CORE_SOURCES` when building the main library.
4. Implement `PluginManager::loadPluginsFromFolder(const QString &folder)` which scans and tries to load plugin libraries.
5. Update UI initialization to call `PluginManager::instance().loadPluginsFromFolder(appPluginFolder)` before building formatting toolbar.

Deliverables
------------
- `docs/PLUGIN_DLL_IMPLEMENTATION_PLAN.md` (this file)
- Small example plugin target in `CMakeLists.txt` showing how to build `MarkdownPlugin` as a shared lib
- `PluginManager` runtime discovery and loader map
- One converted plugin (Markdown) as a sample DLL
- Integration test confirming discovery

If you want, I can implement the minimal POC changes now: convert `MarkdownPlugin` to a shared plugin target, add `Q_PLUGIN_METADATA`/`Q_INTERFACES`, and add a simple `PluginManager::loadPluginsFromFolder()` and build rule that installs the plugin into `build/plugins/` for the dev run. Mark what you prefer and I will implement the POC.
