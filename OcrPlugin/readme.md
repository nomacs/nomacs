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
Before the first run training data for the optical character system must be downloaded for the languages you are targetting. Once the plugin is loaded go to the menu bar "Plugins" => "DkOcrPlugin" => "Language Settings". Additional languages can be downloaded conveniently in this interface. Only languages selected with the checkboxes on the right are used while performing character recognition.

![languagedownload](https://i.imgur.com/Zlsg3Nv.png)

#### Tesseract Configuration Files
To load a Tesseract config file place it into `plugins\tessdata\configs` and set the value `Tesseract Configuration` in the Nomacs settings editor to the respective filename. Example configurations can be found [here](https://github.com/tesseract-ocr/tesseract/tree/master/tessdata/configs).

![settingsedtiortesseractconfig](https://i.imgur.com/BEU9sGP.png)


### Problems and Notes

LeptonicaConfig.cmake include directories are wrongly formatted
set(Leptonica_INCLUDE_DIRS C:/Program Files/leptonica/include;C:/Program Files/leptonica/include/leptonica)
set(Leptonica_INCLUDE_DIRS "C:/Program Files/leptonica/include" "C:/Program Files/leptonica/include/leptonica")
OR
dont have spaces in your installation paths! (same for tesseract)

link ws2_32.lib manually

libpng is required for the LSTM, if it is not present Tesseract will trigger undefined behaviour -> make a PR for Tesseract
