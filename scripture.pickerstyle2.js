var selectedBookNr; 

var expandedWindow;


function ScriptureShowPopup(tag) {
    //first close previous unclosed popup windows
	if (!(typeof expandedWindow === 'undefined' || expandedWindow === null)) {
		expandedWindow.hide();
	}
	var selector = ".".concat(tag.className.replace("-alt", "-selector"));
	// console.log(selector);
	//find the new window, and display this....
	expandedWindow = jQuery(tag).parent().find(selector);
	if (selector==".scripture-chapter-selector") {
		var vp = jQuery(tag).parents(".verse-picker-style-2");
		SetChapterVersMode(vp, 1);
	}
	
	expandedWindow.show();  
}

function ScriptureCloseAndDestroy(){
	if (expandedWindow) {
		expandedWindow.hide();
		delete window.expandedWindow;
	}
}

function ScriptureSelectTranslation(tag) {
	var vp = expandedWindow.parents(".verse-picker-style-2");
	var trans = tag.children[0].innerText;
	ScriptureCloseAndDestroy();
	
	//console.debug(trans);
	/* get the right verse-picker */
	jQuery(".scripture-translation-alt", vp).text(trans);
	jQuery(".scripture-translation select", vp)
	  .val(trans)   //set the value of the select
	  .trigger("change");  //and trigger the ajax refresh ?
}


function ScriptureSelectBook(tag) {
	if (expandedWindow) {
		var vp = expandedWindow.parent();
	} else {
		var vp = jQuery(tag).parents(".verse-picker-style-2");
	}
	var bookname = tag.innerText;
	var booknr = jQuery(tag).attr("value");
	
	ScriptureCloseAndDestroy();
	jQuery(".scripture-book-alt", vp).text(bookname);
	jQuery(".scripture-book select", vp).val(booknr).trigger("change");

}

// var ScriptureVerseStartMouseDown;
var ScriptureVerses = [];  //javascript array for better handling of the verses

function ScriptureCacheVerseObjects(verstable)
{
	//save the <td>'s of the relevant verses into a local array
	var xx = jQuery("td", verstable);
    for (i=0; i<xx.length; i++) {
		versnr = xx[i].innerText;
		ScriptureVerses[versnr - 1] = jQuery(xx[i]);
	}
	
	//for (i=0; i<xx.length; i++) console.log(ScriptureVerses[i].html());
}

function ScriptureHighlightVersRange(vp, element1, element2)
{
	var v1 = parseInt(element1.innerText);
	var v2 = parseInt(element2.innerText);
	
	if (v1 > v2) {  //swop the variables around
		vx = v1;
		v1 = v2;
		v2 = vx;
	}
	//remove highlights - not sure if that will mess around with the screen - maybe
	ScriptureVerses[0].parents("table").find(".selected").removeClass("selected");

	for (i=v1; i<=v2; i++) {
		ScriptureVerses[i - 1].addClass("selected");
	}
	
	//console.log('xx: ',v1,' x ', v2);
	jQuery("#edit-verse-from-verse", vp).val(v1);   /* save this in the underlying fields */
	jQuery("#edit-verse-to-verse", vp).val(v2);   /* save this in the underlying fields */

	var label = jQuery("#edit-verse-from-chapter", vp).val().concat(":", v1);
	if (v2!=v1) label = label.concat("-", v2);
	jQuery(".scripture-chapter-alt", vp).text(label);
}

