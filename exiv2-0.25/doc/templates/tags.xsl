<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" />

<!-- *********************************************************************** -->
<xsl:template match="TAGLIST">

<!-- content generated from XML -->
<xsl:call-template name="report-table" />

</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="HEADER/text">
  <xsl:copy-of select="text()|*" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header">
  <h2><xsl:value-of select="HEADER/title" /></h2>
  <xsl:apply-templates select="HEADER/text" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="report-table">
  <table class="ReportTable" id="report1" cellspacing="0">
    <xsl:call-template name="column-group" />
    <thead>
      <xsl:call-template name="header-row" />
    </thead>
    <tbody>
      <xsl:apply-templates select="ROWSET/ROW" />
    </tbody>
  </table>
  <xsl:call-template name="interactivity" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="column-group">
  <colgroup>
    <col align="center" />
    <col align="right" />
    <col />
    <col />
    <col />
    <col />
  </colgroup>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header-row">
  <tr>
    <th>Tag (hex)</th>
    <th>Tag (dec)</th>
    <th>IFD</th>
    <th>Key</th>
    <th>Type</th>
    <th>Tag description</th>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="ROWSET/ROW[position() mod 2 = 0]">
  <xsl:call-template name="data-row">
    <xsl:with-param name="rowClass" select="'EvenRow'" />
  </xsl:call-template>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="ROWSET/ROW[position() mod 2 = 1]">
  <xsl:call-template name="data-row">
    <xsl:with-param name="rowClass" select="'OddRow'" />
  </xsl:call-template>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="data-row">
  <xsl:param name="rowClass" />
  <tr><xsl:attribute name="class"><xsl:value-of select="$rowClass" /></xsl:attribute>
    <td><xsl:value-of select="taghex" /></td>
    <td><xsl:value-of select="tagdec" /></td>
    <td><xsl:value-of select="ifd" /></td>
    <td><xsl:value-of select="key" /></td>
    <td><xsl:value-of select="type" /></td>
    <td><xsl:value-of select="tagdesc" /></td>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="interactivity">
  <script type="text/javascript">
  //<![CDATA[
  var report1 = new SortableTable(
    document.getElementById("report1"),
    ["String", "Number", "String", "String", "String", "String"]
  );
  report1.onsort = function () { 
	var rows = report1.tBody.rows;
	var l = rows.length;
	for (var i = 0; i < l; i++) {
		removeClassName(rows[i], i % 2 ? "OddRow" : "EvenRow");
		addClassName(rows[i], i % 2 ? "EvenRow" : "OddRow");
	}
  };
  //]]>
</script>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
