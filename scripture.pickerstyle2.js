var selectedBookNr; 

var expandedWindow;


function ScriptureShowPopup(tag) {
    //first close previous unclosed popup windows
	if (!(typeof expandedWindow === 'undefined' || expandedWindow === null)) {
		expandedWindow.hide();
	}
	//find the new window, and display this....
	expandedWindow = jQuery(tag).parent().find(".scripture-translation-selector");
	expandedWindow.show();  
}

function ScriptureCloseAndDestroy(){
	expandedWindow.hide();
	delete window.expandedWindow;
}

function ScriptureSelectTranslation(tag) {
	var vp = expandedWindow.parent();
	var trans = tag.children[0].innerText;
	ScriptureCloseAndDestroy();
	
	//console.debug(trans);
	/* get the right verse-picker */
	vp.find(".scripture-translation-alt").text(trans);
	var sel = vp.find(".scripture-translation select");
	sel.val(trans)
	sel.trigger("change");  //init the ajax ?
}

/* This function is called after an AJAX update of the book list, which is triggered by selecting a different translation. */

function ScriptureBookSelector_AfterRefresh()
{
	//remember the previously selected book (numeric - 1..66)
	selectedBookNr = jQuery("#scripture-book select").val();
	//console.log(selectedBookNr);
	
	//find the new book name, for the newly selected translation
	var found = jQuery("#scripture-book-selector li").filter(function(){
	  //console.log(this.children[0].innerText == selectedBookNr);
	  return (this.children[0].innerText == selectedBookNr);
	});
	//console.log(found);
	//set the book name based on the new translations book names
	jQuery("scripture-book-alt").text(found[0].children[1].innerText);
	jQuery(".scripture-book-selector li").click(function() {
		jQuery(".scripture-book-selector").hide();
		jQuery(".scripture-book-alt").text(this.children[1].innerText);
		jQuery(".scripture-book select").val(this.children[0].innerText);
	});
}



/* The init for the first page load */

jQuery(document).ready(function(){
	jQuery(".scripture-translation-alt").click(function (e) {
		ScriptureShowPopup(this);
		e.stopPropagation();
	});
	
	jQuery(".scripture-translation-selector li").click(function(e) {
		ScriptureSelectTranslation(this);
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
});