function ScriptureSelectChapter(tag) {
	var clicked = jQuery(tag);
	var vp = clicked.parents(".verse-picker-style-2");
	var chapternr = clicked.html();

	clicked.parents("table").find(".selected").removeClass("selected");  /* clear the previous selection */
	clicked.addClass("selected");
	jQuery("#edit-verse-from-chapter", vp).val(chapternr);
	jQuery(".scripture-chapter-alt", vp).text(chapternr);
	//alert(chapternr);
    
	verstable = jQuery(".scripture-verse-picker-verse-table", vp);
	verstable.html(ScriptureBuildVerseTable(vp, chapternr));
	ScriptureCacheVerseObjects(verstable);
	SetChapterVersMode(vp, 2);  //now verse mode
	
	//hook the verse-selector events (range-selectable)
	jQuery(".scripture-verse-picker-verse-table td", vp).bind('mousedown', function(e){
		ScriptureVerseStartMouseDown = e.currentTarget;
		ScriptureHighlightVersRange(vp, ScriptureVerseStartMouseDown, ScriptureVerseStartMouseDown);
		jQuery(".scripture-verse-picker-verse-table td", vp).bind('mousemove', function(e){
			/* do something like range-select setting of "selected" class */
			
			ScriptureHighlightVersRange(vp, ScriptureVerseStartMouseDown, e.currentTarget);
			e.stopPropagation();
		});

		jQuery(".scripture-verse-picker-verse-table td", vp).bind('mouseup',function(e){
			ScriptureHighlightVersRange(vp, ScriptureVerseStartMouseDown, e.currentTarget);
			ScriptureCloseAndDestroy();
			jQuery(".scripture-verse-picker-verse-table td", vp).unbind('mousemove')
		});
		e.stopPropagation();
	});
}

function ScriptureSelectVers(tag) {
	var clicked = jQuery(tag);
	var vp = clicked.parents(".verse-picker-style-2");
	var versnr  = clicked.html();

	/* mark the vers as "selected", for when we open this window again */
	clicked.parents("table").find(".selected").removeClass("selected");  /* clear the previous selection */
	clicked.addClass("selected");
	
	jQuery("#edit-verse-from-verse", vp).val(versnr);   /* save this in the underlying fields */
	/*ScriptureUpdateChapterAltLabel();*/
	var label = jQuery("#edit-verse-from-chapter", vp).val().concat(":", versnr);
	jQuery(".scripture-chapter-alt", vp).text(label);
	ScriptureCloseAndDestroy();
}


function ScriptureBuildChapterTable(maxdata) {
	var nchapters = maxdata.children().length;
	var output = "<table>";
	var maxrows = 10;
	var maxcols = Math.ceil(nchapters / maxrows);
	for (var row=1; row <= maxrows; row++) {
	    output = output.concat("<tr>");
		for (var col=1; col <= maxcols; col++) {
			var chap = (col-1)*maxrows + row;
			if (chap <= nchapters) {
				output = output.concat("<td>", chap, "</td>");
			}
		}
	    output = output.concat("</tr>");
	}
	output = output.concat("</table>");
	
	return output;
}

function ScriptureBuildVerseTable(vp, selectedchapter) {
	var maxdata = jQuery(".scripture-maxdata" ,vp);

	var found = maxdata.children().filter(function(){
	  //console.log(this);
	  //console.log(this.children[0].innerText == selectedBookNr);
	  return (jQuery(this).attr("chapter") == selectedchapter);
	});

	//console.log(found);
	//set the book name based on the new translations book names
	var nverses = 0;
	if (found.length > 0) {
		nverses = found.html();
	}

	var output = "<table>";
	var maxrows = 10;
	var maxcols = Math.ceil(nverses / maxrows);
	for (var row=1; row <= maxrows; row++) {
	    output = output.concat("<tr>");
		for (var col=1; col <= maxcols; col++) {
			var chap = (col-1)*maxrows + row;
			if (chap <= nverses) {
				output = output.concat("<td>", chap, "</td>");
			}
		}
	    output = output.concat("</tr>");
	}
	output = output.concat("</table>");
	
	return output;
}

function SetChapterVersMode(vp, mode) {
	var chap = jQuery(".scripture-verse-picker-chapter", vp);
	var vers = jQuery(".scripture-verse-picker-vers", vp);
	
	if (mode==1) {
		chap.addClass("activepane");
		vers.removeClass("activepane");

		// automatically select the only chapter if there is one only.
		var count = jQuery("td", chap);
		if (count.length==1) {
			ScriptureSelectChapter(count[0]);
		}
	} else {
		chap.removeClass("activepane");
		vers.addClass("activepane");
		/* scroll that the selection is visible */
		var offset = jQuery(".selected", chap).offset(); // Contains .top and .left
//Now animate the scroll-left CSS properties 
		chap.animate({
			scrollLeft: offset.left - 20
		}); 
	}

	
	
}



