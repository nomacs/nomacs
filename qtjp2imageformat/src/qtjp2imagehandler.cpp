/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtjp2imagehandler.h"
#include <QImage>
#include <QStringList>
#include <QString>
#include <QColor>
#include <QFile>
#include <QIODevice>
#include <QImageIOHandler>
#include <QVariant>

#ifdef Q_CC_MSVC
    #define JAS_WIN_MSVC_BUILD
#endif
#include <jasper/jasper.h>

enum SubFormat { Jp2Format, J2kFormat };

/*
    \class Jpeg2000JasperReader
    \brief Jpeg2000JasperReader implements reading and writing of JPEG 2000 image files.

    \internal

    This class is designed to be used together with the an QImageIO IOHandler, and
    it should probably not be neccesary to instanciate it directly.

    Internally it used the Jasper library for coding the image data.
*/
class Jpeg2000JasperReader {
public:
    Jpeg2000JasperReader(QIODevice *iod, const SubFormat format = Jp2Format);

    ~Jpeg2000JasperReader();

    bool read(QImage *pImage);
    bool write(const QImage &image, int quality);
private:
    typedef void (Jpeg2000JasperReader::*ScanlineFunc)(jas_seqent_t** const , uchar*);
    typedef void (Jpeg2000JasperReader::*ScanlineFuncWrite)(jas_matrix_t**, uchar* );

    void copyJasperQt(ScanlineFunc scanlinecopier);
    void copyJasperQtGeneric();
    void copyScanlineJasperQtRGB(jas_seqent_t ** const jasperRow,  uchar * qtScanLine);
    void copyScanlineJasperQtRGBA(jas_seqent_t ** const jasperRow,  uchar * qtScanLine);
    void copyScanlineJasperQtGray(jas_seqent_t ** const jasperRow,  uchar* qtScanLine);
    void copyScanlineJasperQtGrayA(jas_seqent_t ** const jasperRow,  uchar* qtScanLine);

    void copyQtJasper(const ScanlineFuncWrite scanlinecopier);
    void copyScanlineQtJasperRGB(jas_matrix_t **  jasperRow,  uchar * qtScanLine);
    void copyScanlineQtJasperRGBA(jas_matrix_t ** jasperRow,  uchar * qtScanLine);
    void copyScanlineQtJasperColormapRGB(jas_matrix_t **  jasperRow,  uchar * qtScanLine);
    void copyScanlineQtJasperColormapRGBA(jas_matrix_t ** jasperRow,  uchar * qtScanLine);
    void copyScanlineQtJasperColormapGrayscale(jas_matrix_t ** jasperRow,  uchar * qtScanLine);
    void copyScanlineQtJasperColormapGrayscaleA(jas_matrix_t ** jasperRow,  uchar * qtScanLine);

    bool attemptColorspaceChange(int wantedColorSpace);
    bool createJasperMatrix(jas_matrix_t **&matrix);
    bool freeJasperMatrix(jas_matrix_t **matrix);
    void printColorSpaceError();
    jas_image_cmptparm_t createComponentMetadata(const int width, const int height);
    jas_image_t *newRGBAImage(const int width, const int height, bool alpha);
    jas_image_t *newGrayscaleImage(const int width, const int height, bool alpha);
    bool decodeColorSpace(int clrspc, QString &family, QString &specific);
    void printMetatdata(jas_image_t *image);

    bool jasperOk;

    QIODevice *ioDevice;
    QImage qtImage;
    SubFormat format;

    //Qt image properties
    int qtWidth;
    int qtHeight;
    int qtDepth;
    int qtNumComponents;

    jas_image_t *jasper_image;
    //jasper image properties
    int jasNumComponents;
    int jasComponentPrecicion[4];
    int computedComponentWidth ;
    int computedComponentHeight;
    int computedComponentHorizontalSubsampling;
    int computedComponentVerticalSubsampling;
    int jasperColorspaceFamily;
    //maps color to component ( ex: colorComponentMapping[RED]
    //gives the component that contains the red color )
    int colorComponentMapping[4];
    bool hasAlpha;
};

/*!
    \class QtJP2ImageHandler
    \brief The QtJP2ImageHandler class provides support for reading and writing
    JPEG 2000 image files with Qts plugin system.
    Currently, it only supports dynamically-loaded plugins.

    JPEG files comes in two subtypes: the JPEG 2000 file format (\c .jp2) and
    the JPEG 2000 code stream format (\c .j2k, \c .jpc, or \c .j2c).
    QtJP2ImageHandler can read and write both types.

    To select a subtype, use the setOption() function with the QImageIOHandler::SubType
    option and a parameter value of "jp2" or "j2k".

    To set the image quality when writing, you can use setOption() with the
    QImageIOHandler::Quality option, or QImageWriter::setQuality() if your are using a
    QImageWriter object to write the image. The Image quality is specified as an int in
    the range 0 to 100. 0 means maximum compression and 99 means minimum compression.
    100 selects lossless encoding, and this is the default value.

    The JPEG handler is only available as a plugin,
    and this enables you to use normal QImage and QPixmap functions to read and
    write images. For example:

    \code
        myLabel->setPixmap(QPixmap("myimage.jp2"));

        QPixmap myPixmap;
        myPixmap.save("myimage.jp2", "JP2");
    \endcode
*/

/*!
    Constructs an instance of QtJP2ImageHandler initialized to use \a device.
*/
QtJP2ImageHandler::QtJP2ImageHandler(QIODevice *device)
:writeQuality(100)
,subType("jp2")
{
    setDevice(device);
}

/*!
    Desctructor for QtJP2ImageHandler.
*/
QtJP2ImageHandler::~QtJP2ImageHandler()
{

}

