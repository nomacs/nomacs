#!/usr/bin/env python3
# encoding: utf-8
"""
This script exports matplotlib colormaps to xml files.
The exported colormaps are suitable for using them in nomacs.

# c.f. http://matplotlib.org/examples/color/colormaps_reference.html
"""
import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors
import matplotlib.cm as cm
import PIL

# these color maps will be exported.
EXPORTED_CMAPS = [
    'viridis', 'plasma', 'magma', 'inferno',                     #perceptually uniform colormaps
#    'Blues', 'Greens', 'Reds', 'GnBu',                           #sequential colormaps
#    'PiYG', 'RdGy', 'RdBu', 'Spectral', 'coolwarm',              #divergent colormaps
#    'gist_earth', 'terrain', 'hsv', 'nipy_spectral', #misc colormaps
#    'jet', 'terrain', 'gnuplot2', #misc colormaps
]

# 'jet', 'hot', 'autumn' # Qualitative, and Segmented colormaps not supported yet.

def write_steps_xml(filename, steps, name=None, colorspace='RGB'):
    """ writes colomap steps to an xml file """
    if name is None:
        name = filename[:-4] #use file basename

    #convert to xml element
    out = [ '<ColorMap name="%s" space="%s">'%(name, colorspace)]
    #print('steps0:', steps[0])
    for pos, rgb_values  in steps:
        r, g, b = rgb_values
        out += ['  <Point x="%f" r="%f" g="%f" b="%f"/>'%(pos, r, g, b)]
    out += [ '</ColorMap>']

    #write to file
    print("- exporting colormap '%s'"%(name))
    with open(filename, "w", encoding='utf-8') as xml_file:
        for line in out:
            xml_file.write(line + "\n");

def export_colormap(cmap_name):
    try:
        cmap = None
        cmap_filename = cmap_name + ".xml"
        try:
            cmap = cm.get_cmap(cmap_name)
        except AttributeError as exc:
            print("- failed to export colormap '%s': not in your matplotlib. Consult matplotlib.cm' "%cmap_name)
            return

        #sample colormaps at 256 positions from 0.0 to 1.0

        steps = []
        if isinstance(cmap, matplotlib.colors.LinearSegmentedColormap):
            rgba_map = cm.ScalarMappable(cmap=cmap_name)
            positions = np.linspace(0.0, 1.0, 256)

            for pos, r, g, b in rgba_map.to_rgba(positions):
                steps += [(pos, (r,g,b))]
        elif isinstance(cmap, matplotlib.colors.ListedColormap):
            #sample lienar colormaps at 256 points
            for idx, rgb_values  in enumerate(cmap.colors):
                #print('pos:', idx, 'rgb', rgb_values)
                rgb = np.array(rgb_values, dtype=np.float)
                r, g, b = rgb
                pos = float(idx) / len(cmap.colors)
                steps += [(pos, (r,g,b))]
        else:
            raise NotImplementedError("unsupported colormap type: %s"%type(cmap))

        write_steps_xml(cmap_filename, steps, name=cmap_name, colorspace="RGB")
    except Exception as exc:
        print("- failed to export colormap '%s': %s %s"%(cmap_name, type(exc), exc))
        #raise exc


if __name__ == '__main__':
    try:
        for colormap in EXPORTED_CMAPS:
            export_colormap(colormap)
        sys.exit(0)
    except Exception as exc:
        sys.exit(0)
    except Exception as exc:
        print("failed to export all colormaps: ", exc)
    sys.exit(1)