/* This function is called after an AJAX update of the book list, which is triggered by selecting a different translation. */

function ScriptureBookSelector_AfterRefresh()
{
	//first we have to re-create te selector 
	var rep = jQuery(".verse-picker-style-2 .scripture-book select")
          .clone()
          .wrap("<div></div>")
          .parent().html()
          .replace(/select/g,"ul")
          .replace(/option/g,"li");

	// get hold of the relevant verse-picker - maybe pass this as a parameter?
	var vp = jQuery(".verse-picker-style-2 .scripture-book select").parents(".verse-picker-style-2")
	vp.find(".scripture-book-selector").html(rep);
		  
	//remember the previously selected book (numeric - 1..66)
	selectedBookNr = jQuery(".scripture-book select", vp).val();
	//console.log(selectedBookNr);
	
	//find the new book name, for the newly selected translation
	var found = jQuery(".scripture-book-selector li", vp).filter(function(){
	  //console.log(this);
	  //console.log(this.children[0].innerText == selectedBookNr);
	  return (jQuery(this).attr("value") == selectedBookNr);
	});

	//console.log(found);
	//set the book name based on the new translations book names
	if (found.length > 0) {
		ScriptureSelectBook(found[0]);
		
	 	//restore the ajax on the replaced stuff
		jQuery(".scripture-book-selector li", vp).click(function(e) {
			ScriptureSelectBook(this);
			e.stopPropagation();
		});
	}
}






/* The init for the first page load */

jQuery(document).ready(function(){
	jQuery(".scripture-translation-alt, .scripture-book-alt, .scripture-chapter-alt, .scripture-verse-alt").click(function (e) {
		ScriptureShowPopup(this);
		e.stopPropagation();
	});
	
	jQuery(".scripture-translation-selector li").click(function(e) {
		ScriptureSelectTranslation(this);
		e.stopPropagation();
	});

	jQuery(".scripture-book-selector li").click(function(e) {
		ScriptureSelectBook(this);
		e.stopPropagation();
	});
	
	jQuery(".scripture-chapter-selector li").click(function(e) {
		ScriptureSelectChapter(this);
		e.stopPropagation();
	});

	jQuery(document).keyup(function(e) {
		if ((expandedWindow) && (e.keyCode == 27)) ScriptureCloseAndDestroy(); // esc pressed - cancel the window
	});	
	
	jQuery(document).click(function(e) {
	  if (expandedWindow) {
		ScriptureCloseAndDestroy();
	  	e.stopPropagation();
	  }
    })	
	
	//first page-load:  refresh the book-selector menu
	ScriptureBookSelector_AfterRefresh();
});

jQuery.fn.afterajaxbookrefresh = function(bookid) {
	var vp = jQuery(bookid).parents(".verse-picker-style-2");   //find the triggering verse-picker 
	ScriptureBookSelector_AfterRefresh();
};

jQuery.fn.afterajaxmaxrefresh = function(maxid) {
    var maxdata = jQuery(maxid);
	var vp = maxdata.parents(".verse-picker-style-2");   //find the triggering verse-picker 

	//Create the basic structure of the widget
    jQuery(".scripture-chapter-selector", vp).html("<div class='scripture-verse-pane scripture-verse-picker-chapter'><div class='scripture-verse-pane-heading'>Chapter</div><div class='scripture-verse-picker-chapter-table'></div></div><div class='mode element-hidden'>1</div><div class='scripture-verse-pane scripture-verse-picker-vers'><div class='scripture-verse-pane-heading'>Verse</div><div class='scripture-verse-picker-verse-table'></div></div>"); 

	//build the chapter-selector window
	jQuery(".scripture-verse-picker-chapter-table", vp).html(ScriptureBuildChapterTable(maxdata));
	SetChapterVersMode(vp, 1);
	
	//add the click-event handler to them
	jQuery(".scripture-verse-picker-chapter-table td", vp).click(function(e) {
		ScriptureSelectChapter(this);
		e.stopPropagation();
	});
	
//	alert(rep);

};
