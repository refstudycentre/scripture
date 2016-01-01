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

function ScriptureSelectTranslation(tag) {
	expandedWindow.hide();
	var trans = tag.children[0].innerText;
	console.debug(trans);
	/* get the right verse-picker */
	var vp = expandedWindow.parent();
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
	jQuery(".scripture-translation-alt").click(function () {
		ScriptureShowPopup(this);
	});
	
	jQuery(".scripture-translation-selector li").click(function() {
		ScriptureSelectTranslation(this);
	});
});
