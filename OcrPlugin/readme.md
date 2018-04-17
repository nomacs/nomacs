# Nomacs Ocr Plugin
This plugin enables Nomacs to perform optical character recognition on a loaded image.

# Setup and Compiling

#### Compile Leptonica
Configure and build Leptonica

```
git clone --depth=1 https://github.com/DanBloomberg/leptonica
mkdir leptonica-build
cd leptonica-build
cmake -G "Visual Studio 14 Win64" -DSTATIC:BOOL=ON -DCMAKE_INSTALL_PREFIX:PATH=E:/Programming/nomacs-plugins/OcrPlugin/ext/leptonica-install ../leptonica
cmake --build . --target install
```
Note: CMAKE_INSTALL_PREFIX should be changed to your needs and be absolute

#### Compile Tesseract
Configure and build Tesseract-Ocr

```
git clone --depth=1 https://github.com/tesseract-ocr/tesseract.git
mkdir tesseract-build
cd tesseract-build
cmake -G "Visual Studio 14 Win64" -DSTATIC:BOOL=ON -DBUILD_TESTS:BOOL=OFF -DLeptonica_DIR:PATH=../leptonica-install/cmake -DCMAKE_INSTALL_PREFIX:PATH=E:/Programming/nomacs-plugins/OcrPlugin/ext/tesseract-install ../tesseract
cmake --build . --target install
```
Note: CMAKE_INSTALL_PREFIX should be changed to your needs and be absolute

#### Compile nomacs-plugins
Set Nomacs_DIR, set OpenCV_DIR as usual. 
- set Tesseract_DIR to OcrPlugin/ext/tesseract-install/cmake
- set Leptonica_DIR to OcrPlugin/ext/leptonica-install/cmake
- set ENABLE_OCR=ON

#### Language Files
eng.traineddata is currently in the nomacs-plugins repository and is copied during compilation to the /plugins/tessdata directory. 
~~Download `eng.traineddata` from `https://github.com/tesseract-ocr/tessdata/tree/3.04.00` and place it into `plugins\tessdata`


### Problems and Notes

LeptonicaConfig.cmake include directories are wrongly formatted
set(Leptonica_INCLUDE_DIRS C:/Program Files/leptonica/include;C:/Program Files/leptonica/include/leptonica)
set(Leptonica_INCLUDE_DIRS "C:/Program Files/leptonica/include" "C:/Program Files/leptonica/include/leptonica")
OR
dont have spaces in your installation paths! (same for tesseract)

link ws2_32.lib manually