/*!
 Verifies if some values (magic bytes) are set as expected in the
 header of the file. If the magic bytes were found, we assume that we
 can read the file. The function will assume that the \a iod is
 pointing to the beginning of the JPEG 2000 header. (i.e. it will for
 instance not seek to the beginning of a file before reading).

 If \a subType is not 0, it will contain "jp2" or "j2k" upon
 successful return.
*/

bool QtJP2ImageHandler::canRead(QIODevice *iod, QByteArray *subType)
{
    bool bCanRead = false;
    if (iod) {
        qint64 oldPos = iod->pos();

        unsigned char header[12];
        if (iod->read((char*)header,sizeof(header)) == sizeof(header)) {
            char matchheader1[] = "\000\000\000\fjP  \r\n\207\n";  //Jp2 is the JPEG 2000 file format.
            if (memcmp(header, matchheader1, sizeof(matchheader1) - 1) == 0) {
                bCanRead = true;
                if (subType)
                    *subType = "jp2";
            } else {
                char matchheader2[] = "\377\117\377\121\000";  //J2c is the JPEG 2000 code stream.
                if (memcmp(header, matchheader2, sizeof(matchheader2) - 1) == 0) {
                    bCanRead = true;
                    if (subType)
                        *subType = "j2k";
                }
            }
        }
        if (iod->isSequential()) {
            int i = sizeof(header) - 1;
            while (i >= 0) {
                char c = header[i--];
                iod->ungetChar(c);
            }
        } else {
            iod->seek(oldPos);
        }
    }
    return bCanRead;
}

/*! \reimp

*/
bool QtJP2ImageHandler::canRead() const
{
    QByteArray subType;
    if (canRead(device(), &subType)) {
        setFormat(subType);
        return true;
    }
    return false;
}

/*! \reimp

*/
bool QtJP2ImageHandler::read(QImage *image)
{
    Jpeg2000JasperReader reader(device());
    return reader.read(image);
}

/*! \reimp

*/
bool QtJP2ImageHandler::write(const QImage &image)
{
    SubFormat subFormat;
    if (this->subType == "jp2")
        subFormat = Jp2Format;
    else
        subFormat = J2kFormat;

    Jpeg2000JasperReader writer(device(), subFormat);
    return writer.write(image, this->writeQuality);
}

/*!
    Get the value associated  with \a option.
    \sa setOption()
*/
QVariant QtJP2ImageHandler::option(ImageOption option) const
{
    if (option == Quality) {
        return QVariant(this->writeQuality);
    } else if (option == SubType) {
        return QVariant(this->subType);
    } else {
        return QVariant();
    }
}

/*!
    The JPEG 2000 handler supports two options.
    Set \a option to QImageIOHandler::Quality to balance between quality and the compression level,
    where \a value should be an integer in the interval [0-100]. 0 is maximum compression (low quality) and 100 is lossless compression (high quality).

    Set \a option to QImageIOHandler::Subtype to choose the subtype,
    where \a value should be a string equal to either "jp2" or "j2k"
    \sa option()
*/
void QtJP2ImageHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == Quality) {
        bool ok;
        const int quality = value.toInt(&ok);
        if (ok)
            this->writeQuality = quality;
    } else if (option == SubType) {
        const QByteArray subTypeCandidate = value.toByteArray();
        // Test for Jpeg200 file format (jp2) - the default, or stream format (j2k).
        if (subTypeCandidate == "jp2" || subTypeCandidate == "j2k")
            this->subType = subTypeCandidate;
    }
}

/*!
      This function will return true if \a option is set to either QImageIOHandler::Quality or QImageIOHandler::Subtype.

*/
bool QtJP2ImageHandler::supportsOption(ImageOption option) const
{
    return (option == Quality || option == SubType);
}

/*!
 * Return the common identifier of the format.
 * For JPEG 2000 this will return "jp2".
 */
QByteArray QtJP2ImageHandler::name() const
{
    return "jp2";
}


/*
    automatic resource handling for a jas_image_t *
*/
class ScopedJasperImage {
public:
    //take reference to the pointer here, because the pointer
    // may change when we change color spaces.
    ScopedJasperImage(jas_image_t *&image):image(image) { }
    ~ScopedJasperImage() {   jas_image_destroy(image);  }
private:
    jas_image_t *&image;
};



/*! \internal
    Construct a Jpeg2000JasperReader using the provided \a imageIO.
    Note that currenty the jasper library is initialized in this constructor, (and
    freed in the destructor) whic means that:
    -Only one instance of this class may exist at one time
    -no thread safety
*/
Jpeg2000JasperReader::Jpeg2000JasperReader(QIODevice *iod, SubFormat format)
:jasperOk( true )
,ioDevice( iod )
,format( format )
,hasAlpha( false )
{
    if (jas_init()) {
        jasperOk=false;
        qDebug("Jasper Library initialization failed");
    }
}

Jpeg2000JasperReader::~Jpeg2000JasperReader()
{
    if(jasperOk)
        jas_cleanup();
}

/*! \internal
        Opens the file data and atemts to decode it using the Jasper library.
        returns true if succesfu, false on failure
*/
bool Jpeg2000JasperReader::read(QImage *pImage)
{
    /*
        reading proceeds approx. as follows:
        1.open stream and decode using jasper
        2.get image metadata
        3.change colorspace if neccesary
        4.create a QImage of the appropriate type (32-bit for RGB, 8-bit for grayscale)
        5.copy image data from jasper to the qImage

        When copying the image data from the jasper data structures to the QImage,
        a generic copy function (copyJasperQt) iterates through the scanlines and calls
        the provided (via the scanlineCopier argument) scanline copy function for each
        scanline. The scanline copy funciton selected according to image metadata sunch
        as color space and the precence of an apha channel.
    */
    if(!jasperOk)
        return false;
    QByteArray fileContents = ioDevice->readAll();
    jas_stream_t *imageData = jas_stream_memopen(fileContents.data(), fileContents.size());
    jasper_image = jas_image_decode(imageData, jas_image_getfmt(imageData), 0);
    jas_stream_close(imageData);
    if (!jasper_image) {
        qDebug("Jasper Library can't decode Jpeg2000 image data");
        return false;
    }
    ScopedJasperImage scopedImage(jasper_image);
    //printMetatdata(jasper_image);

    qtWidth = jas_image_width(jasper_image);
    qtHeight =jas_image_height(jasper_image);
    jasNumComponents =jas_image_numcmpts(jasper_image);
    jasperColorspaceFamily = jas_clrspc_fam(jas_image_clrspc(jasper_image));

    bool needColorspaceChange = false;
    if ( jasperColorspaceFamily != JAS_CLRSPC_FAM_RGB && jasperColorspaceFamily != JAS_CLRSPC_FAM_GRAY )
        needColorspaceChange=true;

    //get per-componet data
    int c;
    for (c = 0; c < jasNumComponents; ++c) {
        jasComponentPrecicion[c]=jas_image_cmptprec(jasper_image, c);
        //test for precicion
        if(jasComponentPrecicion[c]>8 || jasComponentPrecicion[c]<8)
            needColorspaceChange=true;
        //test for subsampling
        if ( jas_image_cmpthstep(jasper_image, c) != 1 || jas_image_cmptvstep(jasper_image, c) !=1)
            needColorspaceChange=true;
        //test for signed components
        if ( jas_image_cmptsgnd(jasper_image, c) !=0 )
            needColorspaceChange=true;
    }
    /*
        If we encounter a differnt color space than RGB
        (such as XYZ or YCbCr) we change that to RGB.
        Also, if any component has "funny" metatdata( such as precicion !=8 bits
        or subsampling  !=1) we also do a colorspace
        change in order to convert it to something we can load.
    */

    bool decodeOk=true;
    if (needColorspaceChange )
        decodeOk=attemptColorspaceChange(JAS_CLRSPC_SRGB);

    if (!decodeOk) {
        printColorSpaceError();
        return false;
    }

    /*
        Image metadata may have changed, get from jasper.
    */
    qtWidth = jas_image_width(jasper_image);
    qtHeight =jas_image_height(jasper_image);
    jasNumComponents =jas_image_numcmpts(jasper_image);
    jasperColorspaceFamily = jas_clrspc_fam(jas_image_clrspc(jasper_image));
    for (c = 0; c < jasNumComponents; ++c){
        jasComponentPrecicion[c]=jas_image_cmptprec(jasper_image, c);
    }

    if(jasperColorspaceFamily!= JAS_CLRSPC_FAM_RGB && jasperColorspaceFamily!=JAS_CLRSPC_FAM_GRAY) {
        qDebug("The Qt JPEG 2000 reader was unable to convert colorspace to RGB or grayscale");
        return false;
    }
    /*
        if a component has a subsampling factor!=1, we cant trust jas_image_height/width, so
        we need to figure it out ourselves
    */
    bool oddComponentSubsampling=false;
    for (c = 0; c < jasNumComponents; ++c){
        if (jas_image_cmpthstep(jasper_image, c)!=1 || jas_image_cmptvstep(jasper_image, c) !=1) {
            oddComponentSubsampling=true;
        }
    }

    if (oddComponentSubsampling) {
        //Check if all components have the same vertical/horizontal dim and subsampling
        computedComponentWidth = jas_image_cmptwidth (jasper_image, 0);
        computedComponentHeight= jas_image_cmptheight (jasper_image, 0);
        computedComponentHorizontalSubsampling=jas_image_cmpthstep(jasper_image, 0);
        computedComponentVerticalSubsampling=jas_image_cmptvstep(jasper_image, 0);

        for (c = 1; c < jasNumComponents; ++c) {
            if(computedComponentWidth!=jas_image_cmptwidth (jasper_image, c) ||
               computedComponentWidth!=jas_image_cmptwidth (jasper_image, c) ||
               computedComponentHorizontalSubsampling!=jas_image_cmpthstep(jasper_image, c) ||
               computedComponentVerticalSubsampling!=jas_image_cmptvstep(jasper_image, c)     ) {
                qDebug("The Qt JPEG 2000 reader does not support images where component geometry differ from image geometry");
                return false;
            }
        }
        qtWidth= computedComponentWidth *  computedComponentHorizontalSubsampling;
        qtHeight=computedComponentHeight * computedComponentVerticalSubsampling;
    }

    /*
        Sanity check each component
    */
    for (c = 0; c < jasNumComponents; ++c) {
        //test for precicion

        if(jasComponentPrecicion[c]>8 || jasComponentPrecicion[c]<8) {
            qDebug("The Qt JPEG 2000 reader does not support components whith precicion!=8");
            decodeOk=false;
        }
#if 0
        //test the subsampling factor, ie space between pixels on the image grid
        if (oddComponentSubsampling) {
            qDebug("The Qt JPEG 2000 reader does not support components whith a subsampling factor!=1 (yet)");
            decodeOk=false;
        }
#endif
        //test for signed components
        if ( jas_image_cmptsgnd(jasper_image, c) !=0 ) {
            qDebug("Qt JPEG 2000 reader does not support signed components ");
            decodeOk=false;
        }
        /*
            test for component/image geomoetry mismach.
            if oddComponentSubsampling, then this is already taken care of above.
        */
        if(!oddComponentSubsampling)
        if ( jas_image_cmpttlx(jasper_image,c) != 0 ||
             jas_image_cmpttly(jasper_image,c) != 0 ||
             jas_image_cmptbrx(jasper_image,c) != jas_image_brx(jasper_image) ||
             jas_image_cmptbry(jasper_image,c) != jas_image_bry(jasper_image) ||
             jas_image_cmptwidth (jasper_image, c) != jas_image_width (jasper_image) ||
             jas_image_cmptheight(jasper_image, c) != jas_image_height(jasper_image )) {
            qDebug("The Qt JPEG 2000 reader does not support images where component geometry differ from image geometry");
            printMetatdata(jasper_image);
            decodeOk=false;
        }
    }
    if (!decodeOk)
        return false;

    /*
        at this point, the colorspace shoud be either RGB or grayscale,
        and each component should have eigth bits of precicion and
        no unnsuported geometry.
    */
  //  printMetatdata(jasper_image);

    // Get color Components
    jasperColorspaceFamily = jas_clrspc_fam(jas_image_clrspc(jasper_image));
    if(jasperColorspaceFamily == JAS_CLRSPC_FAM_RGB) {
         if(jasNumComponents > 4)
             qDebug("JPEG 2000 reader expected 3 or 4 components, got %d", jasNumComponents);
         //set up mapping from R,G,B -> compnent num.
         colorComponentMapping[0] = jas_image_getcmptbytype(jasper_image, JAS_IMAGE_CT_RGB_R);
         colorComponentMapping[1] = jas_image_getcmptbytype(jasper_image, JAS_IMAGE_CT_RGB_G);
         colorComponentMapping[2] = jas_image_getcmptbytype(jasper_image, JAS_IMAGE_CT_RGB_B);
         qtNumComponents = 3;
    } else if (jasperColorspaceFamily == JAS_CLRSPC_FAM_GRAY) {
        if(jasNumComponents>2)
        qDebug("JPEG 2000 reader epected 1 or 2 components, got %d", jasNumComponents);
            colorComponentMapping[0] = jas_image_getcmptbytype(jasper_image, JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_GRAY_Y));
            qtNumComponents = 1;
    } else {
        printColorSpaceError();
        return false;
    }
/*
    Get Alpha component if one exists. Due to the lack of test images,
    the loading images with alpha channels is a bit untested. It works
    with images saved with this implementation though.
*/
    const int posibleAlphaComponent1 = 3;
    const int posibleAlphaComponent2 = 48;

    if (jasNumComponents==qtNumComponents + 1) {
        colorComponentMapping[qtNumComponents] = jas_image_getcmptbytype(jasper_image, posibleAlphaComponent1);
        if (colorComponentMapping[qtNumComponents] < 0) {
            colorComponentMapping[qtNumComponents] = jas_image_getcmptbytype(jasper_image, posibleAlphaComponent2);
        }
        if (colorComponentMapping[qtNumComponents] > 0){
            hasAlpha=true;
            qtNumComponents++;
        }
     }

     //check for missing components
     for(c = 0; c < qtNumComponents; ++c) {
         if (colorComponentMapping[c] < 0) {
             qDebug("JPEG 2000 reader missing a color component");
             return false;
         }
     }

    //create a Qiamge of the correct type
    if(jasperColorspaceFamily == JAS_CLRSPC_FAM_RGB)
        qtImage = QImage(qtWidth, qtHeight, hasAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    else if (jasperColorspaceFamily == JAS_CLRSPC_FAM_GRAY)
        if (hasAlpha) {
            qtImage = QImage(qtWidth, qtHeight, QImage::Format_ARGB32);
        }else {
            qtImage = QImage(qtWidth, qtHeight, QImage::Format_Indexed8);
            qtImage.setNumColors(256);
            for (int c = 0; c < 256; ++c)
                qtImage.setColor(c, qRgb(c,c,c));
        }

    //copy data
    if(oddComponentSubsampling) {
    /*
        This is a hack really, copying of data with component subsampling!=1
        doesn't fit in with the rest of the scanline copying framework.
    */
    copyJasperQtGeneric();

    }else if(jasperColorspaceFamily == JAS_CLRSPC_FAM_RGB) {
        if(hasAlpha)
            copyJasperQt(&Jpeg2000JasperReader::copyScanlineJasperQtRGBA);
        else
            copyJasperQt(&Jpeg2000JasperReader::copyScanlineJasperQtRGB);
    } else if (jasperColorspaceFamily == JAS_CLRSPC_FAM_GRAY){
        if(hasAlpha)
            copyJasperQt(&Jpeg2000JasperReader::copyScanlineJasperQtGrayA);
        else
            copyJasperQt(&Jpeg2000JasperReader::copyScanlineJasperQtGray);
    }
    if (decodeOk)
        *pImage = qtImage;

    return decodeOk;
}

/*!
    \internal
*/
void Jpeg2000JasperReader::copyJasperQtGeneric()
{
    //create scanline data poinetrs
    jas_matrix_t **jasperMatrix;
    jas_seqent_t **jasperRow;
    createJasperMatrix(jasperMatrix);
    jasperRow=(jas_seqent_t**)malloc(jasNumComponents*sizeof(jas_seqent_t *));

    int imageY=0;
    for(int componentY=0; componentY < computedComponentHeight; ++componentY){
        for (int c = 0; c < jasNumComponents; ++c) {
                jas_image_readcmpt(jasper_image, colorComponentMapping[c], 0, componentY, computedComponentWidth, 1, jasperMatrix[c]);
                jasperRow[c] = jas_matrix_getref(jasperMatrix[c], 0, 0);
        }
        for (int verticalSubsample=0; verticalSubsample<computedComponentVerticalSubsampling; ++verticalSubsample)  {
            uchar *scanLineUchar = qtImage.scanLine(imageY);
            QRgb  *scanLineQRgb = reinterpret_cast<QRgb *>(scanLineUchar);
            for(int componentX=0; componentX < computedComponentWidth; ++componentX){
                for (int horizontalSubsample=0; horizontalSubsample<computedComponentHorizontalSubsampling; ++horizontalSubsample)  {
                    if(jasperColorspaceFamily == JAS_CLRSPC_FAM_RGB) {
                        if (hasAlpha) {
                            *scanLineQRgb++ = jasperRow[3][componentX] << 24 |
                                              jasperRow[0][componentX] << 16 |
                                              jasperRow[1][componentX] << 8 |
                                              jasperRow[2][componentX];
                        }else {
                            *scanLineQRgb++ = jasperRow[0][componentX] << 16 |
                                              jasperRow[1][componentX] << 8 |
                                              jasperRow[2][componentX];
                        }
                    }else if (jasperColorspaceFamily == JAS_CLRSPC_FAM_GRAY) {
                        if (hasAlpha) {
                            *scanLineQRgb++ = jasperRow[1][componentX] << 24 |
                                              jasperRow[0][componentX] << 16 |
                                              jasperRow[0][componentX] << 8 |
                                              jasperRow[0][componentX];
                        }else {
                           *scanLineUchar++ = (jasperRow[0][componentX]);
                        }
                    }
                }
            }
            ++imageY;
        }
    }
}

/*!
    \internal
    Copies data from jasper to QImage. The scanlineCopier parameter spcifies which
    function to use for handling each scan line.
*/
void Jpeg2000JasperReader::copyJasperQt(const ScanlineFunc  scanlineCopier)
{
    //create scanline data poinetrs
    jas_matrix_t **jasperMatrix;
    jas_seqent_t **jasperRow;

    createJasperMatrix(jasperMatrix);
    jasperRow=(jas_seqent_t**)malloc(jasNumComponents*sizeof(jas_seqent_t *));

    for(int scanline = 0; scanline < qtHeight; ++scanline){
        for (int c = 0; c < jasNumComponents; ++c) {
            jas_image_readcmpt(jasper_image, colorComponentMapping[c], 0, scanline, qtWidth, 1, jasperMatrix[c]);
            jasperRow[c] = jas_matrix_getref(jasperMatrix[c], 0, 0);
        }
       (this->*scanlineCopier)(jasperRow, qtImage.scanLine(scanline));
    }

    freeJasperMatrix(jasperMatrix);
    free(jasperRow);
}

/*!
    \internal
    copyScanlineJasperQtRGB is copyies RGB data from jasper to an 32 bit QImage.
*/
void Jpeg2000JasperReader::copyScanlineJasperQtRGB(jas_seqent_t ** const jasperRow,  uchar *qtScanLine)
{
    QRgb *scanLine = reinterpret_cast<QRgb *>(qtScanLine);
    for (int c = 0; c< qtWidth; ++c) {
         *scanLine++ = 0xFF << 24 | jasperRow[0][c] << 16 | jasperRow[1][c] << 8 | jasperRow[2][c];
    }
}

/*!
    \internal
    copyScanlineJasperQtRGBA is copyies RGBA data from jasper to an 32 bit QImage.
*/
void Jpeg2000JasperReader::copyScanlineJasperQtRGBA(jas_seqent_t ** const jasperRow,  uchar *qtScanLine)
{
    QRgb *scanLine = reinterpret_cast<QRgb *>(qtScanLine);
    for (int c = 0; c< qtWidth; ++c) {
         *scanLine++ = jasperRow[3][c] << 24 | jasperRow[0][c] << 16 | jasperRow[1][c] << 8 | jasperRow[2][c];
    }
}
/*!
    \internal
    copyScanlineJasperQtGray copies data from a grayscale image to a 8 bit QImage.
*/
void Jpeg2000JasperReader::copyScanlineJasperQtGray(jas_seqent_t ** const jasperRow,  uchar *qtScanLine)
{
    for (int c = 0; c < qtWidth; ++c) {
        // *qtScanLine++ = (jasperRow[0][c] >> (jasComponentPrecicion[0] - 8));
         *qtScanLine++ = (jasperRow[0][c]);
    }
}
/*!
    \internal
    copyScanlineJasperQtGrayA copies data from a grayscale image to a 32 bit QImage.
    note that in this case we use an 32-bit image for grayscale data, since the the
    alpha walue is per-pixel, not per-color (per-color alpha is supported by 8-bit QImage)
*/
void Jpeg2000JasperReader::copyScanlineJasperQtGrayA(jas_seqent_t ** const jasperRow,  uchar *qtScanLine)
{
    QRgb *scanLine = reinterpret_cast<QRgb *>(qtScanLine);
    for (int c = 0; c < qtWidth; ++c) {
         *scanLine++ = jasperRow[1][c] << 24 | jasperRow[0][c] << 16 | jasperRow[0][c] << 8 | jasperRow[0][c];
    }
}

/////////////// Write

/*!
        Opens the file data and atemts to decode it using the Jasper library.
        returns true on success, false on failure.

        32 Bit and colormapped color images are encoded as RGB images,
        colormapped grayscale images are encoded as grayscale images
*/
bool Jpeg2000JasperReader::write(const QImage &image, int quality)
{
    if(!jasperOk)
        return false;

    qtImage = image;


    qtHeight=qtImage.height();
    qtWidth=qtImage.width();
    qtDepth=qtImage.depth();

    if(qtDepth == 32) // RGB(A)
    {
        jasper_image = newRGBAImage(qtWidth, qtHeight, qtImage.hasAlphaChannel());

		// >DIR: check if the image could be created [22.7.2014 markus]
		if (!jasper_image)
			return false;

        if (qtImage.hasAlphaChannel())
            copyQtJasper(&Jpeg2000JasperReader::copyScanlineQtJasperRGBA);
        else
            copyQtJasper(&Jpeg2000JasperReader::copyScanlineQtJasperRGB);
    }
    else if (qtDepth == 8)
    {
        if (qtImage.allGray()) //colormapped grayscale
        {
            jasper_image = newGrayscaleImage(qtWidth, qtHeight, qtImage.hasAlphaChannel());

			// >DIR: check if the image could be created [22.7.2014 markus]
			if (!jasper_image)
				return false;

            if (qtImage.hasAlphaChannel())
                copyQtJasper(&Jpeg2000JasperReader::copyScanlineQtJasperColormapGrayscaleA);
            else
                copyQtJasper(&Jpeg2000JasperReader::copyScanlineQtJasperColormapGrayscale);
        }
        else // colormapped color
        {
            jasper_image = newRGBAImage(qtWidth, qtHeight, qtImage.hasAlphaChannel());

			// >DIR: check if the image could be created [22.7.2014 markus]
			if (!jasper_image)
				return false;

            if (qtImage.hasAlphaChannel())
               copyQtJasper(&Jpeg2000JasperReader::copyScanlineQtJasperColormapRGBA);
            else
               copyQtJasper(&Jpeg2000JasperReader::copyScanlineQtJasperColormapRGB);
        }
    }
    else
    {
        qDebug("Unable to handle color depth %d", qtDepth);
        return false;
    }

    int fmtid;
    if (format == Jp2Format)
        fmtid=jas_image_strtofmt(const_cast<char*>("jp2"));
    else /*format=J2cFormat*/
        fmtid=jas_image_strtofmt(const_cast<char*>("jpc"));    // JasPer refers to the code stream format as jpc

    const int minQuality = 0;
    const int maxQuality = 100;

    if (quality == -1)
        quality = 100;
    if (quality <= minQuality)
        quality = minQuality;
    if (quality > maxQuality)
        quality = maxQuality;

    // Qt specifies quality as an integer in the range 0..100. Jasper specifies
    // compression rate as an real in the range 0..1, where 1 corresponds to no compression.
    // Computing the rate from quality is difficult, large images get better image
    // quality than small images at the same rate. If the rate is to low, Jasper
    // will generate a completely black image.
    // minirate is the smallest safe rate value.
    const double minRate = 0.001;
    // maxRate specifies maximum target rate, which give the minimum amount
    // of compression. Tests show shat maxRates higer than 0.3 give no additional
    // image quality for most images. Large images could use an even smaller maxRate value.
    const double maxRate = 0.3;
    // Set jasperRate to a value in the range minRate..maxRate. Distribute the quality
    // steps more densly at the lower end if the rate scale.
    const double jasperRate = minRate + pow((double(quality) / double(maxQuality)), 2) * maxRate;

    // The Jasper format string contains two optins:
    // rate: rate=x
    // lossy/lossless compression : mode=real/mode=int
    QString jasperFormatString("");

    // If quality is not maxQuality, we set lossy encoding. (lossless is default)
    if (quality != maxQuality) {
        jasperFormatString += "mode=real";
        jasperFormatString += QString().sprintf(" rate=%f", jasperRate);
    }

    // Open an empty jasper stream that grows automatically
    jas_stream_t * memory_stream = jas_stream_memopen(0, -1);
    // Jasper wants a non-const string.
    char *str = qstrdup(jasperFormatString.toLatin1().constData());
    jas_image_encode(jasper_image, memory_stream,fmtid, str);
    delete [] str;
    jas_stream_flush(memory_stream);

    // jas_stream_t::obj_ is a void* which points to the stream implementation,
    // e.g a file stream or a memory stream. But in our case we know that it is
    // a memory stream since we created the object, so we just reiterpret_cast here..
    char *buffer = reinterpret_cast<char *>(reinterpret_cast<jas_stream_memobj_t*>(memory_stream->obj_)->buf_);
    qint64 length = jas_stream_length(memory_stream);
    ioDevice->write( buffer, length );

    jas_stream_close( memory_stream );
    jas_image_destroy( jasper_image );

    return true;
}

/*!
    \internal
    Copies data from qtImage to jasper. The scanlineCopier parameter spcifies which
    function to use for handling each scan line.

*/
void Jpeg2000JasperReader::copyQtJasper(const ScanlineFuncWrite scanlinecopier)
{   //create jasper matrix for holding one scanline
    jas_matrix_t **jasperMatrix;
    createJasperMatrix(jasperMatrix);

    for(int scanline = 0; scanline<qtHeight; ++scanline){
        (this->*scanlinecopier)(jasperMatrix, qtImage.scanLine(scanline));
        //write a scanline of data to jasper_image
        for(int c = 0; c < jasNumComponents; ++c)
            jas_image_writecmpt(jasper_image, c, 0, scanline ,qtWidth, 1,  jasperMatrix[c]);
    }
    freeJasperMatrix(jasperMatrix);
}

/*!
    \internal

*/
void Jpeg2000JasperReader::copyScanlineQtJasperRGB(jas_matrix_t ** jasperRow,  uchar * qtScanLine)
{
    QRgb *scanLineBuffer = reinterpret_cast<QRgb *>(qtScanLine);
    for( int col=0; col<qtWidth; ++col) {
        jas_matrix_set(jasperRow[0], 0, col, (*scanLineBuffer & 0xFF0000)>>16);
        jas_matrix_set(jasperRow[1], 0, col, (*scanLineBuffer & 0x00FF00)>>8);
        jas_matrix_set(jasperRow[2], 0, col,  *scanLineBuffer & 0x0000FF);
        ++scanLineBuffer;
    }
}

/*!
    \internal

*/
void Jpeg2000JasperReader::copyScanlineQtJasperRGBA(jas_matrix_t ** jasperRow,  uchar * qtScanLine)
{
    QRgb *scanLineBuffer = reinterpret_cast<QRgb *>(qtScanLine);
    for( int col = 0; col<qtWidth; ++col) {
        jas_matrix_set(jasperRow[3], 0, col, (*scanLineBuffer & 0xFF000000)>>24);
        jas_matrix_set(jasperRow[0], 0, col, (*scanLineBuffer & 0x00FF0000)>>16);
        jas_matrix_set(jasperRow[1], 0, col, (*scanLineBuffer & 0x0000FF00)>>8);
        jas_matrix_set(jasperRow[2], 0, col,  *scanLineBuffer & 0x000000FF);
        ++scanLineBuffer;
    }
}

/*!
    \internal

*/
void Jpeg2000JasperReader::copyScanlineQtJasperColormapRGB(jas_matrix_t ** jasperRow,  uchar * qtScanLine)
{
    for( int col = 0; col<qtWidth; ++col) {
        QRgb color =qtImage.color(*qtScanLine);
        jas_matrix_set(jasperRow[0], 0, col, qRed  (color));
        jas_matrix_set(jasperRow[1], 0, col, qGreen(color));
        jas_matrix_set(jasperRow[2], 0, col, qBlue (color));
        ++qtScanLine;
     }
}

/*!
    \internal

*/
void Jpeg2000JasperReader::copyScanlineQtJasperColormapRGBA(jas_matrix_t ** jasperRow,  uchar * qtScanLine)
{
   for( int col = 0; col<qtWidth; ++col) {
        QRgb color =qtImage.color(*qtScanLine);
        jas_matrix_set(jasperRow[0], 0, col, qRed  (color));
        jas_matrix_set(jasperRow[1], 0, col, qGreen(color));
        jas_matrix_set(jasperRow[2], 0, col, qBlue (color));
        jas_matrix_set(jasperRow[3], 0, col, qAlpha(color));
        ++qtScanLine;
     }
}

/*!
    \internal

*/
void Jpeg2000JasperReader::copyScanlineQtJasperColormapGrayscale(jas_matrix_t ** jasperRow,  uchar * qtScanLine)
{
    for( int col = 0; col<qtWidth; ++col) {
        QRgb color =qtImage.color(*qtScanLine);
        jas_matrix_set(jasperRow[0], 0, col, qGray(color));
        ++qtScanLine;
     }
}

/*!
    \internal

*/
void Jpeg2000JasperReader::copyScanlineQtJasperColormapGrayscaleA(jas_matrix_t ** jasperRow,  uchar * qtScanLine)
{
    for( int col = 0; col<qtWidth; ++col) {
        QRgb color =qtImage.color(*qtScanLine);
        jas_matrix_set(jasperRow[0], 0, col, qGray(color));
        jas_matrix_set(jasperRow[1], 0, col, qAlpha(color));
        ++qtScanLine;
     }
}

////////////// Misc

/*!
    \internal
    Attempts to change the color space for the image to wantedColorSpace using the
   jasper library
*/
bool Jpeg2000JasperReader::attemptColorspaceChange(int wantedColorSpace)
{
//  qDebug("Attemting color space change");
    jas_cmprof_t *outprof;
    if (!(outprof = jas_cmprof_createfromclrspc(wantedColorSpace)))
        return false;

    jas_image_t *newimage;
    if (!(newimage = jas_image_chclrspc(jasper_image, outprof, JAS_CMXFORM_INTENT_PER))) {
        jas_cmprof_destroy(outprof);
        return false;
    }
    jas_image_destroy(jasper_image);
    jas_cmprof_destroy(outprof);
    jasper_image = newimage;
    return true;
}

/*!
    \internal
    Set up a component with parameters suitable for storing a QImage
*/
jas_image_cmptparm_t Jpeg2000JasperReader::createComponentMetadata(const int width, const int height)
{
    jas_image_cmptparm_t param;
    param.tlx = 0;
    param.tly = 0;
    param.hstep = 1;
    param.vstep = 1;
    param.width = width;
    param.height = height;
    param.prec = 8;
    param.sgnd = 0;
    return param;
}

/*!
    \internal
    Create a new RGB jasper image with a possible alpha channel
*/
jas_image_t* Jpeg2000JasperReader::newRGBAImage(const int width, const int height, bool alpha)
{
    jasNumComponents = alpha ? 4 : 3;
    jas_image_cmptparm_t *params = new jas_image_cmptparm_t[jasNumComponents];
    jas_image_cmptparm_t param = createComponentMetadata(width, height);
    for (int c=0; c<jasNumComponents; c++)
        params[c] = param;
    jas_image_t *newImage = jas_image_create(jasNumComponents, params,JAS_CLRSPC_SRGB);

	// >DIR: check null pointer [22.7.2014 markus]
	if (!newImage)
		return 0;

    jas_image_setcmpttype(newImage, 0,JAS_IMAGE_CT_RGB_R);
    jas_image_setcmpttype(newImage, 1,JAS_IMAGE_CT_RGB_G);
    jas_image_setcmpttype(newImage, 2,JAS_IMAGE_CT_RGB_B);
    /*
        It is unclear how one stores opacity(alpha) components with jasper,
        the following seems to have no effect. (The opacity component gets
        type id 3 or 48 depending jp2 or j2c format no matter what one puts in here.)

        The symbols are defined as follows:
        #define JAS_IMAGE_CT_RGB_R 0
        #define JAS_IMAGE_CT_RGB_G 1
        #define JAS_IMAGE_CT_RGB_B 2
        #define JAS_IMAGE_CT_OPACITY 0x7FFF
    */
    if(alpha)
        jas_image_setcmpttype(newImage, 3, JAS_IMAGE_CT_OPACITY);
    delete[] params;
	return newImage;
}

/*!
    \internal
    Create a new RGB jasper image with a possible alpha channel

*/
jas_image_t *Jpeg2000JasperReader::newGrayscaleImage(const int width, const int height, bool alpha)
{
    jasNumComponents = alpha ? 2 : 1;
    jas_image_cmptparm_t param = createComponentMetadata(width, height);
    jas_image_t *newImage = jas_image_create(1, &param,JAS_CLRSPC_SGRAY);

	// >DIR: check if the image could be created [22.7.2014 markus]
	if (!newImage)
		return 0;

    jas_image_setcmpttype(newImage, 0,JAS_IMAGE_CT_GRAY_Y);

    /*
        See corresponding comment for newRGBAImage.
    */
    if(alpha)
        jas_image_setcmpttype(newImage, 1, JAS_IMAGE_CT_OPACITY);
    return newImage;
}
/*!
    \internal
    Alocate data structures that holds image data during transfer from the
    Jasper data structures to QImage
*/
bool Jpeg2000JasperReader::createJasperMatrix(jas_matrix_t **&matrix)
{
    matrix = (jas_matrix_t**)malloc(jasNumComponents*sizeof(jas_matrix_t *));
    for (int c = 0; c<jasNumComponents; ++c) {
         matrix[c] = jas_matrix_create(1, qtWidth);
    }
    return true;
}

/*!
    \internal
    Free data structures that holds image data during transfer from the
    Jasper data structures to QImage
*/
bool Jpeg2000JasperReader::freeJasperMatrix(jas_matrix_t **matrix)
{
    for (int c = 0; c < jasNumComponents; ++c) {
        jas_matrix_destroy(matrix[c]);
    }
    free(matrix);
    return false;
}

/*!
    \internal

*/
void Jpeg2000JasperReader::printColorSpaceError()
{
    QString colorspaceFamily, colorspaceSpecific;
    decodeColorSpace(jas_image_clrspc(jasper_image), colorspaceFamily, colorspaceSpecific);
    qDebug("Jpeg2000 decoder is not able to handle color space %s - %s", qPrintable(colorspaceFamily), qPrintable(colorspaceSpecific) );
}
/*!
    \internal

*/
bool Jpeg2000JasperReader::decodeColorSpace(int clrspc, QString &family, QString &specific)
{
    int fam=jas_clrspc_fam(clrspc);
    int mbr=jas_clrspc_mbr(clrspc);

    switch(fam)
    {
        case 0:family= "JAS_CLRSPC_FAM_UNKNOWN"; break;
        case 1:family= "JAS_CLRSPC_FAM_XYZ"; break;
        case 2:family= "JAS_CLRSPC_FAM_LAB";break;
        case 3:family= "JAS_CLRSPC_FAM_GRAY";break;
        case 4:family= "JAS_CLRSPC_FAM_RGB";break;
        case 5:family= "JAS_CLRSPC_FAM_YCBCR";break;
        default:family= "Unknown"; return FALSE;
    }

    switch(mbr)
    {
        case 0:
            switch(fam)
            {
                case 1: specific= "JAS_CLRSPC_CIEXYZ"; break;
                case 2: specific= "JAS_CLRSPC_CIELAB";break;
                case 3: specific= "JAS_CLRSPC_SGRAY"; break;
                case 4: specific= "JAS_CLRSPC_SRGB"; break;
                case 5: specific= "JAS_CLRSPC_SYCBCR";break;
                default:specific= "Unknown"; return FALSE;
            }
            break;
        case 1:
            switch(fam)
            {
                case 3:specific= "JAS_CLRSPC_GENGRAY";break;
                case 4:specific= "JAS_CLRSPC_GENRGB";break;
                case 5:specific= "JAS_CLRSPC_GENYCBCR"; break;
                default: specific= "Unknown"; return FALSE;
            }
            break;
        default:
            return FALSE;
    }
    return true;
}
/*!
    \internal

*/
void Jpeg2000JasperReader::printMetatdata(jas_image_t *image)
{
  //  jas_image_cmptparm_t param
    qDebug("Image Width: %d",  jas_image_width(image));
    qDebug("Image Height: %d", jas_image_height(image));
    qDebug("Coords on reference grid: (%d,%d) (%d,%d)", jas_image_tlx(image), jas_image_tly(image),
                                                        jas_image_brx(image), jas_image_bry(image));
    qDebug("Num image components: %d",jas_image_numcmpts(image));

    QString colorspaceFamily;
    QString colorspaceSpecific;
    decodeColorSpace(jas_image_clrspc(image), colorspaceFamily, colorspaceSpecific);
    qDebug("Color model (space): %d, %s -  %s",jas_image_clrspc(image), qPrintable(colorspaceFamily), qPrintable(colorspaceSpecific));

    qDebug("Componet metadata:");

    for(int c=0; c<jas_image_numcmpts(image); ++c)
    {
        qDebug("Componet %d:", c);
        qDebug("    Component type: %d",     jas_image_cmpttype(image, c));
        qDebug("    Width: %d",     jas_image_cmptwidth(image, c));
        qDebug("    Height: %d",    jas_image_cmptheight(image, c));
        qDebug("    Signedness: %d",jas_image_cmptsgnd(image, c));
        qDebug("    Precision: %d", jas_image_cmptprec(image, c));
        qDebug("    Horizontal subsampling factor: %d",jas_image_cmpthstep(image, c));
        qDebug("    Vertical subsampling factor: %d",  jas_image_cmptvstep(image, c));
        qDebug("    Coords on reference grid: (%d,%d) (%d,%d)", jas_image_cmpttlx(image,c), jas_image_cmpttly(image,c),
                                                                jas_image_cmptbrx(image,c), jas_image_cmptbry(image,c));
    }

}

