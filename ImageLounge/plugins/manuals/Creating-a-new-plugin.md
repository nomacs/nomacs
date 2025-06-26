# Creating a Nomacs Plugin

* Clone the `nomacs` and `nomacs-plugins` repositories and build them.
* Have a look at `ImageLounge/src/DkCore/DkPluginInterface.h`. It describes three types:

  * `DkPluginInterface` defines the basic interface every nomacs plugin.
  Example: SIMPLE_PLUGIN.
  * `DkBatchPluginInterface` is derived from `DkPluginInterface` and defines additional members that are needed for the plugin to be used with the batch processing tool.
  Example: `pageExtractionPlugin`.
  * `DkViewPortInterface` is derived from `DkPluginInterface` and is needed for plugins that display additional GUI elements.
  Example: `patchMatchingPlugin`.
* You can add your plugin to the (forked) nomacs-plugins repository or build it separately.
  * In the first case, create a subfolder in `nomacs-plugins/` and use `SIMPLE_PLUGIN` as a template.
  You will have to adjust the `nomacs-plugins/CMakeLists.txt` to include your new plugin in the build process.
  * In the second case, create a new project of your choice (cmake, Visual Studio, etc.) and add the necessary include and library paths yourself.

* Nomacs plugins are located in `[nomacs build dir]/[Debug or Release]/plugins`. The nomacs-plugins cmake automatically sets the output directory to this path.

## Resolving issues

* _Post-build script_: After a plugin is built, the DependencyCollector.py script is called using the `python` command. Make sure that this command executes python 3 on your system. The script will not work with python 2.x.
