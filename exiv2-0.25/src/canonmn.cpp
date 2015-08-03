// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      canonmn.cpp
  Version:   $Rev: 3835 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             David Cannings (dc) <david@edeca.net>
             Andi Clemens (ac) <andi.clemens@gmx.net>
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id: canonmn.cpp 3835 2015-05-22 03:18:31Z nkbj $")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "canonmn_int.hpp"
#include "tags_int.hpp"
#include "value.hpp"
#include "exif.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <ios>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    //! OffOn, multiple tags
    extern const TagDetails canonOffOn[] = {
        {  0, N_("Off") },
        {  1, N_("On")  }
    };

    //! Special treatment pretty-print function for non-unique lens ids.
    std::ostream& printCsLensByFocalLengthAndMaxAperture(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata);
    std::ostream& printCsLensByFocalLength(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata);

    //! ModelId, tag 0x0010
    extern const TagDetails canonModelId[] = {
        { (long int)0x1010000, "PowerShot A30" },
        { (long int)0x1040000, "PowerShot S300 / Digital IXUS 300 / IXY Digital 300" },
        { (long int)0x1060000, "PowerShot A20" },
        { (long int)0x1080000, "PowerShot A10" },
        { (long int)0x1090000, "PowerShot S110 / Digital IXUS v / IXY Digital 200" },
        { (long int)0x1100000, "PowerShot G2" },
        { (long int)0x1110000, "PowerShot S40" },
        { (long int)0x1120000, "PowerShot S30" },
        { (long int)0x1130000, "PowerShot A40" },
        { (long int)0x1140000, "EOS D30" },
        { (long int)0x1150000, "PowerShot A100" },
        { (long int)0x1160000, "PowerShot S200 / Digital IXUS v2 / IXY Digital 200a" },
        { (long int)0x1170000, "PowerShot A200" },
        { (long int)0x1180000, "PowerShot S330 / Digital IXUS 330 / IXY Digital 300a" },
        { (long int)0x1190000, "PowerShot G3" },
        { (long int)0x1210000, "PowerShot S45" },
        { (long int)0x1230000, "PowerShot SD100 / Digital IXUS II / IXY Digital 30" },
        { (long int)0x1240000, "PowerShot S230 / Digital IXUS v3 / IXY Digital 320" },
        { (long int)0x1250000, "PowerShot A70" },
        { (long int)0x1260000, "PowerShot A60" },
        { (long int)0x1270000, "PowerShot S400 / Digital IXUS 400 / IXY Digital 400" },
        { (long int)0x1290000, "PowerShot G5" },
        { (long int)0x1300000, "PowerShot A300" },
        { (long int)0x1310000, "PowerShot S50" },
        { (long int)0x1340000, "PowerShot A80" },
        { (long int)0x1350000, "PowerShot SD10 / Digital IXUS i / IXY Digital L" },
        { (long int)0x1360000, "PowerShot S1 IS" },
        { (long int)0x1370000, "PowerShot Pro1" },
        { (long int)0x1380000, "PowerShot S70" },
        { (long int)0x1390000, "PowerShot S60" },
        { (long int)0x1400000, "PowerShot G6" },
        { (long int)0x1410000, "PowerShot S500 / Digital IXUS 500 / IXY Digital 500" },
        { (long int)0x1420000, "PowerShot A75" },
        { (long int)0x1440000, "PowerShot SD110 / Digital IXUS IIs / IXY Digital 30a" },
        { (long int)0x1450000, "PowerShot A400" },
        { (long int)0x1470000, "PowerShot A310" },
        { (long int)0x1490000, "PowerShot A85" },
        { (long int)0x1520000, "PowerShot S410 / Digital IXUS 430 / IXY Digital 450" },
        { (long int)0x1530000, "PowerShot A95" },
        { (long int)0x1540000, "PowerShot SD300 / Digital IXUS 40 / IXY Digital 50" },
        { (long int)0x1550000, "PowerShot SD200 / Digital IXUS 30 / IXY Digital 40" },
        { (long int)0x1560000, "PowerShot A520" },
        { (long int)0x1570000, "PowerShot A510" },
        { (long int)0x1590000, "PowerShot SD20 / Digital IXUS i5 / IXY Digital L2" },
        { (long int)0x1640000, "PowerShot S2 IS" },
        { (long int)0x1650000, "PowerShot SD430 / IXUS Wireless / IXY Wireless" },
        { (long int)0x1660000, "PowerShot SD500 / Digital IXUS 700 / IXY Digital 600" },
        { (long int)0x1668000, "EOS D60" },
        { (long int)0x1700000, "PowerShot SD30 / Digital IXUS i zoom / IXY Digital L3" },
        { (long int)0x1740000, "PowerShot A430" },
        { (long int)0x1750000, "PowerShot A410" },
        { (long int)0x1760000, "PowerShot S80" },
        { (long int)0x1780000, "PowerShot A620" },
        { (long int)0x1790000, "PowerShot A610" },
        { (long int)0x1800000, "PowerShot SD630 / Digital IXUS 65 / IXY Digital 80" },
        { (long int)0x1810000, "PowerShot SD450 / Digital IXUS 55 / IXY Digital 60" },
        { (long int)0x1820000, "PowerShot TX1" },
        { (long int)0x1870000, "PowerShot SD400 / Digital IXUS 50 / IXY Digital 55" },
        { (long int)0x1880000, "PowerShot A420" },
        { (long int)0x1890000, "PowerShot SD900 / Digital IXUS 900 Ti / IXY Digital 1000" },
        { (long int)0x1900000, "PowerShot SD550 / Digital IXUS 750 / IXY Digital 700" },
        { (long int)0x1920000, "PowerShot A700" },
        { (long int)0x1940000, "PowerShot SD700 IS / Digital IXUS 800 IS / IXY Digital 800 IS" },
        { (long int)0x1950000, "PowerShot S3 IS" },
        { (long int)0x1960000, "PowerShot A540" },
        { (long int)0x1970000, "PowerShot SD600 / Digital IXUS 60 / IXY Digital 70" },
        { (long int)0x1980000, "PowerShot G7" },
        { (long int)0x1990000, "PowerShot A530" },
        { (long int)0x2000000, "PowerShot SD800 IS / Digital IXUS 850 IS / IXY Digital 900 IS" },
        { (long int)0x2010000, "PowerShot SD40 / Digital IXUS i7 / IXY Digital L4" },
        { (long int)0x2020000, "PowerShot A710 IS" },
        { (long int)0x2030000, "PowerShot A640" },
        { (long int)0x2040000, "PowerShot A630" },
        { (long int)0x2090000, "PowerShot S5 IS" },
        { (long int)0x2100000, "PowerShot A460" },
        { (long int)0x2120000, "PowerShot SD850 IS / Digital IXUS 950 IS" },
        { (long int)0x2130000, "PowerShot A570 IS" },
        { (long int)0x2140000, "PowerShot A560" },
        { (long int)0x2150000, "PowerShot SD750 / Digital IXUS 75 / IXY Digital 90" },
        { (long int)0x2160000, "PowerShot SD1000 / Digital IXUS 70 / IXY Digital 10" },
        { (long int)0x2180000, "PowerShot A550" },
        { (long int)0x2190000, "PowerShot A450" },
        { (long int)0x2230000, "PowerShot G9" },
        { (long int)0x2240000, "PowerShot A650 IS" },
        { (long int)0x2260000, "PowerShot A720 IS" },
        { (long int)0x2290000, "PowerShot SX100 IS" },
        { (long int)0x2300000, "PowerShot SD950 IS / Digital IXUS 960 IS / IXY Digital 2000 IS" },
        { (long int)0x2310000, "PowerShot SD870 IS / Digital IXUS 860 IS / IXY Digital 910 IS" },
        { (long int)0x2320000, "PowerShot SD890 IS / Digital IXUS 970 IS / IXY Digital 820 IS" },
        { (long int)0x2360000, "PowerShot SD790 IS / Digital IXUS 90 IS / IXY Digital 95 IS" },
        { (long int)0x2370000, "PowerShot SD770 IS / Digital IXUS 85 IS / IXY Digital 25 IS" },
        { (long int)0x2380000, "PowerShot A590 IS" },
        { (long int)0x2390000, "PowerShot A580" },
        { (long int)0x2420000, "PowerShot A470" },
        { (long int)0x2430000, "PowerShot SD1100 IS / Digital IXUS 80 IS / IXY Digital 20 IS" },
        { (long int)0x2460000, "PowerShot SX1 IS" },
        { (long int)0x2470000, "PowerShot SX10 IS" },
        { (long int)0x2480000, "PowerShot A1000 IS" },
        { (long int)0x2490000, "PowerShot G10" },
        { (long int)0x2510000, "PowerShot A2000 IS" },
        { (long int)0x2520000, "PowerShot SX110 IS" },
        { (long int)0x2530000, "PowerShot SD990 IS / Digital IXUS 980 IS / IXY Digital 3000 IS" },
        { (long int)0x2540000, "PowerShot SD880 IS / Digital IXUS 870 IS / IXY Digital 920 IS" },
        { (long int)0x2550000, "PowerShot E1" },
        { (long int)0x2560000, "PowerShot D10" },
        { (long int)0x2570000, "PowerShot SD960 IS / Digital IXUS 110 IS / IXY Digital 510 IS" },
        { (long int)0x2580000, "PowerShot A2100 IS" },
        { (long int)0x2590000, "PowerShot A480" },
        { (long int)0x2600000, "PowerShot SX200 IS" },
        { (long int)0x2610000, "PowerShot SD970 IS / Digital IXUS 990 IS / IXY Digital 830 IS" },
        { (long int)0x2620000, "PowerShot SD780 IS / Digital IXUS 100 IS / IXY Digital 210 IS" },
        { (long int)0x2630000, "PowerShot A1100 IS" },
        { (long int)0x2640000, "PowerShot SD1200 IS / Digital IXUS 95 IS / IXY Digital 110 IS" },
        { (long int)0x2700000, "PowerShot G11" },
        { (long int)0x2710000, "PowerShot SX120 IS" },
        { (long int)0x2720000, "PowerShot S90" },
        { (long int)0x2750000, "PowerShot SX20 IS" },
        { (long int)0x2760000, "PowerShot SD980 IS / Digital IXUS 200 IS / IXY Digital 930 IS" },
        { (long int)0x2770000, "PowerShot SD940 IS / Digital IXUS 120 IS / IXY Digital 220 IS" },
        { (long int)0x2800000, "PowerShot A495" },
        { (long int)0x2810000, "PowerShot A490" },
        { (long int)0x2820000, "PowerShot A3100 IS / A3150 IS" },
        { (long int)0x2830000, "PowerShot A3000 IS" },
        { (long int)0x2840000, "PowerShot SD1400 IS / IXUS 130 / IXY 400F" },
        { (long int)0x2850000, "PowerShot SD1300 IS / IXUS 105 / IXY 200F" },
        { (long int)0x2860000, "PowerShot SD3500 IS / IXUS 210 / IXY 10S" },
        { (long int)0x2870000, "PowerShot SX210 IS" },
        { (long int)0x2880000, "PowerShot SD4000 IS / IXUS 300 HS / IXY 30S" },
        { (long int)0x2890000, "PowerShot SD4500 IS / IXUS 1000 HS / IXY 50S" },
        { (long int)0x2920000, "PowerShot G12" },
        { (long int)0x2930000, "PowerShot SX30 IS" },
        { (long int)0x2940000, "PowerShot SX130 IS" },
        { (long int)0x2950000, "PowerShot S95" },
        { (long int)0x2980000, "PowerShot A3300 IS" },
        { (long int)0x2990000, "PowerShot A3200 IS" },
        { (long int)0x3000000, "PowerShot ELPH 500 HS / IXUS 310 HS / IXY 31S" },
        { (long int)0x3010000, "PowerShot Pro90 IS" },
        { (long int)0x3010001, "PowerShot A800" },
        { (long int)0x3020000, "PowerShot ELPH 100 HS / IXUS 115 HS / IXY 210F" },
        { (long int)0x3030000, "PowerShot SX230 HS" },
        { (long int)0x3040000, "PowerShot ELPH 300 HS / IXUS 220 HS / IXY 410F" },
        { (long int)0x3050000, "PowerShot A2200" },
        { (long int)0x3060000, "PowerShot A1200" },
        { (long int)0x3070000, "PowerShot SX220 HS" },
        { (long int)0x3080000, "PowerShot G1 X" },
        { (long int)0x3090000, "PowerShot SX150 IS" },
        { (long int)0x3100000, "PowerShot ELPH 510 HS / IXUS 1100 HS / IXY 51S" },
        { (long int)0x3110000, "PowerShot S100 (new)" },
        { (long int)0x3130000, "PowerShot SX40 HS" },
        { (long int)0x3120000, "PowerShot ELPH 310 HS / IXUS 230 HS / IXY 600F" },
        { (long int)0x3140000, "IXY 32S" },
        { (long int)0x3160000, "PowerShot A1300" },
        { (long int)0x3170000, "PowerShot A810" },
        { (long int)0x3180000, "PowerShot ELPH 320 HS / IXUS 240 HS / IXY 420F" },
        { (long int)0x3190000, "PowerShot ELPH 110 HS / IXUS 125 HS / IXY 220F" },
        { (long int)0x3200000, "PowerShot D20" },
        { (long int)0x3210000, "PowerShot A4000 IS" },
        { (long int)0x3220000, "PowerShot SX260 HS" },
        { (long int)0x3230000, "PowerShot SX240 HS" },
        { (long int)0x3240000, "PowerShot ELPH 530 HS / IXUS 510 HS / IXY 1" },
        { (long int)0x3250000, "PowerShot ELPH 520 HS / IXUS 500 HS / IXY 3" },
        { (long int)0x3260000, "PowerShot A3400 IS" },
        { (long int)0x3270000, "PowerShot A2400 IS" },
        { (long int)0x3280000, "PowerShot A2300" },
        { (long int)0x3330000, "PowerShot G15" },
        { (long int)0x3340000, "PowerShot SX50" },
        { (long int)0x3350000, "PowerShot SX160 IS" },
        { (long int)0x3360000, "PowerShot S110 (new)" },
        { (long int)0x3370000, "PowerShot SX500 IS" },
        { (long int)0x3380000, "PowerShot N" },
        { (long int)0x3390000, "IXUS 245 HS / IXY 430F" },
        { (long int)0x3400000, "PowerShot SX280 HS" },
        { (long int)0x3410000, "PowerShot SX270 HS" },
        { (long int)0x3420000, "PowerShot A3500 IS" },
        { (long int)0x3430000, "PowerShot A2600" },
        { (long int)0x3450000, "PowerShot A1400" },
        { (long int)0x3460000, "PowerShot ELPH 130 IS / IXUS 140 / IXY 110F" },
        { (long int)0x3470000, "PowerShot ELPH 115/120 IS / IXUS 132/135 / IXY 90F/100F" },
        { (long int)0x3490000, "PowerShot ELPH 330 HS / IXUS 255 HS / IXY 610F" },
        { (long int)0x3510000, "PowerShot A2500" },
        { (long int)0x3540000, "PowerShot G16" },
        { (long int)0x3550000, "PowerShot S120" },
        { (long int)0x3560000, "PowerShot SX170 IS" },
        { (long int)0x3580000, "PowerShot SX510 HS" },
        { (long int)0x3590000, "PowerShot S200 (new)" },
        { (long int)0x3600000, "IXY 620F" },
        { (long int)0x3610000, "PowerShot N100" },
        { (long int)0x3640000, "PowerShot G1 X Mark II" },
        { (long int)0x3650000, "PowerShot D30" },
        { (long int)0x3660000, "PowerShot SX700 HS" },
        { (long int)0x3670000, "PowerShot SX600 HS" },
        { (long int)0x3680000, "PowerShot ELPH 140 IS / IXUS 150 / IXY 130" },
        { (long int)0x3690000, "PowerShot ELPH 135 / IXUS 145 / IXY 120" },
        { (long int)0x3700000, "PowerShot ELPH 340 HS / IXUS 265 HS / IXY 630" },
        { (long int)0x3710000, "PowerShot ELPH 150 IS / IXUS 155 / IXY 140" },
        { (long int)0x3740000, "EOS M3" },
        { (long int)0x3750000, "PowerShot SX60 HS" },
        { (long int)0x3760000, "PowerShot SX520 HS" },
        { (long int)0x3770000, "PowerShot SX400 IS" },
        { (long int)0x3780000, "PowerShot G7 X" },
        { (long int)0x3790000, "PowerShot N2" },
        { (long int)0x3800000, "PowerShot SX530 HS" },
        { (long int)0x3820000, "PowerShot SX710 HS" },
        { (long int)0x3830000, "PowerShot SX610 HS" },
        { (long int)0x3870000, "PowerShot ELPH 160 / IXUS 160" },
        { (long int)0x3890000, "PowerShot ELPH 170 IS / IXUS 170" },
        { (long int)0x3910000, "PowerShot SX410 HS" },
        { (long int)0x4040000, "PowerShot G1" },
        { (long int)0x6040000, "PowerShot S100 / Digital IXUS / IXY Digital" },
        { (long int)0x4007d673, "DC19/DC21/DC22" },
        { (long int)0x4007d674, "XH A1" },
        { (long int)0x4007d675, "HV10" },
        { (long int)0x4007d676, "MD130/MD140/MD150/MD160/ZR850" },
        { (long int)0x4007d777, "DC50" },
        { (long int)0x4007d778, "HV20" },
        { (long int)0x4007d779, "DC211" },
        { (long int)0x4007d77a, "HG10" },
        { (long int)0x4007d77b, "HR10" },
        { (long int)0x4007d77c, "MD255/ZR950" },
        { (long int)0x4007d81c, "HF11" },
        { (long int)0x4007d878, "HV30" },
        { (long int)0x4007d87c, "XH A1S" },
        { (long int)0x4007d87e, "DC301/DC310/DC311/DC320/DC330" },
        { (long int)0x4007d87f, "FS100" },
        { (long int)0x4007d880, "HF10" },
        { (long int)0x4007d882, "HG20/HG21" },
        { (long int)0x4007d925, "HF21" },
        { (long int)0x4007d926, "HF S11" },
        { (long int)0x4007d978, "HV40" },
        { (long int)0x4007d987, "DC410/DC411/DC420" },
        { (long int)0x4007d988, "FS19/FS20/FS21/FS22/FS200" },
        { (long int)0x4007d989, "HF20/HF200" },
        { (long int)0x4007d98a, "HF S10/S100" },
        { (long int)0x4007da8e, "HF R10/R16/R17/R18/R100/R106" },
        { (long int)0x4007da8f, "HF M30/M31/M36/M300/M306" },
        { (long int)0x4007da90, "HF S20/S21/S200" },
        { (long int)0x4007da92, "FS31/FS36/FS37/FS300/FS305/FS306/FS307" },
        { (long int)0x4007dda9, "HF G25" },
        { (long int)0x80000001, "EOS-1D" },
        { (long int)0x80000167, "EOS-1DS" },
        { (long int)0x80000168, "EOS 10D" },
        { (long int)0x80000169, "EOS-1D Mark III" },
        { (long int)0x80000170, "EOS Digital Rebel / 300D / Kiss Digital" },
        { (long int)0x80000174, "EOS-1D Mark II" },
        { (long int)0x80000175, "EOS 20D" },
        { (long int)0x80000176, "EOS Digital Rebel XSi / 450D / Kiss X2" },
        { (long int)0x80000188, "EOS-1Ds Mark II" },
        { (long int)0x80000189, "EOS Digital Rebel XT / 350D / Kiss Digital N" },
        { (long int)0x80000190, "EOS 40D" },
        { (long int)0x80000213, "EOS 5D" },
        { (long int)0x80000215, "EOS-1Ds Mark III" },
        { (long int)0x80000218, "EOS 5D Mark II" },
        { (long int)0x80000219, "WFT-E1" },
        { (long int)0x80000232, "EOS-1D Mark II N" },
        { (long int)0x80000234, "EOS 30D" },
        { (long int)0x80000236, "EOS Digital Rebel XTi / 400D / Kiss Digital X" },
        { (long int)0x80000241, "WFT-E2" },
        { (long int)0x80000246, "WFT-E3" },
        { (long int)0x80000250, "EOS 7D" },
        { (long int)0x80000252, "EOS Rebel T1i / 500D / Kiss X3" },
        { (long int)0x80000254, "EOS Rebel XS / 1000D / Kiss F" },
        { (long int)0x80000261, "EOS 50D" },
        { (long int)0x80000269, "EOS-1D X" },
        { (long int)0x80000270, "EOS Rebel T2i / 550D / Kiss X4" },
        { (long int)0x80000271, "WFT-E4" },
        { (long int)0x80000273, "WFT-E5" },
        { (long int)0x80000281, "EOS-1D Mark IV" },
        { (long int)0x80000285, "EOS 5D Mark III" },
        { (long int)0x80000286, "EOS Rebel T3i / 600D / Kiss X5" },
        { (long int)0x80000287, "EOS 60D" },
        { (long int)0x80000288, "EOS Rebel T3 / 1100D / Kiss X50" },
        { (long int)0x80000289, "EOS 7D Mark II" },
        { (long int)0x80000297, "WFT-E2 II" },
        { (long int)0x80000298, "WFT-E4 II" },
        { (long int)0x80000301, "EOS Rebel T4i / 650D / Kiss X6i" },
        { (long int)0x80000302, "EOS 6D" },
        { (long int)0x80000324, "EOS-1D C" },
        { (long int)0x80000325, "EOS 70D" },
        { (long int)0x80000326, "EOS Rebel T5i / 700D / Kiss X7i" },
        { (long int)0x80000327, "EOS Rebel T5 / 1200D / Kiss X70" },
        { (long int)0x80000331, "EOS M" },
        { (long int)0x80000355, "EOS M2" },
        { (long int)0x80000346, "EOS Rebel SL1 / 100D / Kiss X7" },
        { (long int)0x80000347, "EOS Rebel T6s / 760D / 8000D" },
        { (long int)0x80000382, "EOS 5DS" },
        { (long int)0x80000393, "EOS Rebel T6i / 750D / Kiss X8i" },
        { (long int)0x80000401, "EOS 5DS R" }
    };

    //! SerialNumberFormat, tag 0x0015
    extern const TagDetails canonSerialNumberFormat[] = {
        {  (long int)0x90000000, N_("Format 1") },
        {  (long int)0xa0000000, N_("Format 2") },
    };

    //! SuperMacro, tag 0x001a
    extern const TagDetails canonSuperMacro[] = {
        {  0, N_("Off")     },
        {  1, N_("On (1)")  },
        {  2, N_("On (2)")  }
    };

    //! ColorSpace, tag 0x00b4
    extern const TagDetails canonColorSpace[] = {
        {  1, N_("sRGB")      },
        {  2, N_("Adobe RGB") }
    };

    // Canon MakerNote Tag Info
    const TagInfo CanonMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "0x0000", "0x0000", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0001, "CameraSettings", N_("Camera Settings"), N_("Various camera settings"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0002, "FocalLength", N_("Focal Length"), N_("Focal length"), canonId, makerTags, unsignedShort, -1, printFocalLength),
        TagInfo(0x0003, "0x0003", "0x0003", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0004, "ShotInfo", N_("Shot Info"), N_("Shot information"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0005, "Panorama", N_("Panorama"), N_("Panorama"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0006, "ImageType", N_("Image Type"), N_("Image type"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0007, "FirmwareVersion", N_("Firmware Version"), N_("Firmware version"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0008, "FileNumber", N_("File Number"), N_("File number"), canonId, makerTags, unsignedLong, -1, print0x0008),
        TagInfo(0x0009, "OwnerName", N_("Owner Name"), N_("Owner Name"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x000c, "SerialNumber", N_("Serial Number"), N_("Camera serial number"), canonId, makerTags, unsignedLong, -1, print0x000c),
        TagInfo(0x000d, "CameraInfo", N_("Camera Info"), N_("Camera info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x000f, "CustomFunctions", N_("Custom Functions"), N_("Custom Functions"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0010, "ModelID", N_("ModelID"), N_("Model ID"), canonId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonModelId)),
        TagInfo(0x0012, "PictureInfo", N_("Picture Info"), N_("Picture info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0013, "ThumbnailImageValidArea", N_("Thumbnail Image Valid Area"), N_("Thumbnail image valid area"), canonId, makerTags, signedShort, -1, printValue),
        TagInfo(0x0015, "SerialNumberFormat", N_("Serial Number Format"), N_("Serial number format"), canonId, makerTags, unsignedLong, -1, EXV_PRINT_TAG(canonSerialNumberFormat)),
        TagInfo(0x001a, "SuperMacro", N_("Super Macro"), N_("Super macro"), canonId, makerTags, signedShort, -1, EXV_PRINT_TAG(canonSuperMacro)),
        TagInfo(0x0026, "AFInfo", N_("AF Info"), N_("AF info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0083, "OriginalDecisionDataOffset", N_("Original Decision Data Offset"), N_("Original decision data offset"), canonId, makerTags, signedLong, -1, printValue),
        TagInfo(0x00a4, "WhiteBalanceTable", N_("White Balance Table"), N_("White balance table"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x0095, "LensModel", N_("Lens Model"), N_("Lens model"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0096, "InternalSerialNumber", N_("Internal Serial Number"), N_("Internal serial number"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0097, "DustRemovalData", N_("Dust Removal Data"), N_("Dust removal data"), canonId, makerTags, asciiString, -1, printValue),
        TagInfo(0x0099, "CustomFunctions", N_("Custom Functions"), N_("Custom functions"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00a0, "ProcessingInfo", N_("Processing Info"), N_("Processing info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00aa, "MeasuredColor", N_("Measured Color"), N_("Measured color"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00b4, "ColorSpace", N_("ColorSpace"), N_("ColorSpace"), canonId, makerTags, signedShort,  -1, EXV_PRINT_TAG(canonColorSpace)),
        TagInfo(0x00b5, "0x00b5", "0x00b5", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00c0, "0x00c0", "0x00c0", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00c1, "0x00c1", "0x00c1", N_("Unknown"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x00d0, "VRDOffset", N_("VRD Offset"), N_("VRD offset"), canonId, makerTags, unsignedLong, -1, printValue),
        TagInfo(0x00e0, "SensorInfo", N_("Sensor Info"), N_("Sensor info"), canonId, makerTags, unsignedShort, -1, printValue),
        TagInfo(0x4001, "ColorData", N_("Color Data"), N_("Color data"), canonId, makerTags, unsignedShort, -1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", N_("Unknown CanonMakerNote tag"), canonId, makerTags, asciiString, -1, printValue)
    };

    const TagInfo* CanonMakerNote::tagList()
    {
        return tagInfo_;
    }

    //! Macro, tag 0x0001
    extern const TagDetails canonCsMacro[] = {
        { 1, N_("On")  },
        { 2, N_("Off") }
    };

    //! Quality, tag 0x0003
    extern const TagDetails canonCsQuality[] = {
        { 1,   N_("Economy")      },
        { 2,   N_("Normal")       },
        { 3,   N_("Fine")         },
        { 4,   N_("RAW")          },
        { 5,   N_("Superfine")    },
        { 130, N_("Normal Movie") },
        { 131, N_("Movie (2)")    }
    };

    //! FlashMode, tag 0x0004
    extern const TagDetails canonCsFlashMode[] = {
        {  0, N_("Off")            },
        {  1, N_("Auto")           },
        {  2, N_("On")             },
        {  3, N_("Red-eye")        },
        {  4, N_("Slow sync")      },
        {  5, N_("Auto + red-eye") },
        {  6, N_("On + red-eye")   },
        { 16, N_("External")       },
        { 16, N_("External")       }    // To silence compiler warning
    };

    //! DriveMode, tag 0x0005
    extern const TagDetails canonCsDriveMode[] = {
        { 0, N_("Single / timer")             },
        { 1, N_("Continuous")                 },
        { 2, N_("Movie")                      },
        { 3, N_("Continuous, speed priority") },
        { 4, N_("Continuous, low")            },
        { 5, N_("Continuous, high")           }
    };

    //! FocusMode, tag 0x0007
    extern const TagDetails canonCsFocusMode[] = {
        {  0, N_("One shot AF")  },
        {  1, N_("AI servo AF")  },
        {  2, N_("AI focus AF")  },
        {  3, N_("Manual focus") },
        {  4, N_("Single")       },
        {  5, N_("Continuous")   },
        {  6, N_("Manual focus") },
        { 16, N_("Pan focus")    },
        { 16, N_("Pan focus")    }    // To silence compiler warning
    };

    //! ImageSize, tag 0x000a
    extern const TagDetails canonCsImageSize[] = {
        { 0, N_("Large")    },
        { 1, N_("Medium")   },
        { 2, N_("Small")    },
        { 5, N_("Medium 1") },
        { 6, N_("Medium 2") },
        { 7, N_("Medium 3") }
    };

    //! EasyMode, tag 0x000b
    extern const TagDetails canonCsEasyMode[] = {
        {  0, N_("Full auto")        },
        {  1, N_("Manual")           },
        {  2, N_("Landscape")        },
        {  3, N_("Fast shutter")     },
        {  4, N_("Slow shutter")     },
        {  5, N_("Night Scene")      },
        {  6, N_("Gray scale")       },
        {  7, N_("Sepia")            },
        {  8, N_("Portrait")         },
        {  9, N_("Sports")           },
        { 10, N_("Macro / close-up") },
        { 11, N_("Black & white")    },
        { 12, N_("Pan focus")        },
        { 13, N_("Vivid")            },
        { 14, N_("Neutral")          },
        { 15, N_("Flash off")        },
        { 16, N_("Long shutter")     },
        { 17, N_("Super macro")      },
        { 18, N_("Foliage")          },
        { 19, N_("Indoor")           },
        { 20, N_("Fireworks")        },
        { 21, N_("Beach")            },
        { 22, N_("Underwater")       },
        { 23, N_("Snow")             },
        { 24, N_("Kids & pets")      },
        { 25, N_("Night SnapShot")   },
        { 26, N_("Digital macro")    },
        { 27, N_("My Colors")        },
        { 28, N_("Still image")      }
    };

    //! DigitalZoom, tag 0x000c
    extern const TagDetails canonCsDigitalZoom[] = {
        { 0, N_("None")  },
        { 1, "2x"        },
        { 2, "4x"        },
        { 3, N_("Other") },
        { 3, N_("Other") }                      // To silence compiler warning
    };

    //! Contrast, Saturation Sharpness, tags 0x000d, 0x000e, 0x000f
    extern const TagDetails canonCsLnh[] = {
        { 0xffff, N_("Low")    },
        { 0x0000, N_("Normal") },
        { 0x0001, N_("High")   }
    };

    //! ISOSpeeds, tag 0x0010
    extern const TagDetails canonCsISOSpeed[] = {
        {     0, N_("n/a")       },
        {    14, N_("Auto High") },
        {    15, N_("Auto")      },
        {    16,   "50"      },
        {    17,  "100"      },
        {    18,  "200"      },
        {    19,  "400"      },
        {    20,  "800"      },
        { 16464,   "80"      },
        { 16484,  "100"      },
        { 16584,  "200"      },
        { 16784,  "400"      },
        { 17184,  "800"      },
        { 17984, "1600"      },
        { 19584, "3200"      }
    };

    //! MeteringMode, tag 0x0011
    extern const TagDetails canonCsMeteringMode[] = {
        { 0, N_("Default")         },
        { 1, N_("Spot")            },
        { 2, N_("Average")         },
        { 3, N_("Evaluative")      },
        { 4, N_("Partial")         },
        { 5, N_("Center weighted") }
    };

    //! FocusType, tag 0x0012
    extern const TagDetails canonCsFocusType[] = {
        {  0, N_("Manual")       },
        {  1, N_("Auto")         },
        {  2, N_("Not known")    },
        {  3, N_("Macro")        },
        {  4, N_("Very close")   },
        {  5, N_("Close")        },
        {  6, N_("Middle range") },
        {  7, N_("Far range")    },
        {  8, N_("Pan focus")    },
        {  9, N_("Super macro")  },
        { 10, N_("Infinity")     }
    };

    //! AFPoint, tag 0x0013
    extern const TagDetails canonCsAfPoint[] = {
        { 0x2005, N_("Manual AF point selection") },
        { 0x3000, N_("None (MF)")                 },
        { 0x3001, N_("Auto-selected")             },
        { 0x3002, N_("Right")                     },
        { 0x3003, N_("Center")                    },
        { 0x3004, N_("Left")                      },
        { 0x4001, N_("Auto AF point selection")   }
    };

    //! ExposureProgram, tag 0x0014
    extern const TagDetails canonCsExposureProgram[] = {
        { 0, N_("Easy shooting (Auto)")   },
        { 1, N_("Program (P)")            },
        { 2, N_("Shutter priority (Tv)")  },
        { 3, N_("Aperture priority (Av)") },
        { 4, N_("Manual (M)")             },
        { 5, N_("A-DEP")                  },
        { 6, N_("M-DEP")                  }
    };

    //! LensType, tag 0x0016
    extern const TagDetails canonCsLensType[] = {
        {   1, "Canon EF 50mm f/1.8"                                        },
        {   2, "Canon EF 28mm f/2.8"                                        },
        {   3, "Canon EF 135mm f/2.8 Soft"                                  },
        {   4, "Canon EF 35-105mm f/3.5-4.5"                                }, // 0
        {   4, "Sigma UC Zoom 35-135mm f/4-5.6"                             }, // 1
        {   5, "Canon EF 35-70mm f/3.5-4.5"                                 },
        {   6, "Canon EF 28-70mm f/3.5-4.5"                                 }, // 0
        {   6, "Sigma 18-50mm f/3.5-5.6 DC"                                 }, // 1
        {   6, "Sigma 18-125mm f/3.5-5.6 DC IF ASP"                         }, // 2
        {   6, "Tokina AF193-2 19-35mm f/3.5-4.5"                           }, // 3
        {   6, "Sigma 28-80mm f/3.5-5.6 II Macro"                           }, // 4
        {   7, "Canon EF 100-300mm f/5.6L"                                  },
        {   8, "Canon EF 100-300mm f/5.6"                                   }, // 0
        {   8, "Sigma 70-300mm f/4-5.6 [APO] DG Macro"                      }, // 1
        {   8, "Tokina AT-X 242 AF 24-200mm f/3.5-5.6"                      }, // 2
        {   9, "Canon EF 70-210mm f/4"                                      }, // 0
        {   9, "Sigma 55-200mm f/4-5.6 DC"                                  }, // 1
        {  10, "Canon EF 50mm f/2.5 Macro"                                  }, // 0
        {  10, "Sigma 50mm f/2.8 EX"                                        }, // 1
        {  10, "Sigma 28mm f/1.8"                                           }, // 2
        {  10, "Sigma 105mm f/2.8 Macro EX"                                 }, // 3
        {  10, "Sigma 70mm f/2.8 EX DG Macro EF"                            }, // 4
        {  11, "Canon EF 35mm f/2"                                          },
        {  13, "Canon EF 15mm f/2.8 Fisheye"                                },
        {  14, "Canon EF 50-200mm f/3.5-4.5L"                               },
        {  15, "Canon EF 50-200mm f/3.5-4.5"                                },
        {  16, "Canon EF 35-135mm f/3.5-4.5"                                },
        {  17, "Canon EF 35-70mm f/3.5-4.5A"                                },
        {  18, "Canon EF 28-70mm f/3.5-4.5"                                 },
        {  20, "Canon EF 100-200mm f/4.5A"                                  },
        {  21, "Canon EF 80-200mm f/2.8L"                                   },
        {  22, "Canon EF 20-35mm f/2.8L"                                    }, // 0
        {  22, "Tokina AT-X 280 AF PRO 28-80mm f/2.8 Aspherical"            }, // 1
        {  23, "Canon EF 35-105mm f/3.5-4.5"                                },
        {  24, "Canon EF 35-80mm f/4-5.6 Power Zoom"                        },
        {  25, "Canon EF 35-80mm f/4-5.6 Power Zoom"                        },
        {  26, "Canon EF 100mm f/2.8 Macro"                                 }, // 0
        {  26, "Cosina 100mm f/3.5 Macro AF"                                }, // 1
        {  26, "Tamron SP AF 90mm f/2.8 Di Macro"                           }, // 2
        {  26, "Tamron SP AF 180mm f/3.5 Di Macro"                          }, // 3
        {  26, "Carl Zeiss Planar T* 50mm f/1.4"                            }, // 4
        {  27, "Canon EF 35-80mm f/4-5.6"                                   },
        {  28, "Canon EF 80-200mm f/4.5-5.6"                                }, // 0
        {  28, "Tamron SP AF 28-105mm f/2.8 LD Aspherical IF"               }, // 1
        {  28, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"  }, // 2
        {  28, "Tamron AF 70-300mm f/4-5.6 Di LD 1:2 Macro"                 }, // 3
        {  28, "Tamron AF Aspherical 28-200mm f/3.8-5.6"                    }, // 4
        {  29, "Canon EF 50mm f/1.8 II"                                     },
        {  30, "Canon EF 35-105mm f/4.5-5.6"                                },
        {  31, "Canon EF 75-300mm f/4-5.6"                                  }, // 0
        {  31, "Tamron SP AF 300mm f/2.8 LD IF"                             }, // 1
        {  32, "Canon EF 24mm f/2.8"                                        }, // 0
        {  32, "Sigma 15mm f/2.8 EX Fisheye"                                }, // 1
        {  33, "Voigtlander or Carl Zeiss Lens"                             }, // 0
        {  33, "Voigtlander Ultron 40mm f/2 SLII Aspherical"                }, // 1
        {  33, "Voigtlander Color Skopar 20mm f/3.5 SLII Aspherical"        }, // 2
        {  33, "Voigtlander APO-Lanthar 90mm f/3.5 SLII Close Focus"        }, // 3
        {  33, "Carl Zeiss Distagon 15mm T* f/2.8 ZE"                       }, // 4
        {  33, "Carl Zeiss Distagon 18mm T* f/3.5 ZE"                       }, // 5
        {  33, "Carl Zeiss Distagon 21mm T* f/2.8 ZE"                       }, // 6
        {  33, "Carl Zeiss Distagon 25mm T* f/2 ZE"                         }, // 7
        {  33, "Carl Zeiss Distagon 28mm T* f/2 ZE"                         }, // 8
        {  33, "Carl Zeiss Distagon 35mm T* f/2 ZE"                         }, // 9
        {  33, "Carl Zeiss Distagon 35mm T* f/1.4 ZE"                       }, // 10
        {  33, "Carl Zeiss Planar 50mm T* f/1.4 ZE"                         }, // 11
        {  33, "Carl Zeiss Makro-Planar T* 50mm f/2 ZE"                     }, // 12
        {  33, "Carl Zeiss Makro-Planar T* 100mm f/2 ZE"                    }, // 13
        {  33, "Carl Zeiss Apo-Sonnar T* 135mm f/2 ZE"                      }, // 14
        {  35, "Canon EF 35-80mm f/4-5.6"                                   },
        {  36, "Canon EF 38-76mm f/4.5-5.6"                                 },
        {  37, "Canon EF 35-80mm f/4-5.6"                                   }, // 0
        {  37, "Tamron 70-200mm f/2.8 Di LD IF Macro"                       }, // 1
        {  37, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro Model A20" }, // 2
        {  37, "Tamron SP AF 17-50mm f/2.8 XR Di II VC LD Aspherical [IF] " }, // 3
        {  37, "Tamron AF 18-270mm f/3.5-6.3 Di II VC LD Aspherical [IF] Macro" }, // 4
        {  38, "Canon EF 80-200mm f/4.5-5.6"                                },
        {  39, "Canon EF 75-300mm f/4-5.6"                                  },
        {  40, "Canon EF 28-80mm f/3.5-5.6"                                 },
        {  41, "Canon EF 28-90mm f/4-5.6"                                   },
        {  42, "Canon EF 28-200mm f/3.5-5.6"                                }, // 0
        {  42, "Tamron AF 28-300mm f/3.5-6.3 XR Di VC LD Aspherical [IF] Macro Model A20" }, // 1
        {  43, "Canon EF 28-105mm f/4-5.6"                                  },
        {  44, "Canon EF 90-300mm f/4.5-5.6"                                },
        {  45, "Canon EF-S 18-55mm f/3.5-5.6"                               },
        {  46, "Canon EF 28-90mm f/4-5.6"                                   },
        {  48, "Canon EF-S 18-55mm f/3.5-5.6 IS"                            },
        {  49, "Canon EF-S 55-250mm f/4-5.6 IS"                             },
        {  50, "Canon EF-S 18-200mm f/3.5-5.6 IS"                           },
        {  51, "Canon EF-S 18-135mm f/3.5-5.6 IS"                           },
        {  52, "Canon EF-S 18-55mm f/3.5-5.6 IS II"                         },
        {  53, "Canon EF-S 18-55mm f/3.5-5.6 III"                           },
        {  54, "Canon EF-S 55-250mm f/4-5.6 IS II"                          },
        {  94, "Canon TS-E 17mm f/4L"                                       },
        {  95, "Canon TS-E 24.0mm f/3.5 L II"                               },
        { 124, "Canon MP-E 65mm f/2.8 1-5x Macro Photo"                     },
        { 125, "Canon TS-E 24mm f/3.5L"                                     },
        { 126, "Canon TS-E 45mm f/2.8"                                      },
        { 127, "Canon TS-E 90mm f/2.8"                                      },
        { 129, "Canon EF 300mm f/2.8L"                                      },
        { 130, "Canon EF 50mm f/1.0L"                                       },
        { 131, "Canon EF 28-80mm f/2.8-4L"                                  }, // 0
        { 131, "Sigma 8mm f/3.5 EX DG Circular Fisheye"                     }, // 1
        { 131, "Sigma 17-35mm f/2.8-4 EX DG Aspherical HSM"                 }, // 2
        { 131, "Sigma 17-70mm f/2.8-4.5 DC Macro"                           }, // 3
        { 131, "Sigma APO 50-150mm f/2.8 EX DC HSM"                         }, // 4
        { 131, "Sigma APO 120-300mm f/2.8 EX DG HSM"                        }, // 5
        { 131, "Sigma 4.5mm F2.8 EX DC HSM Circular Fisheye"                }, // 6
        { 131, "Sigma 70-200mm f/2.8 APO EX HSM"                            }, // 7
        { 132, "Canon EF 1200mm f/5.6L"                                     },
        { 134, "Canon EF 600mm f/4L IS"                                     },
        { 135, "Canon EF 200mm f/1.8L"                                      },
        { 136, "Canon EF 300mm f/2.8L"                                      },
        { 137, "Canon EF 85mm f/1.2L"                                       }, // 0
        { 137, "Sigma 18-50mm f/2.8-4.5 DC OS HSM"                          }, // 1
        { 137, "Sigma 50-200mm f/4-5.6 DC OS HSM"                           }, // 2
        { 137, "Sigma 18-250mm f/3.5-6.3 DC OS HSM"                         }, // 3
        { 137, "Sigma 24-70mm f/2.8 IF EX DG HSM"                           }, // 4
        { 137, "Sigma 18-125mm f/3.8-5.6 DC OS HSM"                         }, // 5
        { 137, "Sigma 17-70mm f/2.8-4 DC Macro OS HSM"                      }, // 6
        { 137, "Sigma 17-50mm f/2.8 OS HSM"                                 }, // 7
        { 137, "Sigma 18-200mm f/3.5-6.3 II DC OS HSM"                      }, // 8
        { 137, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                  }, // 9
        { 137, "Sigma 8-16mm f/4.5-5.6 DC HSM"                              }, // 10
        { 137, "Tamron SP 17-50mm f/2.8 XR Di II VC"                        }, // 11
        { 137, "Tamron SP 60mm f/2 Macro Di II"                             }, // 12
        { 137, "Sigma 10-20mm f/3.5 EX DC HSM"                              }, // 13
        { 137, "Tamron SP 24-70mm f/2.8 Di VC USD"                          }, // 14
        { 137, "Sigma 18-35mm f/1.8 DC HSM"                                 }, // 15
        { 137, "Sigma 12-24mm f/4.5-5.6 DG HSM II"                          }, // 16
        { 138, "Canon EF 28-80mm f/2.8-4L"                                  },
        { 139, "Canon EF 400mm f/2.8L"                                      },
        { 140, "Canon EF 500mm f/4.5L"                                      },
        { 141, "Canon EF 500mm f/4.5L"                                      },
        { 142, "Canon EF 300mm f/2.8L IS"                                   },
        { 143, "Canon EF 500mm f/4L IS"                                     },
        { 144, "Canon EF 35-135mm f/4-5.6 USM"                              },
        { 145, "Canon EF 100-300mm f/4.5-5.6 USM"                           },
        { 146, "Canon EF 70-210mm f/3.5-4.5 USM"                            },
        { 147, "Canon EF 35-135mm f/4-5.6 USM"                              },
        { 148, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 149, "Canon EF 100mm f/2 USM"                                     },
        { 150, "Canon EF 14mm f/2.8L"                                       }, // 0
        { 150, "Sigma 20mm EX f/1.8"                                        }, // 1
        { 150, "Sigma 30mm f/1.4 DC HSM"                                    }, // 2
        { 150, "Sigma 24mm f/1.8 DG Macro EX"                               }, // 3
        { 150, "Sigma 28mm f/1.8 DG Macro EX"                               }, // 4
        { 151, "Canon EF 200mm f/2.8L"                                      },
        { 152, "Canon EF 300mm f/4L IS"                                     }, // 0
        { 152, "Sigma 12-24mm f/4.5-5.6 EX DG ASPHERICAL HSM"               }, // 1
        { 152, "Sigma 14mm f/2.8 EX Aspherical HSM"                         }, // 2
        { 152, "Sigma 10-20mm f/4-5.6"                                      }, // 3
        { 152, "Sigma 100-300mm f/4"                                        }, // 4
        { 153, "Canon EF 35-350mm f/3.5-5.6L"                               }, // 0
        { 153, "Sigma 50-500mm f/4-6.3 APO HSM EX"                          }, // 1
        { 153, "Tamron AF 28-300mm f/3.5-6.3 XR LD Aspherical [IF] Macro"   }, // 2
        { 153, "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical [IF] Macro Model A14" }, // 3
        { 153, "Tamron 18-250mm f/3.5-6.3 Di II LD Aspherical [IF] Macro"   }, // 4
        { 154, "Canon EF 20mm f/2.8 USM"                                    },
        { 155, "Canon EF 85mm f/1.8 USM"                                    },
        { 156, "Canon EF 28-105mm f/3.5-4.5 USM"                            }, // 0
        { 156, "Tamron SP 70-300mm f/4-5.6 Di VC USD"                       }, // 1
        { 160, "Canon EF 20-35mm f/3.5-4.5 USM"                             }, // 0
        { 160, "Tamron AF 19-35mm f/3.5-4.5"                                }, // 1
        { 160, "Tokina AT-X 124 AF 12-24mm f/4 DX"                          }, // 2
        { 160, "Tokina AT-X 107 AF DX Fish-eye 10-17mm f/3.5-4.5"           }, // 3
        { 160, "Tokina AT-X 116 PRO DX AF 11-16mm f/2.8"                    }, // 4
        { 161, "Canon EF 28-70mm f/2.8L"                                    }, // 0
        { 161, "Sigma 24-70mm EX f/2.8"                                     }, // 1
        { 161, "Sigma 28-70mm f/2.8 EX"                                     }, // 2
        { 161, "Sigma 24-60mm f/2.8 EX DG"                                  }, // 3
        { 161, "Tamron AF 17-50mm f/2.8 Di-II LD Aspherical"                }, // 4
        { 161, "Tamron 90mm f/2.8"                                          }, // 5
        { 161, "Tamron SP AF 17-35mm f/2.8-4 Di LD Aspherical IF"           }, // 6
        { 161, "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical [IF] Macro"  }, // 7
        { 162, "Canon EF 200mm f/2.8L"                                      },
        { 163, "Canon EF 300mm f/4L"                                        },
        { 164, "Canon EF 400mm f/5.6L"                                      },
        { 165, "Canon EF 70-200mm f/2.8 L"                                  },
        { 166, "Canon EF 70-200mm f/2.8 L + 1.4x"                           },
        { 167, "Canon EF 70-200mm f/2.8 L + 2x"                             },
        { 168, "Canon EF 28mm f/1.8 USM"                                    },
        { 169, "Canon EF 17-35mm f/2.8L"                                    }, // 0
        { 169, "Sigma 18-200mm f/3.5-6.3 DC OS"                             }, // 1
        { 169, "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"                   }, // 2
        { 169, "Sigma 18-50mm f/2.8 Macro"                                  }, // 3
        { 169, "Sigma 50mm f/1.4 EX DG HSM"                                 }, // 4
        { 169, "Sigma 85mm f/1.4 EX DG HSM"                                 }, // 5
        { 169, "Sigma 30mm f/1.4 EX DC HSM"                                 }, // 6
        { 169, "Sigma 35mm f/1.4 DG HSM"                                    }, // 7
        { 170, "Canon EF 200mm f/2.8L II"                                   },
        { 171, "Canon EF 300mm f/4L"                                        },
        { 172, "Canon EF 400mm f/5.6L"                                      }, // 0
        { 172, "Sigma 150-600mm f/5-6.3 DG OS HSM | S"                      }, // 1
        { 173, "Canon EF 180mm Macro f/3.5L"                                }, // 0
        { 173, "Sigma 180mm EX HSM Macro f/3.5"                             }, // 1
        { 173, "Sigma APO Macro 150mm f/3.5 EX DG IF HSM"                   }, // 2
        { 174, "Canon EF 135mm f/2L"                                        }, // 0
        { 174, "Sigma 70-200mm f/2.8 EX DG APO OS HSM"                      }, // 1
        { 174, "Sigma 50-500mm f/4.5-6.3 APO DG OS HSM"                     }, // 2
        { 174, "Sigma 150-500mm f/5-6.3 APO DG OS HSM"                      }, // 3
        { 175, "Canon EF 400mm f/2.8L"                                      },
        { 176, "Canon EF 24-85mm f/3.5-4.5 USM"                             },
        { 177, "Canon EF 300mm f/4L IS"                                     },
        { 178, "Canon EF 28-135mm f/3.5-5.6 IS"                             },
        { 179, "Canon EF 24mm f/1.4L"                                       },
        { 180, "Canon EF 35mm f/1.4L"                                       }, // 0
        { 180, "Sigma 50mm f/1.4 DG HSM | A"                                }, // 1
        { 180, "Sigma 24mm f/1.4 DG HSM | A"                                }, // 2
        { 181, "Canon EF 100-400mm f/4.5-5.6L IS + 1.4x"                    },
        { 182, "Canon EF 100-400mm f/4.5-5.6L IS + 2x"                      },
        { 183, "Canon EF 100-400mm f/4.5-5.6L IS"                           }, // 0
        { 183, "Sigma 150mm f/2.8 EX DG OS HSM APO Macro"                   }, // 1
        { 183, "Sigma 105mm f/2.8 EX DG OS HSM Macro"                       }, // 2
        { 183, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                   }, // 3
        { 183, "Sigma 150-600mm f/5-6.3 DG OS HSM | C"                      }, // 4
        { 184, "Canon EF 400mm f/2.8L + 2x"                                 },
        { 185, "Canon EF 600mm f/4L IS"                                     },
        { 186, "Canon EF 70-200mm f/4L"                                     },
        { 187, "Canon EF 70-200mm f/4L + 1.4x"                              },
        { 188, "Canon EF 70-200mm f/4L + 2x"                                },
        { 189, "Canon EF 70-200mm f/4L + 2.8x"                              },
        { 190, "Canon EF 100mm f/2.8 Macro USM"                             },
        { 191, "Canon EF 400mm f/4 DO IS"                                   },
        { 193, "Canon EF 35-80mm f/4-5.6 USM"                               },
        { 194, "Canon EF 80-200mm f/4.5-5.6 USM"                            },
        { 195, "Canon EF 35-105mm f/4.5-5.6 USM"                            },
        { 196, "Canon EF 75-300mm f/4-5.6 USM"                              },
        { 197, "Canon EF 75-300mm f/4-5.6 IS USM"                           },
        { 198, "Canon EF 50mm f/1.4 USM"                                    }, // 0
        { 198, "Zeiss Otus 55mm f/1.4 ZE"                                   }, // 1
        { 198, "Zeiss Otus 85mm f/1.4 ZE"                                   }, // 2
        { 199, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 200, "Canon EF 75-300mm f/4-5.6 USM"                              },
        { 201, "Canon EF 28-80mm f/3.5-5.6 USM"                             },
        { 202, "Canon EF 28-80mm f/3.5-5.6 USM IV"                          },
        { 208, "Canon EF 22-55mm f/4-5.6 USM"                               },
        { 209, "Canon EF 55-200mm f/4.5-5.6"                                },
        { 210, "Canon EF 28-90mm f/4-5.6 USM"                               },
        { 211, "Canon EF 28-200mm f/3.5-5.6 USM"                            },
        { 212, "Canon EF 28-105mm f/4-5.6 USM"                              },
        { 213, "Canon EF 90-300mm f/4.5-5.6 USM"                            }, // 0
        { 213, "Tamron SP 150-600mm F/5-6.3 Di VC USD"                      }, // 1
        { 213, "Tamron 16-300mm f/3.5-6.3 Di II VC PZD Macro"               }, // 2
        { 213, "Tamron SP 70-300mm f/4-5.6 Di VC USD"                       }, // 3
        { 214, "Canon EF-S 18-55mm f/3.5-5.6 USM"                           },
        { 215, "Canon EF 55-200mm f/4.5-5.6 II USM"                         },
        { 217, "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD"                  },
        { 224, "Canon EF 70-200mm f/2.8L IS"                                },
        { 225, "Canon EF 70-200mm f/2.8L IS + 1.4x"                         },
        { 226, "Canon EF 70-200mm f/2.8L IS + 2x"                           },
        { 227, "Canon EF 70-200mm f/2.8L IS + 2.8x"                         },
        { 228, "Canon EF 28-105mm f/3.5-4.5 USM"                            },
        { 229, "Canon EF 16-35mm f/2.8L"                                    },
        { 230, "Canon EF 24-70mm f/2.8L"                                    },
        { 231, "Canon EF 17-40mm f/4L"                                      },
        { 232, "Canon EF 70-300mm f/4.5-5.6 DO IS USM"                      },
        { 233, "Canon EF 28-300mm f/3.5-5.6L IS"                            },
        { 234, "Canon EF-S 17-85mm f4-5.6 IS USM"                           }, // 0
        { 234, "Tokina AT-X 12-28mm f/4 PRO DX"                             }, // 1
        { 235, "Canon EF-S 10-22mm f/3.5-4.5 USM"                           },
        { 236, "Canon EF-S 60mm f/2.8 Macro USM"                            },
        { 237, "Canon EF 24-105mm f/4L IS"                                  },
        { 238, "Canon EF 70-300mm f/4-5.6 IS USM"                           },
        { 239, "Canon EF 85mm f/1.2L II"                                    },
        { 240, "Canon EF-S 17-55mm f/2.8 IS USM"                            },
        { 241, "Canon EF 50mm f/1.2L"                                       },
        { 242, "Canon EF 70-200mm f/4L IS"                                  },
        { 243, "Canon EF 70-200mm f/4L IS + 1.4x"                           },
        { 244, "Canon EF 70-200mm f/4L IS + 2x"                             },
        { 245, "Canon EF 70-200mm f/4L IS + 2.8x"                           },
        { 246, "Canon EF 16-35mm f/2.8L II"                                 },
        { 247, "Canon EF 14mm f/2.8L II USM"                                },
        { 248, "Canon EF 200mm f/2L IS"                                     },
        { 249, "Canon EF 800mm f/5.6L IS"                                   },
        { 250, "Canon EF 24 f/1.4L II"                                      },
        { 251, "Canon EF 70-200mm f/2.8L IS II USM"                         },
        { 252, "Canon EF 70-200mm f/2.8L IS II USM + 1.4x"                  },
        { 253, "Canon EF 70-200mm f/2.8L IS II USM + 2x"                    },
        { 254, "Canon EF 100mm f/2.8L Macro IS USM"                         },
        { 255, "Sigma 24-105mm f/4 DG OS HSM | A"                           }, // 0
        { 255, "Sigma 180mm f/2.8 EX DG OS HSM APO Macro"                   }, // 1
        { 488, "Canon EF-S 15-85mm f/3.5-5.6 IS USM"                        },
        { 489, "Canon EF 70-300mm f/4-5.6L IS USM"                          },
        { 490, "Canon EF 8-15mm f/4L Fisheye USM"                           },
        { 491, "Canon EF 300mm f/2.8L IS II USM"                            },
        { 492, "Canon EF 400mm f/2.8L IS II USM"                            },
        { 493, "Canon EF 500mm f/4L IS II USM"                              }, // 0
        { 493, "Canon EF 24-105mm f/4L IS USM"                              }, // 1
        { 494, "Canon EF 600mm f/4.0L IS II USM"                            },
        { 495, "Canon EF 24-70mm f/2.8L II USM"                             },
        { 496, "Canon EF 200-400mm f/4L IS USM"                             },
        { 499, "Canon EF 200-400mm f/4L IS USM + 1.4x"                      },
        { 502, "Canon EF 28mm f/2.8 IS USM"                                 },
        { 503, "Canon EF 24mm f/2.8 IS USM"                                 },
        { 504, "Canon EF 24-70mm f/4L IS USM"                               },
        { 505, "Canon EF 35mm f/2 IS USM"                                   },
        { 506, "Canon EF 400mm f/4 DO IS II USM"                            },
        { 507, "Canon EF 16-35mm f/4L IS USM"                               },
        { 508, "Canon EF 11-24mm f/4L USM"                                  },
        { 4142,"Canon EF-S 18-135mm f/3.5-5.6 IS STM"                       },
        { 4143,"Canon EF-M 18-55mm f/3.5-5.6 IS STM"                        }, // 0
        { 4143,"Tamron 18-200mm F/3.5-6.3 Di III VC"                        }, // 1
        { 4144,"Canon EF 40mm f/2.8 STM"                                    },
        { 4145,"Canon EF-M 22mm f/2 STM"                                    },
        { 4146,"Canon EF-S 18-55mm f/3.5-5.6 IS STM"                        },
        { 4147,"Canon EF-M 11-22mm f/4-5.6 IS STM"                          },
        { 4148,"Canon EF-S 55-250mm f/4-5.6 IS STM"                         },
        { 4149,"Canon EF-M 55-200mm f/4.5-6.3 IS STM"                       },
        { 4150,"Canon EF-S 10-18mm f/4.5-5.6 IS STM"                        },
        { 4152,"Canon EF 24-105mm f/3.5-5.6 IS STM"                         },
        { 4154,"Canon EF-S 24mm f/2.8 STM"                                  },
        { 4156,"Canon EF 50mm f/1.8 STM"                                    }
    };

    //! A lens id and a pretty-print function for special treatment of the id.
    struct LensIdFct {
        long     id_;                           //!< Lens id
        PrintFct fct_;                          //!< Pretty-print function
        //! Comparison operator for find template
        bool operator==(long id) const { return id_ == id; }
    };

    //! List of lens ids which require special treatment with the medicine
    const LensIdFct lensIdFct[] = {
        {   4, printCsLensByFocalLength }, // not tested
        {   6, printCsLensByFocalLength },
        {   8, printCsLensByFocalLength },
        {   9, printCsLensByFocalLength },
        {  10, printCsLensByFocalLengthAndMaxAperture }, // works partly
        {  22, printCsLensByFocalLength },
        {  26, printCsLensByFocalLengthAndMaxAperture }, // works partly
        {  28, printCsLensByFocalLength },
        {  31, printCsLensByFocalLength },
        {  32, printCsLensByFocalLength },
        {  33, printCsLensByFocalLengthAndMaxAperture }, // works partly
        {  37, printCsLensByFocalLength },
        {  42, printCsLensByFocalLength },
        { 131, printCsLensByFocalLength },
        { 137, printCsLensByFocalLength }, // not tested
        { 150, printCsLensByFocalLength },
        { 152, printCsLensByFocalLength },
        { 153, printCsLensByFocalLength },
        { 156, printCsLensByFocalLength },
        { 160, printCsLensByFocalLength },
        { 161, printCsLensByFocalLength },
        { 169, printCsLensByFocalLength },
        { 172, printCsLensByFocalLength }, // not tested
        { 173, printCsLensByFocalLength }, // works partly
        { 174, printCsLensByFocalLength }, // not tested
        { 180, printCsLensByFocalLength },
        { 183, printCsLensByFocalLength }, // not tested
        { 198, printCsLensByFocalLength }, // not tested
        { 213, printCsLensByFocalLength }, // not tested
        { 234, printCsLensByFocalLength }, // not tested
        { 255, printCsLensByFocalLength }, // not tested
        { 493, printCsLensByFocalLength }, // not tested
        { 4143,printCsLensByFocalLength }  // not tested
    };

    //! FlashActivity, tag 0x001c
    extern const TagDetails canonCsFlashActivity[] = {
        { 0, N_("Did not fire") },
        { 1, N_("Fired")        }
    };

    //! FlashDetails, tag 0x001d
    extern const TagDetailsBitmask canonCsFlashDetails[] = {
        { 0x4000, N_("External flash")        },
        { 0x2000, N_("Internal flash")        },
        { 0x0001, N_("Manual")                },
        { 0x0002, N_("TTL")                   },
        { 0x0004, N_("A-TTL")                 },
        { 0x0008, N_("E-TTL")                 },
        { 0x0010, N_("FP sync enabled")       },
        { 0x0080, N_("2nd-curtain sync used") },
        { 0x0800, N_("FP sync used")          }
    };

    //! FocusContinuous, tag 0x0020
    extern const TagDetails canonCsFocusContinuous[] = {
        { 0, N_("Single")     },
        { 1, N_("Continuous") }
    };

    //! AESetting, tag 0x0021
    extern const TagDetails canonCsAESetting[] = {
        { 0, N_("Normal AE")                       },
        { 1, N_("Exposure compensation")           },
        { 2, N_("AE lock")                         },
        { 3, N_("AE lock + exposure compensation") },
        { 4, N_("No AE")                           }
    };

    //! ImageStabilization, tag 0x0022
    extern const TagDetails canonCsImageStabilization[] = {
        { 0, N_("Off")           },
        { 1, N_("On")            },
        { 2, N_("On, shot only") }
    };

    //! SpotMeteringMode, tag 0x0027
    extern const TagDetails canonCsSpotMeteringMode[] = {
        { 0,   N_("Center")   },
        { 1,   N_("AF Point") }
    };

    //! PhotoEffect, tag 0x0028
    extern const TagDetails canonCsPhotoEffect[] = {
        { 0,   N_("Off")           },
        { 1,   N_("Vivid")         },
        { 2,   N_("Neutral")       },
        { 3,   N_("Smooth")        },
        { 4,   N_("Sepia")         },
        { 5,   N_("B&W")           },
        { 6,   N_("Custom")        },
        { 100, N_("My color data") },
        { 100, N_("My color data") }    // To silence compiler warning
    };

    //! ManualFlashOutput, tag 0x0029
    extern const TagDetails canonCsManualFlashOutput[] = {
        { 0x0000, N_("n/a")    },
        { 0x0500, N_("Full")   },
        { 0x0502, N_("Medium") },
        { 0x0504, N_("Low")    },
        { 0x7fff, N_("n/a")    }
    };

    //! SRAWQuality, tag 0x002e
    extern const TagDetails canonCsSRAWQuality[] = {
        { 0, N_("n/a")          },
        { 1, N_("sRAW1 (mRAW)") },
        { 2, N_("sRAW2 (sRAW)") }
    };

    // Canon Camera Settings Tag Info
    const TagInfo CanonMakerNote::tagInfoCs_[] = {
        TagInfo(0x0001, "Macro", N_("Macro"), N_("Macro mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsMacro)),
        TagInfo(0x0002, "Selftimer", N_("Selftimer"), N_("Self timer"), canonCsId, makerTags, signedShort, 1, printCs0x0002),
        TagInfo(0x0003, "Quality", N_("Quality"), N_("Quality"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsQuality)),
        TagInfo(0x0004, "FlashMode", N_("Flash Mode"), N_("Flash mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFlashMode)),
        TagInfo(0x0005, "DriveMode", N_("Drive Mode"), N_("Drive mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsDriveMode)),
        TagInfo(0x0006, "0x0006", "0x0006", N_("Unknown"), canonCsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "FocusMode", N_("Focus Mode"), N_("Focus mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusMode)),
        TagInfo(0x0008, "0x0008", "0x0008", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000a, "ImageSize", N_("Image Size"), N_("Image size"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageSize)),
        TagInfo(0x000b, "EasyMode", N_("Easy Mode"), N_("Easy shooting mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsEasyMode)),
        TagInfo(0x000c, "DigitalZoom", N_("Digital Zoom"), N_("Digital zoom"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsDigitalZoom)),
        TagInfo(0x000d, "Contrast", N_("Contrast"), N_("Contrast setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x000e, "Saturation", N_("Saturation"), N_("Saturation setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x000f, "Sharpness", N_("Sharpness"), N_("Sharpness setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsLnh)),
        TagInfo(0x0010, "ISOSpeed", N_("ISO Speed Mode"), N_("ISO speed setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsISOSpeed)),
        TagInfo(0x0011, "MeteringMode", N_("Metering Mode"), N_("Metering mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsMeteringMode)),
        TagInfo(0x0012, "FocusType", N_("Focus Type"), N_("Focus type setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusType)),
        TagInfo(0x0013, "AFPoint", N_("AF Point"), N_("AF point selected"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsAfPoint)),
        TagInfo(0x0014, "ExposureProgram", N_("Exposure Program"), N_("Exposure mode setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsExposureProgram)),
        TagInfo(0x0015, "0x0015", "0x0015", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0016, "LensType", N_("Lens Type"), N_("Lens type"), canonCsId, makerTags, signedShort, 1, printCsLensType),
        TagInfo(0x0017, "Lens", N_("Lens"), N_("'long' and 'short' focal length of lens (in 'focal units') and 'focal units' per mm"), canonCsId, makerTags, unsignedShort, 3, printCsLens),
        TagInfo(0x0018, "ShortFocal", N_("Short Focal"), N_("Short focal"), canonCsId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0019, "FocalUnits", N_("Focal Units"), N_("Focal units"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x001a, "MaxAperture", N_("Max Aperture"), N_("Max aperture"), canonCsId, makerTags, signedShort, 1, printSi0x0015),
        TagInfo(0x001b, "MinAperture", N_("Min Aperture"), N_("Min aperture"), canonCsId, makerTags, signedShort, 1, printSi0x0015),
        TagInfo(0x001c, "FlashActivity", N_("Flash Activity"), N_("Flash activity"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFlashActivity)),
        TagInfo(0x001d, "FlashDetails", N_("Flash Details"), N_("Flash details"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG_BITMASK(canonCsFlashDetails)),
        TagInfo(0x001e, "0x001e", "0x001e", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x001f, "0x001f", "0x001f", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0020, "FocusContinuous", N_("Focus Continuous"), N_("Focus continuous setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsFocusContinuous)),
        TagInfo(0x0021, "AESetting", N_("AESetting"), N_("AE setting"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsAESetting)),
        TagInfo(0x0022, "ImageStabilization", N_("Image Stabilization"), N_("Image stabilization"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsImageStabilization)),
        TagInfo(0x0023, "DisplayAperture", N_("Display Aperture"), N_("Display aperture"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0024, "ZoomSourceWidth", N_("Zoom Source Width"), N_("Zoom source width"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0025, "ZoomTargetWidth", N_("Zoom Target Width"), N_("Zoom target width"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0026, "0x0026", "0x0026", N_("Unknown"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0027, "SpotMeteringMode", N_("Spot Metering Mode"), N_("Spot metering mode"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSpotMeteringMode)),
        TagInfo(0x0028, "PhotoEffect", N_("Photo Effect"), N_("Photo effect"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsPhotoEffect)),
        TagInfo(0x0029, "ManualFlashOutput", N_("Manual Flash Output"), N_("Manual flash output"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)),
        TagInfo(0x002a, "ColorTone", N_("Color Tone"), N_("Color tone"), canonCsId, makerTags, signedShort, 1, printValue),
        TagInfo(0x002e, "SRAWQuality", N_("SRAW Quality Tone"), N_("SRAW quality"), canonCsId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsSRAWQuality)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", N_("Unknown Canon Camera Settings 1 tag"), canonCsId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListCs()
    {
        return tagInfoCs_;
    }

    //! WhiteBalance, multiple tags
    extern const TagDetails canonSiWhiteBalance[] = {
        {  0, N_("Auto")                        },
        {  1, N_("Daylight")                    },
        {  2, N_("Cloudy")                      },
        {  3, N_("Tungsten")                    },
        {  4, N_("Fluorescent")                 },
        {  5, N_("Flash")                       },
        {  6, N_("Custom")                      },
        {  7, N_("Black & White")               },
        {  8, N_("Shade")                       },
        {  9, N_("Manual Temperature (Kelvin)") },
        { 10, N_("PC Set 1")                    },
        { 11, N_("PC Set 2")                    },
        { 12, N_("PC Set 3")                    },
        { 14, N_("Daylight Fluorescent")        },
        { 15, N_("Custom 1")                    },
        { 16, N_("Custom 2")                    },
        { 17, N_("Underwater")                  },
        { 18, N_("Custom 3")                    },
        { 19, N_("Custom 3")                    },
        { 20, N_("PC Set 4")                    },
        { 21, N_("PC Set 5")                    },
        { 23, N_("Auto (ambience priority)")    }
    };

    //! AFPointUsed, tag 0x000e
    extern const TagDetailsBitmask canonSiAFPointUsed[] = {
        { 0x0004, N_("left")   },
        { 0x0002, N_("center") },
        { 0x0001, N_("right")  }
    };

    //! FlashBias, tag 0x000f
    extern const TagDetails canonSiFlashBias[] = {
        { 0xffc0, "-2 EV"    },
        { 0xffcc, "-1.67 EV" },
        { 0xffd0, "-1.50 EV" },
        { 0xffd4, "-1.33 EV" },
        { 0xffe0, "-1 EV"    },
        { 0xffec, "-0.67 EV" },
        { 0xfff0, "-0.50 EV" },
        { 0xfff4, "-0.33 EV" },
        { 0x0000, "0 EV"     },
        { 0x000c, "0.33 EV"  },
        { 0x0010, "0.50 EV"  },
        { 0x0014, "0.67 EV"  },
        { 0x0020, "1 EV"     },
        { 0x002c, "1.33 EV"  },
        { 0x0030, "1.50 EV"  },
        { 0x0034, "1.67 EV"  },
        { 0x0040, "2 EV"     }
    };

    // Canon Shot Info Tag
    const TagInfo CanonMakerNote::tagInfoSi_[] = {
        TagInfo(0x0001, "0x0001", "0x0001", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed Used"), N_("ISO speed used"), canonSiId, makerTags, unsignedShort, 1, printSi0x0002),
        TagInfo(0x0003, "MeasuredEV", N_("Measured EV"), N_("Measured EV"), canonSiId, makerTags, unsignedShort, 1, printSi0x0003),
        TagInfo(0x0004, "TargetAperture", N_("Target Aperture"), N_("Target Aperture"), canonSiId, makerTags, unsignedShort, 1, printSi0x0015),
        TagInfo(0x0005, "TargetShutterSpeed", N_("Target Shutter Speed"), N_("Target shutter speed"), canonSiId, makerTags, unsignedShort, 1, printSi0x0016),
        TagInfo(0x0006, "0x0006", "0x0006", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "WhiteBalance", N_("White Balance"), N_("White balance setting"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)),
        TagInfo(0x0008, "0x0008", "0x0008", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0009, "Sequence", N_("Sequence"), N_("Sequence number (if in a continuous burst)"), canonSiId, makerTags, unsignedShort, 1, printSi0x0009),
        TagInfo(0x000a, "0x000a", "0x000a", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000b, "0x000b", "0x000b", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000c, "0x000c", "0x000c", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000e, "AFPointUsed", N_("AF Point Used"), N_("AF point used"), canonSiId, makerTags, unsignedShort, 1, printSi0x000e),
        TagInfo(0x000f, "FlashBias", N_("Flash Bias"), N_("Flash bias"), canonSiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonSiFlashBias)),
        TagInfo(0x0010, "0x0010", "0x0010", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0011, "0x0011", "0x0011", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0012, "0x0012", "0x0012", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0013, "SubjectDistance", N_("Subject Distance"), N_("Subject distance (units are not clear)"), canonSiId, makerTags, unsignedShort, 1, printSi0x0013),
        TagInfo(0x0014, "0x0014", "0x0014", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0015, "ApertureValue", N_("Aperture Value"), N_("Aperture"), canonSiId, makerTags, unsignedShort, 1, printSi0x0015),
        TagInfo(0x0016, "ShutterSpeedValue", N_("Shutter Speed Value"), N_("Shutter speed"), canonSiId, makerTags, unsignedShort, 1, printSi0x0016),
        TagInfo(0x0017, "MeasuredEV2", N_("Measured EV 2"), N_("Measured EV 2"), canonSiId, makerTags, unsignedShort, 1, printSi0x0017),
        TagInfo(0x0018, "0x0018", "0x0018", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", N_("Unknown"), canonSiId, makerTags, unsignedShort, 1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonSiTag)", "(UnknownCanonSiTag)", N_("Unknown Canon Camera Settings 2 tag"), canonSiId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListSi()
    {
        return tagInfoSi_;
    }

    //! PanoramaDirection, tag 0x0005
    extern const TagDetails canonPaDirection[] = {
        { 0, N_("Left to right")          },
        { 1, N_("Right to left")          },
        { 2, N_("Bottom to top")          },
        { 3, N_("Top to bottom")          },
        { 4, N_("2x2 matrix (Clockwise)") }
    };

    // Canon Panorama Info
    const TagInfo CanonMakerNote::tagInfoPa_[] = {
        TagInfo(0x0002, "PanoramaFrame", N_("Panorama Frame"), N_("Panorama frame number"), canonPaId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "PanoramaDirection", N_("Panorama Direction"), N_("Panorama direction"), canonPaId, makerTags, unsignedShort, 1, EXV_PRINT_TAG(canonPaDirection)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCs2Tag)", "(UnknownCanonCs2Tag)", N_("Unknown Canon Panorama tag"), canonPaId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPa()
    {
        return tagInfoPa_;
    }

    // Canon Custom Function Tag Info
    const TagInfo CanonMakerNote::tagInfoCf_[] = {
        TagInfo(0x0001, "NoiseReduction", N_("Noise Reduction"), N_("Long exposure noise reduction"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0002, "ShutterAeLock", N_("Shutter Ae Lock"), N_("Shutter/AE lock buttons"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0003, "MirrorLockup", N_("Mirror Lockup"), N_("Mirror lockup"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0004, "ExposureLevelIncrements", N_("Exposure Level Increments"), N_("Tv/Av and exposure level"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "AFAssist", N_("AF Assist"), N_("AF assist light"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0006, "FlashSyncSpeedAv", N_("Flash Sync Speed Av"), N_("Shutter speed in Av mode"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0007, "AEBSequence", N_("AEB Sequence"), N_("AEB sequence/auto cancellation"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0008, "ShutterCurtainSync", N_("Shutter Curtain Sync"), N_("Shutter curtain sync"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0009, "LensAFStopButton", N_("Lens AF Stop Button"), N_("Lens AF stop button Fn. Switch"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000a, "FillFlashAutoReduction", N_("Fill Flash Auto Reduction"), N_("Auto reduction of fill flash"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000b, "MenuButtonReturn", N_("Menu Button Return"), N_("Menu button return position"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000c, "SetButtonFunction", N_("Set Button Function"), N_("SET button func. when shooting"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000d, "SensorCleaning", N_("Sensor Cleaning"), N_("Sensor cleaning"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000e, "SuperimposedDisplay", N_("Superimposed Display"), N_("Superimposed display"), canonCfId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x000f, "ShutterReleaseNoCFCard", N_("Shutter Release No CF Card"), N_("Shutter Release W/O CF Card"), canonCfId, makerTags, unsignedShort, 1, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", N_("Unknown Canon Custom Function tag"), canonCfId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListCf()
    {
        return tagInfoCf_;
    }

    //! AFPointsUsed, tag 0x0016
    extern const TagDetailsBitmask canonPiAFPointsUsed[] = {
        { 0x01, N_("right")     },
        { 0x02, N_("mid-right") },
        { 0x04, N_("bottom")    },
        { 0x08, N_("center")    },
        { 0x10, N_("top")       },
        { 0x20, N_("mid-left")  },
        { 0x40, N_("left")      }
    };

    //! AFPointsUsed20D, tag 0x001a
    extern const TagDetailsBitmask canonPiAFPointsUsed20D[] = {
        { 0x001, N_("top")         },
        { 0x002, N_("upper-left")  },
        { 0x004, N_("upper-right") },
        { 0x008, N_("left")        },
        { 0x010, N_("center")      },
        { 0x020, N_("right")       },
        { 0x040, N_("lower-left")  },
        { 0x080, N_("lower-right") },
        { 0x100, N_("bottom")      }
    };

    // Canon Picture Info Tag
    const TagInfo CanonMakerNote::tagInfoPi_[] = {
        TagInfo(0x0002, "ImageWidth", N_("Image Width"), N_("Image width"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0003, "ImageHeight", N_("Image Height"), N_("Image height"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0004, "ImageWidthAsShot", N_("Image Width As Shot"), N_("Image width (as shot)"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0005, "ImageHeightAsShot", N_("Image Height As Shot"), N_("Image height (as shot)"), canonPiId, makerTags, unsignedShort, 1, printValue),
        TagInfo(0x0016, "AFPointsUsed", N_("AF Points Used"), N_("AF points used"), canonPiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed)),
        TagInfo(0x001a, "AFPointsUsed20D", N_("AF Points Used 20D"), N_("AF points used (20D)"), canonPiId, makerTags, unsignedShort, 1, EXV_PRINT_TAG_BITMASK(canonPiAFPointsUsed20D)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPiTag)", "(UnknownCanonPiTag)", N_("Unknown Canon Picture Info tag"), canonPiId, makerTags, unsignedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPi()
    {
        return tagInfoPi_;
    }

    //! BracketMode, tag 0x0003
    extern const TagDetails canonBracketMode[] = {
        { 0, N_("Off") },
        { 1, N_("AEB") },
        { 2, N_("FEB") },
        { 3, N_("ISO") },
        { 4, N_("WB")  }
    };

    //! RawJpgSize, tag 0x0007
    extern const TagDetails canonRawJpgSize[] = {
        { 0,   N_("Large")        },
        { 1,   N_("Medium")       },
        { 2,   N_("Small")        },
        { 5,   N_("Medium 1")     },
        { 6,   N_("Medium 2")     },
        { 7,   N_("Medium 3")     },
        { 8,   N_("Postcard")     },
        { 9,   N_("Widescreen")   },
        { 129, N_("Medium Movie") },
        { 130, N_("Small Movie")  }
    };

    //! NoiseReduction, tag 0x0008
    extern const TagDetails canonNoiseReduction[] = {
        { 0, N_("Off")  },
        { 1, N_("On 1") },
        { 2, N_("On 2") },
        { 3, N_("On")   },
        { 4, N_("Auto") }
    };

    //! WBBracketMode, tag 0x0009
    extern const TagDetails canonWBBracketMode[] = {
        { 0, N_("Off")           },
        { 1, N_("On (shift AB)") },
        { 2, N_("On (shift GM)") }
    };

    //! FilterEffect, tag 0x000e
    extern const TagDetails canonFilterEffect[] = {
        { 0, N_("None")   },
        { 1, N_("Yellow") },
        { 2, N_("Orange") },
        { 3, N_("Red")    },
        { 4, N_("Green")  }
    };

    //! ToningEffect, tag 0x000e
    extern const TagDetails canonToningEffect[] = {
        { 0, N_("None")   },
        { 1, N_("Sepia")  },
        { 2, N_("Blue")   },
        { 3, N_("Purple") },
        { 4, N_("Green")  }
    };

    // Canon File Info Tag
    const TagInfo CanonMakerNote::tagInfoFi_[] = {
        TagInfo(0x0001, "FileNumber", N_("File Number"), N_("File Number"), canonFiId, makerTags, unsignedLong, 1, printFiFileNumber),
        TagInfo(0x0003, "BracketMode", N_("Bracket Mode"), N_("Bracket Mode"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonBracketMode)),
        TagInfo(0x0004, "BracketValue", N_("Bracket Value"), N_("Bracket Value"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0005, "BracketShotNumber", N_("Bracket Shot Number"), N_("Bracket Shot Number"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0006, "RawJpgQuality", N_("Raw Jpg Quality"), N_("Raw Jpg Quality"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonCsQuality)),
        TagInfo(0x0007, "RawJpgSize", N_("Raw Jpg Size"), N_("Raw Jpg Size"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonRawJpgSize)),
        TagInfo(0x0008, "NoiseReduction", N_("Noise Reduction"), N_("Noise Reduction"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonNoiseReduction)),
        TagInfo(0x0009, "WBBracketMode", N_("WB Bracket Mode"), N_("WB Bracket Mode"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonWBBracketMode)),
        TagInfo(0x000c, "WBBracketValueAB", N_("WB Bracket Value AB"), N_("WB Bracket Value AB"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000d, "WBBracketValueGM", N_("WB Bracket Value GM"), N_("WB Bracket Value GM"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000e, "FilterEffect", N_("Filter Effect"), N_("Filter Effect"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonFilterEffect)),
        TagInfo(0x000f, "ToningEffect", N_("Toning Effect"), N_("Toning Effect"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonToningEffect)),
        TagInfo(0x0010, "MacroMagnification", N_("Macro Magnification"), N_("Macro magnification"), canonFiId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0013, "LiveViewShooting", N_("Live View Shooting"), N_("Live view shooting"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)),
        TagInfo(0x0014, "FocusDistanceUpper", N_("Focus Distance Upper"), N_("Focus Distance Upper"), canonFiId, makerTags, signedShort, 1, printFiFocusDistance),
        TagInfo(0x0015, "FocusDistanceLower", N_("Focus Distance Lower"), N_("Focus Distance Lower"), canonFiId, makerTags, signedShort, 1, printFiFocusDistance),
        TagInfo(0x0019, "FlashExposureLock", N_("Flash Exposure Lock"), N_("Flash exposure lock"), canonFiId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonOffOn)),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonFiTag)", "(UnknownCanonFiTag)", N_("Unknown Canon File Info tag"), canonFiId, makerTags, signedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListFi()
    {
        return tagInfoFi_;
    }

    //! Tone Curve Values
    extern const TagDetails canonToneCurve[] = {
        { 0, N_("Standard") },
	{ 1, N_("Manual")   },
	{ 2, N_("Custom")   }
    };

    //! Sharpness Frequency Values
    extern const TagDetails canonSharpnessFrequency[] = {
        { 0, N_("n/a")      },
        { 1, N_("Lowest")   },
        { 2, N_("Low")      },
        { 3, N_("Standard") },
        { 4, N_("High")     },
        { 5, N_("Highest")  }
    };

    //! PictureStyle Values
    extern const TagDetails canonPictureStyle[] = {
        { 0x00, N_("None")            },
        { 0x01, N_("Standard")        },
        { 0x02, N_("Portrait")        },
        { 0x03, N_("High Saturation") },
        { 0x04, N_("Adobe RGB")       },
        { 0x05, N_("Low Saturation")  },
        { 0x06, N_("CM Set 1")        },
        { 0x07, N_("CM Set 2")        },
        { 0x21, N_("User Def. 1")     },
        { 0x22, N_("User Def. 2")     },
        { 0x23, N_("User Def. 3")     },
        { 0x41, N_("PC 1")            },
        { 0x42, N_("PC 2")            },
        { 0x43, N_("PC 3")            },
        { 0x81, N_("Standard")        },
        { 0x82, N_("Portrait")        },
        { 0x83, N_("Landscape")       },
        { 0x84, N_("Neutral")         },
        { 0x85, N_("Faithful")        },
        { 0x86, N_("Monochrome")      },
        { 0x87, N_("Auto")            },
        { 0x88, N_("Fine Detail")     }
    };

    // Canon Processing Info Tag
    const TagInfo CanonMakerNote::tagInfoPr_[] = {
        TagInfo(0x0001, "ToneCurve", N_("ToneCurve"), N_("Tone curve"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonToneCurve)),
        TagInfo(0x0002, "Sharpness", N_("Sharpness"), N_("Sharpness"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0003, "SharpnessFrequency", N_("SharpnessFrequency"), N_("Sharpness frequency"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonSharpnessFrequency)),
        TagInfo(0x0004, "SensorRedLevel", N_("SensorRedLevel"), N_("Sensor red level"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0005, "SensorBlueLevel", N_("SensorBlueLevel"), N_("Sensor blue level"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0006, "WhiteBalanceRed", N_("WhiteBalanceRed"), N_("White balance red"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0007, "WhiteBalanceBlue", N_("WhiteBalanceBlue"), N_("White balance blue"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x0008, "WhiteBalance", N_("WhiteBalance"), N_("White balance"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonSiWhiteBalance)),
        TagInfo(0x0009, "ColorTemperature", N_("ColorTemperature"), N_("Color Temperature"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000a, "PictureStyle", N_("PictureStyle"), N_("Picture style"), canonPrId, makerTags, signedShort, 1, EXV_PRINT_TAG(canonPictureStyle)),
        TagInfo(0x000b, "DigitalGain", N_("DigitalGain"), N_("Digital gain"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000c, "WBShiftAB", N_("WBShiftAB"), N_("WBShift AB"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0x000d, "WBShiftGM", N_("WBShiftGM"), N_("WB Shift GM"), canonPrId, makerTags, signedShort, 1, printValue),
        TagInfo(0xffff, "(UnknownCanonPrTag)", "(UnknownCanonPrTag)", N_("Unknown Canon Processing Info tag"), canonPrId, makerTags, signedShort, 1, printValue)
    };

    const TagInfo* CanonMakerNote::tagListPr()
    {
        return tagInfoPr_;
    }

    std::ostream& CanonMakerNote::printFiFileNumber(std::ostream& os,
                                                    const Value& value,
                                                    const ExifData* metadata)
    {
        std::ios::fmtflags f( os.flags() );
        if (   !metadata || value.typeId() != unsignedLong
            || value.count() == 0)
        {
            os << "(" << value << ")";
            os.flags(f);
            return os;
        }

        ExifData::const_iterator pos = metadata->findKey(ExifKey("Exif.Image.Model"));
        if (pos == metadata->end()) return os << "(" << value << ")";

        // Ported from Exiftool
        std::string model = pos->toString();
        if (   model.find("20D") != std::string::npos
            || model.find("350D") != std::string::npos
            || model.substr(model.size() - 8, 8) == "REBEL XT"
            || model.find("Kiss Digital N") != std::string::npos) {
            uint32_t val = value.toLong();
            uint32_t dn = (val & 0xffc0) >> 6;
            uint32_t fn = ((val >> 16) & 0xff) + ((val & 0x3f) << 8);
            os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
            os.flags(f);
            return os;
        }
        if (   model.find("30D") != std::string::npos
            || model.find("400D") != std::string::npos
            || model.find("REBEL XTi") != std::string::npos
            || model.find("Kiss Digital X") != std::string::npos
            || model.find("K236") != std::string::npos) {
            uint32_t val = value.toLong();
            uint32_t dn = (val & 0xffc00) >> 10;
            while (dn < 100) dn += 0x40;
            uint32_t fn = ((val & 0x3ff) << 4) + ((val >> 20) & 0x0f);
            os << std::dec << dn << "-" << std::setw(4) << std::setfill('0') << fn;
            os.flags(f);
            return os;
        }

        os.flags(f);
        return os << "(" << value << ")";
    }

    std::ostream& CanonMakerNote::printFocalLength(std::ostream& os,
                                                   const Value& value,
                                                   const ExifData* metadata)
    {
        std::ios::fmtflags f( os.flags() );
        if (   !metadata
            || value.count() < 4
            || value.typeId() != unsignedShort) {
            os.flags(f);
            return os << value;
        }

        ExifKey key("Exif.CanonCs.Lens");
        ExifData::const_iterator pos = metadata->findKey(key);
        if (   pos != metadata->end()
            && pos->value().count() >= 3
            && pos->value().typeId() == unsignedShort) {
            float fu = pos->value().toFloat(2);
            if (fu != 0.0) {
                float fl = value.toFloat(1) / fu;
                std::ostringstream oss;
                oss.copyfmt(os);
                os << std::fixed << std::setprecision(1);
                os << fl << " mm";
                os.copyfmt(oss);
                os.flags(f);
                return os;
            }
        }

        os.flags(f);
        return os << value;
    }

    std::ostream& CanonMakerNote::print0x0008(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::string n = value.toString();
        if (n.length() < 4) return os << "(" << n << ")";
        return os << n.substr(0, n.length() - 4) << "-"
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::istringstream is(value.toString());
        uint32_t l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::printCs0x0002(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        if (l == 0) {
            os << "Off";
        }
        else {
            os << l / 10.0 << " s";
        }
        return os;
    }

    //! Helper structure
    struct LensTypeAndFocalLengthAndMaxAperture {
        long        lensType_;                  //!< Lens type
        std::string focalLength_;               //!< Focal length
        std::string maxAperture_;               //!< Aperture
    };

    //! Compare tag details with a lens entry
    bool operator==(const TagDetails& td, const LensTypeAndFocalLengthAndMaxAperture& ltfl) {
        return (   td.val_ == ltfl.lensType_
                && std::string(td.label_).find(ltfl.focalLength_) != std::string::npos
                && std::string(td.label_).find(ltfl.maxAperture_) != std::string::npos);
    }

    void extractLensFocalLength(LensTypeAndFocalLengthAndMaxAperture& ltfl,
                                const ExifData* metadata)
    {
        ExifKey key("Exif.CanonCs.Lens");
        ExifData::const_iterator pos = metadata->findKey(key);
        if (   pos != metadata->end()
            && pos->value().count() >= 3
            && pos->value().typeId() == unsignedShort) {
            float fu = pos->value().toFloat(2);
            if (fu != 0.0) {
                float len1 = pos->value().toLong(0) / fu;
                float len2 = pos->value().toLong(1) / fu;
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(0);
                if (len1 == len2) {
                    oss << len1 << "mm";
                } else {
                    oss << len2 << "-" << len1 << "mm";
                }
                ltfl.focalLength_ = oss.str();
            }
        }
    }

    std::ostream& printCsLensByFocalLengthAndMaxAperture(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        LensTypeAndFocalLengthAndMaxAperture ltfl;
        ltfl.lensType_ = value.toLong();

        extractLensFocalLength(ltfl, metadata);
        if (ltfl.focalLength_.empty()) return os << value;

        ExifKey key("Exif.CanonCs.MaxAperture");
        ExifData::const_iterator pos = metadata->findKey(key);
        if (   pos != metadata->end()
            && pos->value().count() == 1
            && pos->value().typeId() == unsignedShort) {

            long val = static_cast<int16_t>(pos->value().toLong(0));
            if (val > 0)
            {
                std::ostringstream oss;
                oss << std::setprecision(2);
                oss << fnumber(canonEv(val));
                ltfl.maxAperture_ = oss.str();
            }
        }
        if (ltfl.maxAperture_.empty()) return os << value;

        const TagDetails* td = find(canonCsLensType, ltfl);
        if (!td) return os << value;
        return os << td->label_;
    }

    std::ostream& printCsLensByFocalLength(std::ostream& os,
                                           const Value& value,
                                           const ExifData* metadata)
    {
        if (   !metadata || value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        LensTypeAndFocalLengthAndMaxAperture ltfl;
        ltfl.lensType_ = value.toLong();

        extractLensFocalLength(ltfl, metadata);
        if (ltfl.focalLength_.empty()) return os << value;

        const TagDetails* td = find(canonCsLensType, ltfl);
        if (!td) return os << value;
        return os << td->label_;
    }

    std::ostream& CanonMakerNote::printCsLensType(std::ostream& os,
                                                  const Value& value,
                                                  const ExifData* metadata)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << "(" << value << ")";

        const LensIdFct* lif = find(lensIdFct, value.toLong());
        if (!lif) {
            return EXV_PRINT_TAG(canonCsLensType)(os, value, metadata);
        }
        if (metadata && lif->fct_) {
            return lif->fct_(os, value, metadata);
        }
        return os << value;
    }

    std::ostream& CanonMakerNote::printCsLens(std::ostream& os,
                                              const Value& value,
                                              const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );

        if (   value.count() < 3
            || value.typeId() != unsignedShort) {
            os << "(" << value << ")";
            os.flags(f);
            return os;
        }

        float fu = value.toFloat(2);
        if (fu == 0.0) return os << value;
        float len1 = value.toLong(0) / fu;
        float len2 = value.toLong(1) / fu;
        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(1);
        if (len1 == len2) {
            os << len1 << " mm";
        } else {
            os << len2 << " - " << len1 << " mm";
        }
        os.copyfmt(oss);
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0002(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            // Ported from Exiftool by Will Stokes
            os << exp(canonEv(value.toLong()) * log(2.0)) * 100.0 / 32.0;
        }
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0003(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() == unsignedShort
            && value.count() > 0) {
            // The offset of '5' seems to be ok for most Canons (see Exiftool)
            // It might be explained by the fakt, that most Canons have a longest
            // exposure of 30s which is 5 EV below 1s
            // see also printSi0x0017
            std::ostringstream oss;
            oss.copyfmt(os);
            int res = static_cast<int>(100.0 * (static_cast<short>(value.toLong()) / 32.0 + 5.0) + 0.5);
            os << std::fixed << std::setprecision(2) << res / 100.0;
            os.copyfmt(oss);
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0009(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x000e(std::ostream& os,
                                                const Value& value,
                                                const ExifData* pExifData)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        long num = (l & 0xf000) >> 12;
        os << num << " focus points; ";
        long used = l & 0x0fff;
        if (used == 0) {
            os << "none";
        }
        else {
            EXV_PRINT_TAG_BITMASK(canonSiAFPointUsed)(os, value, pExifData);
        }
        os << " used";
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0013(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        long l = value.toLong();
        if (l == 0xffff) {
            os << "Infinite";
        }
        else {
            os << l << "";
        }
        os.flags(f);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0015(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        long val = static_cast<int16_t>(value.toLong());
        if (val < 0) return os << value;
        os << std::setprecision(2)
           << "F" << fnumber(canonEv(val));
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0016(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        std::ios::fmtflags f( os.flags() );
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        URational ur = exposureTime(canonEv(value.toLong()));
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        os.flags(f);
        return os << " s";
    }

    std::ostream& CanonMakerNote::printSi0x0017(std::ostream& os,
                                                const Value& value,
                                                const ExifData*)
    {
        if (   value.typeId() != unsignedShort
            || value.count() == 0) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::fixed << std::setprecision(2)
           << value.toLong() / 8.0 - 6.0;
        os.copyfmt(oss);
        return os;
    }

    std::ostream& CanonMakerNote::printFiFocusDistance(std::ostream& os,
                                                       const Value& value,
                                                       const ExifData*)
    {
       std::ios::fmtflags f( os.flags() );
       if (   value.typeId() != signedShort
         || value.count() == 0) return os << value;

      std::ostringstream oss;
      oss.copyfmt(os);
      os << std::fixed << std::setprecision(2);

      long l = value.toLong();
      if (l == 0xffff) {
        os << "Infinite";
      }
      else {
        os << value.toLong()/100.0 << " m";
      }

      os.copyfmt(oss);
      os.flags(f);
      return os;
    }

// *****************************************************************************
// free functions

    float canonEv(long val)
    {
        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        float frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0f / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0f / 3;
        }
        return sign * (val + frac) / 32.0f;
    }

}}                                      // namespace Internal, Exiv2
