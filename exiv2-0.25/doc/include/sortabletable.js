/*----------------------------------------------------------------------------\
|                            Sortable Table 1.03                              |
|-----------------------------------------------------------------------------|
|                         Created by Erik Arvidsson                           |
|                  (http://webfx.eae.net/contact.html#erik)                   |
|                      For WebFX (http://webfx.eae.net/)                      |
|-----------------------------------------------------------------------------|
| A DOM 1 based script that allows an ordinary HTML table to be sortable.     |
|-----------------------------------------------------------------------------|
|                  Copyright (c) 1998 - 2002 Erik Arvidsson                   |
|-----------------------------------------------------------------------------|
| This software is provided "as is", without warranty of any kind, express or |
| implied, including  but not limited  to the warranties of  merchantability, |
| fitness for a particular purpose and noninfringement. In no event shall the |
| authors or  copyright  holders be  liable for any claim,  damages or  other |
| liability, whether  in an  action of  contract, tort  or otherwise, arising |
| from,  out of  or in  connection with  the software or  the  use  or  other |
| dealings in the software.                                                   |
| - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |
| This  software is  available under the  three different licenses  mentioned |
| below.  To use this software you must chose, and qualify, for one of those. |
| - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |
| The WebFX Non-Commercial License          http://webfx.eae.net/license.html |
| Permits  anyone the right to use the  software in a  non-commercial context |
| free of charge.                                                             |
| - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |
| The WebFX Commercial license           http://webfx.eae.net/commercial.html |
| Permits the  license holder the right to use  the software in a  commercial |
| context. Such license must be specifically obtained, however it's valid for |
| any number of  implementations of the licensed software.                    |
| - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |
| GPL - The GNU General Public License    http://www.gnu.org/licenses/gpl.txt |
| Permits anyone the right to use and modify the software without limitations |
| as long as proper  credits are given  and the original  and modified source |
| code are included. Requires  that the final product, software derivate from |
| the original  source or any  software  utilizing a GPL  component, such  as |
| this, is also licensed under the GPL license.                               |
|-----------------------------------------------------------------------------|
| 2003-01-10 | First version                                                  |
| 2003-01-19 | Minor changes to the date parsing                              |
| 2003-01-28 | JScript 5.0 fixes (no support for 'in' operator)               |
| 2003-02-01 | Sloppy typo like error fixed in getInnerText                   |
|-----------------------------------------------------------------------------|
| Created 2003-01-10 | All changes are in the log above. | Updated 2003-02-01 |
\----------------------------------------------------------------------------*/

/*
   Changes made (ahu):
   + Support header TH element (headerOnclick)
   + Added support for a TD attribute 'sortValue' (getRowValue): 
     If it is there, use its value for sorting instead of the cell content. 
     This allows sorting by a value that differs from what is seen on the 
     screen. As a simple example, consider sorting a column containing
     formatted numbers: <TD sortValue="1000">1,000.00</TD>

   + 15-May 2004, ahu: changed default sort order (on the first click) to
     ascending (SortableTable.prototype.sort)

   ToDo: 
   - Allow for different Date formats
*/

function SortableTable(oTable, oSortTypes) {

	this.element = oTable;
	this.tHead = oTable.tHead;
	this.tBody = oTable.tBodies[0];
	this.document = oTable.ownerDocument || oTable.document;
	
	this.sortColumn = null;
	this.descending = null;
	
	var oThis = this;
	this._headerOnclick = function (e) {
		oThis.headerOnclick(e);
	};
	
	
	// only IE needs this
	var win = this.document.defaultView || this.document.parentWindow;
	this._onunload = function () {
		oThis.destroy();
	};
	if (win && typeof win.attachEvent != "undefined") {
		win.attachEvent("onunload", this._onunload);
	}
	
	this.initHeader(oSortTypes || []);
}

SortableTable.gecko = navigator.product == "Gecko";
SortableTable.msie = /msie/i.test(navigator.userAgent);
// Mozilla is faster when doing the DOM manipulations on
// an orphaned element. MSIE is not
SortableTable.removeBeforeSort = SortableTable.gecko;

SortableTable.prototype.onsort = function () {};

// adds arrow containers and events
// also binds sort type to the header cells so that reordering columns does
// not break the sort types
SortableTable.prototype.initHeader = function (oSortTypes) {
	var cells = this.tHead.rows[0].cells;
	var l = cells.length;
	var img, c;
	for (var i = 0; i < l; i++) {
		c = cells[i];
		img = this.document.createElement("IMG");
		img.src = "../include/blank.png"; // relative to the file that uses this
		c.appendChild(img);
		if (oSortTypes[i] != null) {
			c._sortType = oSortTypes[i];
		}
		if (typeof c.addEventListener != "undefined")
			c.addEventListener("click", this._headerOnclick, false);
		else if (typeof c.attachEvent != "undefined")		
			c.attachEvent("onclick", this._headerOnclick);
	}
	this.updateHeaderArrows();
};

// remove arrows and events
SortableTable.prototype.uninitHeader = function () {
	var cells = this.tHead.rows[0].cells;
	var l = cells.length;
	var c;
	for (var i = 0; i < l; i++) {
		c = cells[i];
		c.removeChild(c.lastChild);
		if (typeof c.removeEventListener != "undefined")
			c.removeEventListener("click", this._headerOnclick, false);
		else if (typeof c.detachEvent != "undefined")
			c.detachEvent("onclick", this._headerOnclick);
	}
};

SortableTable.prototype.updateHeaderArrows = function () {
	var cells = this.tHead.rows[0].cells;
	var l = cells.length;
	var img;
	for (var i = 0; i < l; i++) {
		img = cells[i].lastChild;
		if (i == this.sortColumn)
			img.className = "SortArrow " + (this.descending ? "descending" : "ascending");
		else
			img.className = "SortArrow";			
	}
};

