################################################################################
# File     : iptc.awk
# Version  : $Rev: 570 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History  : 07-Feb-04, ahu: created
#
# Description:
#  Awk script to convert a taglist to XML format used in the documentation.
#  $ taglist [SectionName] | awk -f iptc.awk > iptc.xml
################################################################################

BEGIN {
   FS = ", "
   print "<?xml version = '1.0'?>";   
   print "<?xml-stylesheet type=\"text/xsl\" href=\"iptc.xsl\"?>";

   print "<TAGLIST>"
   print "<HEADER>"
   print "<title>Iptc datasets defined in Exiv2</title>"
   print "<text>"
   print "<p>Datasets are defined according to the specification of the Iptc "
   print "<a href=\"http://www.iptc.org/IIM/\">Information Interchange Model (IIM)</a>.</p>"
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
   print "      <recname>" $4 "</recname>"
   print "      <mandatory>" $5 "</mandatory>"
   print "      <repeatable>" $6 "</repeatable>"
   print "      <minbytes>" $7 "</minbytes>"
   print "      <maxbytes>" $8 "</maxbytes>"
   print "      <key>" $9 "</key>"
   print "      <type>" $10 "</type>"
   print "      <tagdesc>" $11 "</tagdesc>"
   print "   </ROW>";
}

END {
   print "</ROWSET>"   
   print "</TAGLIST>"
}
