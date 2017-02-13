# Page Extraction
Author: Markus Diem
_13.02.2017_

The page extraction plugin detects document pages and either draws them `Draw To Page` or crops the image w.r.t. the rectangle's bounding box `Crop to Page` or `Crop To Metadata` (experimental).

## Algorithm
You can choose between two algorithms:
- Multiple thresholds (default) [0] _by Markus Diem_
- Bashkar [1] _by Thomas Lang_
To choose a method, open `Edit > Settings > Editor > Page Extraction Plugin`.