SortableTable.prototype.headerOnclick = function (e) {
	// find TD or TH element
	var el = e.target || e.srcElement;
	while (el.tagName != "TD" && el.tagName != "TH")
		el = el.parentNode;
	
	this.sort(el.cellIndex);	
};

SortableTable.prototype.getSortType = function (nColumn) {
	var cell = this.tHead.rows[0].cells[nColumn];
	var val = cell._sortType;
	if (val != "")
		return val;
	return "String";
};

// only nColumn is required
// if bDescending is left out the old value is taken into account
// if sSortType is left out the sort type is found from the sortTypes array

SortableTable.prototype.sort = function (nColumn, bDescending, sSortType) {
	if (sSortType == null)
		sSortType = this.getSortType(nColumn);

	// exit if None	
	if (sSortType == "None")
		return;
	
	if (bDescending == null) {
		if (this.sortColumn != nColumn)
                        // 15-May 2004, ahu: change to false for ascending
			this.descending = false;
		else
			this.descending = !this.descending;
	}	
	
	this.sortColumn = nColumn;
	
	if (typeof this.onbeforesort == "function")
		this.onbeforesort();
	
	var f = this.getSortFunction(sSortType, nColumn);
	var a = this.getCache(sSortType, nColumn);
	var tBody = this.tBody;
	
	a.sort(f);
	
	if (this.descending)
		a.reverse();
	
	if (SortableTable.removeBeforeSort) {
		// remove from doc
		var nextSibling = tBody.nextSibling;
		var p = tBody.parentNode;
		p.removeChild(tBody);
	}
	
	// insert in the new order
	var l = a.length;
	for (var i = 0; i < l; i++)
		tBody.appendChild(a[i].element);
	
	if (SortableTable.removeBeforeSort) {	
		// insert into doc
		p.insertBefore(tBody, nextSibling);
	}
	
	this.updateHeaderArrows();
	
	this.destroyCache(a);
	
	if (typeof this.onsort == "function")
		this.onsort();
};

SortableTable.prototype.asyncSort = function (nColumn, bDescending, sSortType) {
	var oThis = this;
	this._asyncsort = function () {
		oThis.sort(nColumn, bDescending, sSortType);
	};
	window.setTimeout(this._asyncsort, 1);	
};

SortableTable.prototype.getCache = function (sType, nColumn) {
	var rows = this.tBody.rows;
	var l = rows.length;
	var a = new Array(l);
	var r;
	for (var i = 0; i < l; i++) {
		r = rows[i];
		a[i] = {
			value:		this.getRowValue(r, sType, nColumn),
			element:	r
		};
	};
	return a;
};

SortableTable.prototype.destroyCache = function (oArray) {
	var l = oArray.length;
	for (var i = 0; i < l; i++) {
		oArray[i].value = null;
		oArray[i].element = null;
		oArray[i] = null;
	}
}

SortableTable.prototype.getRowValue = function (oRow, sType, nColumn) {
	var c = oRow.cells[nColumn];
        var s = c.getAttribute("sortValue");
        if (!s) {
		if (typeof c.innerText != "undefined")
			s = c.innerText;
		else
			s = SortableTable.getInnerText(c);
	}
	return this.getValueFromString(s, sType);
};

SortableTable.getInnerText = function (oNode) {
	var s = "";	
	var cs = oNode.childNodes;
	var l = cs.length;
	for (var i = 0; i < l; i++) {
		switch (cs[i].nodeType) {
			case 1: //ELEMENT_NODE
				s += SortableTable.getInnerText(cs[i]);
				break;
			case 3:	//TEXT_NODE
				s += cs[i].nodeValue;
				break;
		}
	}
	return s;
}

SortableTable.prototype.getValueFromString = function (sText, sType) {
	switch (sType) {
		case "Number":
			return Number(sText);
		case "CaseInsensitiveString":
			return sText.toUpperCase();
		case "Date":
			var parts = sText.split("-");
			var d = new Date(0);
			d.setFullYear(parts[0]);
			d.setDate(parts[2]);
			d.setMonth(parts[1] - 1);			
			return d.valueOf();		
	}
	return sText;
};

SortableTable.prototype.getSortFunction = function (sType, nColumn) {
	return function compare(n1, n2) {
		if (n1.value < n2.value)
			return -1;
		if (n2.value < n1.value)
			return 1;
		return 0;
	};
};

SortableTable.prototype.destroy = function () {
	this.uninitHeader();
	var win = this.document.parentWindow;
	if (win && typeof win.detachEvent != "undefined") {	// only IE needs this
		win.detachEvent("onunload", this._onunload);
	}	
	this._onunload = null;
	this.element = null;
	this.tHead = null;
	this.tBody = null;
	this.document = null;
	this._headerOnclick = null;
	this.sortTypes = null;
	this._asyncsort = null;
	this.onsort = null;
};

/* -------------------------------------------------------------------------- */

function addClassName(el, sClassName) {
	var s = el.className;
	var p = s.split(" ");
	var l = p.length;
	for (var i = 0; i < l; i++) {
		if (p[i] == sClassName)
			return;
	}
	p[p.length] = sClassName;
	el.className = p.join(" ");			
}

function removeClassName(el, sClassName) {
	var s = el.className;
	var p = s.split(" ");
	var np = [];
	var l = p.length;
	var j = 0;
	for (var i = 0; i < l; i++) {
		if (p[i] != sClassName)
			np[j++] = p[i];
	}
	el.className = np.join(" ");
}
