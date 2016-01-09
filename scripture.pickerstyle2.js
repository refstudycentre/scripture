var selectedBookNr; 

var expandedWindow;


function ScriptureShowPopup(tag) {
    //first close previous unclosed popup windows
	if (!(typeof expandedWindow === 'undefined' || expandedWindow === null)) {
		expandedWindow.hide();
	}
	var selector = tag.className.replace("-alt", "-selector").replace("scripture",".scripture");
	// console.log(selector);
	//find the new window, and display this....
	expandedWindow = jQuery(tag).parent().find(selector);
	expandedWindow.show();  
}

function ScriptureCloseAndDestroy(){
	if (expandedWindow) {
		expandedWindow.hide();
		delete window.expandedWindow;
	}
}

function ScriptureSelectTranslation(tag) {
	var vp = expandedWindow.parent();
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

function ScriptureSelectChapter(tag) {
	var vp = jQuery(tag).parents(".verse-picker-style-2");
	ScriptureCloseAndDestroy();

	alert(tag);
}


function ScriptureSelectVerse(tag) {
	var vp = jQuery(tag).parents(".verse-picker-style-2");
	ScriptureCloseAndDestroy();

	alert(tag);
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

	jQuery(".scripture-verse-selector li").click(function(e) {
		ScriptureSelectVerse(this);
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
	var vp = jQuery(maxid).parents(".verse-picker-style-2");   //find the triggering verse-picker 
	//alert("freshed max");
};
