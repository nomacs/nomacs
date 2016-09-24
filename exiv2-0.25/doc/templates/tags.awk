################################################################################
# File     : tags.awk
# Version  : $Rev: 785 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History  : 07-Feb-04, ahu: created
#
# Description:
#  Awk script to convert a taglist to XML format used in the documentation.
#  $ taglist [itemName] | awk -f tags.awk > tags.xml
################################################################################

BEGIN {
   FS = ",	"  # ,\t
   print "<?xml version = '1.0'?>";   
   print "<?xml-stylesheet type=\"text/xsl\" href=\"tags.xsl\"?>";

   print "<TAGLIST>"
   print "<HEADER>"
   print "<title>XYZ MakerNote Tags defined in Exiv2</title>"
   print "<text>"
   print "<p>Tags found in the MakerNote of images taken with XYZ cameras. These tags "
   print "are defined by Exiv2 in accordance with <a href=\"makernote.html#RX\">[X]</a>.</p>"
   print "<p>Click on a column header to sort the table.</p>"
   print "</text>"
   print "</HEADER>"
   print "<ROWSET>"
}

{
   print "   <ROW num=\"" ++row "\">";
   print "      <tagname>" $1 "</tagname>"
   print "      <tagdec>" $2 "</tagdec>"
   print "      <taghex>" $3 "</taghex>"
   print "      <ifd>" $4 "</ifd>"
   print "      <key>" $5 "</key>"
   print "      <type>" $6 "</type>"
   print "      <tagdesc>" $7 "</tagdesc>"
   print "   </ROW>";
}

END {
   print "</ROWSET>"   
   print "</TAGLIST>"
}
