# Batch Processing
Florian Kleber
_kleber@caa.tuwien.ac.at_
_02.03.2017_

# Usage


- Tools > Batch Processing
  - opens the batch processing tab
  - Input: choose input by selecting files and press 'Batch Process'. Check the selected files in the tab 'File List'
  - Adjustments and Transform: optionally select 'Adjustments' (e.g. Grayscale, Normalize) or 'Transform' (e.g. Resize or Orientation) as preprocessing
  - Plugins: all loaded plugins are shown in the list. You can choose one or more plugins that are applied to the preprocessed image. If a plugin is selected all corresponding settings are shown. These can be configured individually for the batch processing.
  - Output: select the directory and filename where to save the output image. If no image is saved, only the corrsponding PAGE xml file is saved.
  - Profiles: allows to save or load a profile. In the list all available (saved) profiles are shown. Select a specific profile to execute it or save the current profile by 'Create New Profile'. If 'Apply Default is pressed' all settings are reset.

# Note
  If a profile is loaded and if a parameter of the selected plugin is changed, the parameter value change is immediatly saved (you do not have to save the profile again). If Input/Adjustments/Transform/Output or the plugin selection itself is changed, the profile must be saved again to store all changes.

  Thus, if you like to to have a profile with different parameter settings, you have to load a profile, save it with a different name and change the parameter values.
