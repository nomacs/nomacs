libqpsd
=======

PSD (Photoshop Document) & PSB (Photoshop Big) Plugin for Qt/C++ (Qt4/Qt5)


This project is based on the Photoshop File Format Specification found in http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm

Format:

	PSD - SUPPORTED
	
	PSB - SUPPORTED
	
Compression:

	Raw - SUPPORTED

	RLE - SUPPORTED

	Zip without Prediction - Unsupported

	Zip with Prediction - Unsupported
	
Color Mode

	Bitmap - SUPPORTED

	Grayscale - SUPPORTED

	Indexed - SUPPORTED

	RGB - SUPPORTED

	CMYK - SUPPORTED

	Multichannel - SUPPORTED

	Duotone - SUPPORTED

	Lab - SUPPORTED (minor bug on Lab to RGB computation exists)
	
Depth

	1-bit - SUPPORTED (Bitmap)
	
	8-bits - SUPPORTED
	
	16-bits - SUPPORTED (RGB ONLY)*
	
	32-bits - SUPPORTED (RGB ONLY)*
	
	*16-bit and 32-bit depths are scaled down to 8-bits


CONTRIBUTE

	If you have an existing PSD/PSB file with different color mode or compression. Feel free to send me
	an email at ronmarti18@gmail.com
	
	If you want to contribute to the code, just fork the project and pull requests.

CONTRIBUTORS

	Ronie Martinez aka Code ReaQtor (ronmarti18@gmail.com)
	Yuezhao Huang (huangezhao@gmail.com)
	
	username "asgohtals" (http://qt-project.org/member/136052) - contributed PSB files for testing: berlin-cmyk.psb, berlin-quadtone.psb & wall-small-Lab.psb

	
