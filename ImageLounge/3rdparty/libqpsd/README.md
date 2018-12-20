libqpsd
=======

PSD (Photoshop Document) & PSB (Photoshop Big) Plugin for Qt/C++ (Qt4/Qt5)


This project is based on the Photoshop File Format Specification found in http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm

### FEATURES

#### Supported Formats

- [X] PSD
- [X] PSB

#### Supported Compressions

- [X] Raw
- [X] RLE
- [ ] Zip without Prediction
- [ ] Zip with Prediction

#### Supported Color Modes

- [X] Bitmap
- [X] Grayscale
- [X] Indexed
- [X] RGB
- [X] CMYK
- [X] Multichannel
- [X] Duotone
- [X] Lab

#### Supported Depths

- [X] 1-bit (Bitmap)
- [X] 8-bits
- [X] 16-bits (Note: 16-bit depth is scaled down to 8-bits)
- [ ] 32-bits (Tonemapping Algorithm, needed help!)


### CONTRIBUTE

- If you have an existing PSD/PSB file with different color mode or compression. Feel free to send me an [email](mailto:ronmarti18@gmail.com). I need samples with zip-compressed layers (zip w/ or w/o prediction).
- If you want to contribute to the code, just fork the project and pull requests.
- If you want to keep this project alive you can send your donations to:
    - Bitcoin: 3A23hHJF8q8hNPz3sedqD9T7g25ELPcvg2
    - Paypal: https://www.paypal.me/RonieMartinez

### CONTRIBUTORS

- Ronie Martinez aka Code ReaQtor (ronmarti18@gmail.com)
- Yuezhao Huang (huangezhao@gmail.com)
- username "asgohtals" (http://qt-project.org/member/136052) - contributed PSB files for testing: 
    - berlin-cmyk.psb
    - berlin-quadtone.psb
    - wall-small-Lab.psb

### LIST OF SOFTWARE USING libqpsd

- [nomacs](http://nomacs.org/)
- [EzViewer](https://github.com/yuezhao/ezviewer)
- [PhotoQt](http://photoqt.org/)
- [Seer](http://www.1218.io/)
- *for applications not included in the list, shoot me an email*

### REFERENCES

- http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm
- http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
- http://help.adobe.com/en_US/photoshop/cs/using/WSfd1234e1c4b69f30ea53e41001031ab64-73eea.html#WSfd1234e1c4b69f30ea53e41001031ab64-73e5a
- http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf
