/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hconnectionmanager_serviceconfiguration.h"
#include "hconnectionmanager_serviceconfiguration_p.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*******************************************************************************
 * HConnectionManagerServiceConfigurationPrivate
 *******************************************************************************/
HConnectionManagerServiceConfigurationPrivate::HConnectionManagerServiceConfigurationPrivate() :
    m_supportedProtocolInfos(getDefaultProtocolInfo())
{
}

HConnectionManagerServiceConfigurationPrivate::~HConnectionManagerServiceConfigurationPrivate()
{
}

void HConnectionManagerServiceConfigurationPrivate::addDefaultImageProtocolInfos(HProtocolInfos* container)
{
    // jpeg
    container->append(HProtocolInfo("http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN"));
    container->append(HProtocolInfo("http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM"));
    container->append(HProtocolInfo("http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED"));
    container->append(HProtocolInfo("http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG"));
    container->append(HProtocolInfo("http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM_ICO"));
    container->append(HProtocolInfo("http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG_ICO"));

    // png
    container->append(HProtocolInfo("http-get:*:image/png:DLNA.ORG_PN=PNG_TN"));
    container->append(HProtocolInfo("http-get:*:image/png:DLNA.ORG_PN=PNG_LRG"));
    container->append(HProtocolInfo("http-get:*:image/png:DLNA.ORG_PN=PNG_SM_ICO"));
    container->append(HProtocolInfo("http-get:*:image/png:DLNA.ORG_PN=PNG_LRG_ICO"));

    // GIF
    container->append(HProtocolInfo("http-get:*:image/gif:DLNA.ORG_PN=GIF_LRG"));
}

void HConnectionManagerServiceConfigurationPrivate::addDefaultAudioProtocolInfos(HProtocolInfos* container)
{
    // AC-3
    container->append(HProtocolInfo("http-get:*:audio/vnd.dolby.dd-raw:DLNA.ORG_PN=AC3"));

    // AMR
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_3gpp"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_WBPlus"));

    // ATRAC3plus
    container->append(HProtocolInfo("http-get:*:audio/x-sony-oma:DLNA.ORG_PN=ATRAC3plus"));

    // LPCM
    container->append(HProtocolInfo("http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM"));
//    container->append(HProtocolInfo("http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM_low"));
//    container->append(HProtocolInfo("http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM_MPS"));

    // mp3
    container->append(HProtocolInfo("http-get:*:audio/mpeg:DLNA.ORG_PN=MP3"));
//    container->append(HProtocolInfo("http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X"));

    // mp4
    container->append(HProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS"));
    container->append(HProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_192"));
    container->append(HProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_192"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_192"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_320"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_320"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_MULT5_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_MULT5_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_MULT7_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_MULT7_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_MULT5_ADTS"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MULT5_ISO"));
    container->append(HProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_MULT5_ISO"));
    // TODO the rest of MP4 profiles

    // WMA
    container->append(HProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE"));
    container->append(HProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL"));
    container->append(HProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO"));
    container->append(HProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL"));
    container->append(HProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL_MULT5"));
}

void HConnectionManagerServiceConfigurationPrivate::addDefaultVideoProtocolInfos(HProtocolInfos* container)
{
    // MPEG-1
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1"));

    // MPEG-2
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO"));

    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_50_AC3_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_60_AC3_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_ISO"));
    container->append(HProtocolInfo("http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_ISO"));

    // MPEG-4
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_S_HEAAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_ATRAC3plus"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC_LTP"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AMR"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_HEAAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC_MULT5"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_ATRAC3plus"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC"));
    container->append(HProtocolInfo("http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR"));
    container->append(HProtocolInfo("http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_H263_3GPP_P3_L10_AMR"));
    container->append(HProtocolInfo("http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P3_L10_AMR"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_NDSD"));

    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MULT5"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L2"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_MPEG1_L3"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AC3"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_520"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_940"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L31_HD_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L32_HD_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_1080i_AAC"));
    container->append(HProtocolInfo("http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_720p_AAC"));

    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA"));
    container->append(HProtocolInfo("http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_T"));

    // WMV9
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHM_BASE"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3"));
}

void HConnectionManagerServiceConfigurationPrivate::addGenericProtocolInfos(HProtocolInfos* container)
{
    container->append(HProtocolInfo("http-get:*:application/octet-stream:*"));
    container->append(HProtocolInfo("http-get:*:application/ogg:*"));

    container->append(HProtocolInfo("http-get:*:text/plain:*"));
    container->append(HProtocolInfo("http-get:*:text/xml:*"));

    container->append(HProtocolInfo("http-get:*:image/jpeg:*"));
    container->append(HProtocolInfo("http-get:*:image/png:*"));
    container->append(HProtocolInfo("http-get:*:image/gif:*"));
    container->append(HProtocolInfo("http-get:*:image/tiff:*"));

    container->append(HProtocolInfo("http-get:*:audio/basic:*"));
    container->append(HProtocolInfo("http-get:*:audio/L24:*"));
    container->append(HProtocolInfo("http-get:*:audio/mp4:*"));
    container->append(HProtocolInfo("http-get:*:audio/mpeg:*"));
    container->append(HProtocolInfo("http-get:*:audio/ogg:*"));
    container->append(HProtocolInfo("http-get:*:audio/vorbis:*"));
    container->append(HProtocolInfo("http-get:*:audio/vnd.rn-realaudio:*"));
    container->append(HProtocolInfo("http-get:*:audio/vnd.wave:*"));
    container->append(HProtocolInfo("http-get:*:audio/wav:*"));
    container->append(HProtocolInfo("http-get:*:audio/x-wav:*"));
    container->append(HProtocolInfo("http-get:*:audio/x-flac:*"));
    container->append(HProtocolInfo("http-get:*:application/ogg:*"));

    container->append(HProtocolInfo("http-get:*:video/mpeg:*"));
    container->append(HProtocolInfo("http-get:*:video/mp4:*"));
    container->append(HProtocolInfo("http-get:*:video/ogg:*"));
    container->append(HProtocolInfo("http-get:*:video/quicktime:*"));
    container->append(HProtocolInfo("http-get:*:video/x-ms-wmv:*"));
    container->append(HProtocolInfo("http-get:*:video/x-flv:*"));
    container->append(HProtocolInfo("http-get:*:video/avi:*"));
    container->append(HProtocolInfo("http-get:*:video/divx:*"));
    container->append(HProtocolInfo("http-get:*:video/x-msvideo:*"));
}

HProtocolInfos HConnectionManagerServiceConfigurationPrivate::getDefaultProtocolInfo()
{
    HProtocolInfos retVal;

    addDefaultImageProtocolInfos(&retVal);
    addDefaultAudioProtocolInfos(&retVal);
    addDefaultVideoProtocolInfos(&retVal);
    addGenericProtocolInfos(&retVal);

    return retVal;
}

/*******************************************************************************
 * HConnectionManagerServiceConfiguration
 *******************************************************************************/
HConnectionManagerServiceConfiguration::HConnectionManagerServiceConfiguration() :
    h_ptr(new HConnectionManagerServiceConfigurationPrivate())
{
}

HConnectionManagerServiceConfiguration::HConnectionManagerServiceConfiguration(
    HConnectionManagerServiceConfigurationPrivate& dd) :
        h_ptr(&dd)
{
}

HConnectionManagerServiceConfiguration::~HConnectionManagerServiceConfiguration()
{
    delete h_ptr;
}

void HConnectionManagerServiceConfiguration::doClone(HClonable* target) const
{
    HConnectionManagerServiceConfiguration* conf =
        dynamic_cast<HConnectionManagerServiceConfiguration*>(target);

    if (!conf)
    {
        return;
    }

    conf->h_ptr->m_supportedProtocolInfos = h_ptr->m_supportedProtocolInfos;
}

HConnectionManagerServiceConfiguration* HConnectionManagerServiceConfiguration::newInstance() const
{
    return new HConnectionManagerServiceConfiguration();
}

HConnectionManagerServiceConfiguration* HConnectionManagerServiceConfiguration::clone() const
{
    return static_cast<HConnectionManagerServiceConfiguration*>(HClonable::clone());
}

bool HConnectionManagerServiceConfiguration::isValid() const
{
    return !h_ptr->m_supportedProtocolInfos.isEmpty();
}

void HConnectionManagerServiceConfiguration::setSupportedContentInfo(const HProtocolInfos& infos)
{
    if (!infos.isEmpty())
    {
        h_ptr->m_supportedProtocolInfos = infos;
    }
}

const HProtocolInfos& HConnectionManagerServiceConfiguration::supportedContentInfo() const
{
    return h_ptr->m_supportedProtocolInfos;
}
}

}
}
