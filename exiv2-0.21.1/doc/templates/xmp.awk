################################################################################
# File     : xmp.awk
# Version  : $Rev: 1327 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History  : 23-Nov-07, ahu: created
#
# Description:
#  Awk script to convert an XMP property list to XML format used in the 
#  documentation.
#  $ taglist [xmpList] | awk -f xmp.awk > [xmpList].xml
################################################################################

BEGIN {
   FS = ",	"  # ,\t
   print "<?xml version = '1.0'?>";   
   print "<?xml-stylesheet type=\"text/xsl\" href=\"xmp.xsl\"?>";

   print "<TAGLIST>"
   print "<HEADER>"
   print "<title>XMP tags defined in Exiv2</title>"
   print "<text>"
   print "<p>Some description</p>"
   print "<p>Click on a column header to sort the table.</p>"
   print "</text>"
   print "</HEADER>"
   print "<ROWSET>"
}

{
   print "   <ROW num=\"" ++row "\">";
   print "      <tagname>" $1 "</tagname>"
   print "      <title>" $2 "</title>"
   print "      <xmpvaltype>" $3 "</xmpvaltype>"
   print "      <type>" $4 "</type>"
   print "      <category>" $5 "</category>"
   print "      <tagdesc>" $6 "</tagdesc>"
   print "   </ROW>";
}

END {
   print "</ROWSET>"   
   print "</TAGLIST>"
}
