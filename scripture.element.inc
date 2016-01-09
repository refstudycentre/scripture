<?php

function scripture_verse_picker_getstyle($input){
  $style = 1;  //default
  if (array_key_exists('#settings', $input)) {
	$settings = $input['#settings'];
	if (array_key_exists('style', $settings)) {
		$style = $settings['style'];
	} else {
		$style=1;
	}
  }
  return $style;
}


/**
 * Renders the inside of the element
 * DO NOT call drupal_render($variables['element']) (i.e. on the entire element), as this will cause recursion.
 */
function theme_scripture_verse_picker($variables) {
  $output = "";
  $e = &$variables['element'];
  
  $style = scripture_verse_picker_getstyle($e);  //default
  
  if ($style==2) drupal_add_js(drupal_get_path('module', 'scripture').'/scripture.pickerstyle2.js');
  
  # generic outer wrapper to distinguish the different styles
  $output .= "<div class='verse-picker-style-{$style}'>";
  
//   $output .= drupal_render($e['preview']);
  $t = drupal_render($e['translation']);
  $b = drupal_render($e['book']);
  $fc = drupal_render($e['from_chapter']);
  $fv = drupal_render($e['from_verse']);
  $tc = drupal_render($e['to_chapter']);
  $tv = drupal_render($e['to_verse']);
  if ($style==2) $maxdata = drupal_render($e['maxdata']);
  
  # Translation 
  switch ($style) {
    case 1 :  
	  $output .= "<div class='scripture-translation'>{$t}</div>";
	  break;
    case 2 : 
	  $output .= "<div class='scripture-translation element-hidden'>{$t}</div>
	              <div class='scripture-translation-alt'>{$e['translation']['#default_value']}</div>
				  <div class='scripture-translation-selector element-hidden'><ul>";
	  foreach ($e['translation']['#options'] as $key => $value) {
		$output .= "<li><div class='transabbr'>$key</div><div class='transdesc'>$value</div></li>";
	  }
	  $output .= "</ul></div>";
	  break;
	default :
	  drupal_set_message('Unknown style {$style} selected, not implemented in Translation part', 'error');
  }

  # Book-name  
  switch ($style) {
    case 1 :  
	  $output .= "<div class='scripture-book'>{$b}</div>";
	  break;
    case 2 : 
	  $output .= "<div class='scripture-book element-hidden'>{$b}</div>
	              <div class='scripture-book-alt'>{$e['book']['#default_value']}</div>
				  <div class='scripture-book-selector element-hidden'></div>{$maxdata}";
	  break;
	default :
	  drupal_set_message('Unknown style {$style} selected, not implemented in Book part', 'error');
  }

  # Chapter and Verse
  $output .= "<div class='scripture-verse'>{$fc} : ${fv}";
  
  if (strlen($tv)) {
    if (strlen($tc)) {
      $output .= " - {$tc} : {$tv}";
    } else {
      $output .= " - {$tv}";
    }
  }
  
  $output .= "</div></div>";
  
  return $output;
}

/**
 * Render inline wrappers form elements
 */
function theme_scripture_inline_wrapper($var) {
  $tooltip = $var['element']['#title'];
  if (!empty($var['element']['#description'])) {
    $tooltip .= ": " . $var['element']['#description'];
  }
  return "<span title='{$tooltip}'>{$var['element']['#children']}</span>";
}

function scripture_ajax_getmaxdata($form, &$form_state) {
  $wrap = $form_state['#book-verse-max']; 
  $trans = $form['verse']['#value']['translation'];
  $book = $form['verse']['#value']['book'];

  $rst = db_query("SELECT chapternum, max(versenum + 0) as MaxVers
				   FROM lw_verses
				   WHERE translation=:trans and booknum=:book
				   GROUP BY chapternum", array(
				     ':trans' => $trans, 
				     ':book' => $book));
  $content = ''; //$trans."x".$book;
  foreach ($rst as $row) {
		$content .= "<div class='scripture-chapter-max-verse' chapter='{$row->chapternum}'>{$row->MaxVers}</div>";
  }
  return "<div id='{$wrap}' class='element-hidden'>{$content}</div>";
}

function scripture_ajax_translation_changed_multiple($form, &$form_state) {
  $style = $form_state['#style'];
  if (empty($style)) $style = 1;
  $wrapb = '#'.$form_state['#book-wrapper'];
  $wrapm = '#'.$form_state['#book-verse-max']; 
  
  $commands = array();
  $commands[] = ajax_command_replace($wrapb, render(scripture_ajax_translation_changed($form, $form_state)), array());
  // Invoke any JavaScript func
  if ($style==2) { 
	$commands[] = ajax_command_invoke(NULL, 'afterajaxbookrefresh', array($wrapb));
    $commands[] = ajax_command_replace($wrapm, scripture_ajax_getmaxdata($form, $form_state), array());
    $commands[] = ajax_command_invoke(NULL, 'afterajaxmaxrefresh', array($wrapm));
  }
  return array('#type' => 'ajax', '#commands' => $commands);
}


function scripture_ajax_book_changed_multiple($form, &$form_state) {
  $wrap = '#'.$form_state['#book-verse-max']; 

  $commands = array();
  $commands[] = ajax_command_replace($wrap, scripture_ajax_getmaxdata($form, $form_state), array());
  $commands[] = ajax_command_invoke(NULL, 'afterajaxmaxrefresh', array($wrap));
  return array('#type' => 'ajax', '#commands' => $commands);
}


/**
 * gets called when verse picker element is used in a form
 */
function scripture_verse_picker_process($element, &$form_state, $complete_form) {
  
  $style = scripture_verse_picker_getstyle($element);  //default
  
  $form_state['#style'] = $style;   //copy style into form_state, for ajax processing
  
  // Element settings
  $settings = empty($element['#scripture_settings']) ? array() : $element['#scripture_settings'];
  $settings += array(
    'allow_translation'   => TRUE,  // translation cannot be edited if FALSE
    'allow_book'          => TRUE,  // book cannot be edited if FALSE
    'allow_range'         => TRUE,  // only a single verse may be selected if FALSE
  );

  // Default values to populate element
  $init = empty($element['#value']) ? array() : $element['#value'];
  $init += empty($element['#default_value']) ? array() : $element['#default_value'];
  $init += array(
    'translation' => NULL,
    'book' => NULL,
    'from_chapter' => '',
    'from_verse' => '',
    'to_chapter' => '',
    'to_verse' => '',
  );

  // Options for select lists
  $translations = scripture_get_translations();
  foreach ($translations as $id => &$t) {
	if ($style==2) $t = "{$t->lang} / {$t->name}";
	else           $t = "{$t->abbr} / {$t->lang} / {$t->name}";
  }
  if (!isset($translations[$init['translation']])) {
    $init['translation'] = NULL;
  }
  $books = scripture_get_books($init['translation']);
  foreach ($books as $booknum => &$b) {
    if ($style==2) $b = "{$b->bookname}";
	else           $b = "{$booknum}. {$b->bookname}";
  }
  if (!isset($books[$init['book']])) {
    $init['book'] = NULL;
  }

  // TODO: http://drupal.stackexchange.com/questions/162582/making-a-custom-form-element-ajax-aware

  // The compound form element:
  
  $element['#tree'] = TRUE;
  
  //only call this the first time...  the "process" hook is called for each ajax call, and the ajax id should be constant over the ajax calls
  if (empty($form_state['#book-wrapper'])) {
	  $form_state['#book-wrapper'] = drupal_html_id('scripture_verse_picker_book');
	  if ($style==2) $form_state['#book-verse-max'] = drupal_html_id('scripture_verse_picker_maxdata');
  }

  $element['translation'] = array(
    '#type' => 'select',
    '#title' => t('Translation:'),
    '#title_display' => 'before',
    '#ajax' => array(
	 /* old method, but single statement possible only */
 	 // 'wrapper'  => $form_state['#book-wrapper'],
     // 'callback' => 'scripture_ajax_translation_changed',
     // 'method'   => 'replace',
	  'callback' => 'scripture_ajax_translation_changed_multiple',
	  'wrapper' => 'wrapper',
    ),
    '#disabled' => !$settings['allow_translation'],
  );
  
  // Translation is always required for the verse range to even make sense
  $element['translation']['#required'] = TRUE;
  $element['translation']['#options'] = $translations;
  $element['translation']['#default_value'] = empty($init['translation']) ? variable_get('scripture_default_translation', NULL) : $init['translation'];

  $element['book'] = array(
    '#prefix'        => "<div id='{$form_state['#book-wrapper']}'>",
    '#suffix'        => "</div>",
    '#type'          => 'select',
    '#title'         => t('Book:'),
    '#options'       => $books,
    '#required'      => false,
    '#default_value' => $init['book'],
    '#disabled'      => !$settings['allow_book'],
  );
  
  if ($style==2) {
	  /* we need the number of chapters and verse - and we put this in an ajax call here */
	  $element['book']['#ajax'] = array(
		'callback' => 'scripture_ajax_book_changed_multiple',
		'wrapper' => 'wrapper',
	  ); 
	  //explicitly add the book-verse-max id in here.
	  $element['maxdata'] = array(
								'#markup' => "<div id='{$form_state['#book-verse-max']}' class='scripture-max-data'>hierso kom die maxdata</div>",
							);
  }
  
  
  $element['from_chapter'] = array(
    '#type'           => 'textfield',
    '#size'           => 3,
    '#maxlength'      => 3,
    '#required'       => false,
    '#default_value'  => $init['from_chapter'],
    '#title_display'  => 'invisible',
    '#theme_wrappers' => array('scripture_inline_wrapper'),
  );
  
  $element['from_verse'] = array(
    '#type'           => 'textfield',
    '#size'           => 3,
    '#maxlength'      => 3,
    '#required'       => false,
    '#default_value'  => $init['from_verse'],
    '#title_display'  => 'invisible',
    '#theme_wrappers' => array('scripture_inline_wrapper'),
  );
  
  $element['to_chapter'] = array(
    '#type'           => 'textfield',
    '#title'          => t('To chapter'),
    '#description'    => t('Last chapter in the range. Leave blank to use the "from chapter" value.'),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#required'       => false,
    '#default_value'  => $init['to_chapter'],
    '#access'         => $settings['allow_range'],
    '#title_display'  => 'invisible',
    '#theme_wrappers' => array('scripture_inline_wrapper'),
  );
  
  $element['to_verse'] = array(
    '#type'           => 'textfield',
    '#title'          => t('To verse'),
    '#description'    => t('Last verse in the range. Leave this and "To chapter" blank to use the "from verse" value.'),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#required'       => false,
    '#default_value'  => $init['to_verse'],
    '#access'         => $settings['allow_range'],
    '#title_display'  => 'invisible',
    '#theme_wrappers' => array('scripture_inline_wrapper'),
  );
    
  if ($settings['allow_range']) {
    $element['from_chapter']['#title'] = t('From chapter');
    $element['from_chapter']['#description'] = t('First chapter in the range.');
    $element['from_verse']['#title'] = t('From verse');
    $element['from_verse']['#description'] = t('First verse in the range.');
  } else {
    $element['from_chapter']['#title'] = t('Chapter');
    $element['from_verse']['#title'] = t('Verse');
  }

  return $element;

}

/**
 * Modify user input
 * http://drupal.stackexchange.com/questions/125386/how-does-value-callback-really-work-nested-elements
 *
 * value_callback provides a way to process values before they are returned to the form submit function.
 * You need to provide a #value_callback if you want to do fancy processing on the input.
 * If you just want #value set to #default_value, you don't need this hook at all, since that will happen anyway.
 * This function is mainly useful if you need to do something non-standard with the value, like transpose an array into an option list.
 *
 * Value callbacks should consider three conditions:
 * - Input is being provided directly. This happens when a form is submitted.
 * - No input is provided, but the field definition has a default value.
 * - No input is provided and there is no default value.
 */
function form_type_scripture_verse_picker_value($element, $input = FALSE, &$form_state) {
  
  if ($input === FALSE) {
    
    $input = isset($element['#default_value']) ? $element['#default_value'] : array();
    
  } else {
    
    if (empty($input['to_chapter'])) {
  
      // use the from_chapter value if to_chapter was empty
      $input['to_chapter'] = $input['from_chapter'];
  
      if (empty($input['to_verse'])) {
        // use the from_verse value if both to_chapter and to_verse were empty
        $input['to_verse'] = $input['from_verse'];
      }
  
    }
    
  }
  
  return $input;

}

/**
 * Validate user input
 * Checks the verse ranges logically
 * Checks if verses exist in database
 * Loads vids from database
 */
function scripture_verse_picker_validate($element, &$form_state) {

  // if the element is empty and not required, then all is well
  if (scripture_widget_is_empty($element) && $element['#required'] == FALSE) {
    return $element;
  } // else further inspection is required
  
  $v = &$element['#value'];
  $errors = FALSE;

  // check that a starting verse and chapter is selected
  if (empty($v['from_chapter'])) {
    form_error($element, t('Specify a starting chapter.'));
    $errors = TRUE;
  }
  if (empty($v['from_verse'])) {
    form_error($element, t('Specify a starting verse.'));
    $errors = TRUE;
  }
  
  // check that verse accompanies chapter
  if (!empty($v['to_chapter']) && empty($v['to_verse'])) {
    form_error($element, t('You filled in a chapter, but not a verse.'));
    $errors = TRUE;
  }
  
  if (empty($v['to_chapter'])) {
  
    // use the from_chapter value if to_chapter was empty
    $v['to_chapter'] = $v['from_chapter'];
  
    if (empty($v['to_verse'])) {
      // use the from_verse value if both to_chapter and to_verse were empty
      $v['to_verse'] = $v['from_verse'];
    }
  
  }
  
  // check that the verse range is positive
  if ($v['from_chapter'] > $v['to_chapter'] || ($v['from_chapter'] == $v['to_chapter'] && $v['from_verse'] > $v['to_verse'])) {
    form_error($element, t('First verse and chapter must be before the last verse and chapter.'));
    $errors = TRUE;
  }
  
  // if there are no other errors, check if the verses exist by requesting their vids
  // need to check for non-empty parameters, because the "required" validation criteria is not checked on individual elements when this is called with AJAX
  if (!$errors && !empty($v['from_chapter']) && !empty($v['to_chapter']) && !empty($v['from_verse']) && !empty($v['to_verse'])) {
  
    // get the verse ids using the above information
    $v['from_vid'] = scripture_get_vid($v['translation'], $v['book'], $v['from_chapter'], $v['from_verse']);
    $v['to_vid'] = scripture_get_vid($v['translation'], $v['book'], $v['to_chapter'], $v['to_verse']);
    
    // check if the verses exist, i.e. have vids
    if (!($v['from_vid'] && $v['to_vid'])) {
      // at least one verse does not exist in the range
      form_error($element, t('Requested verse does not exist.'));
      $errors = TRUE;
    }
    
  }
  
  // update the form state with the new values
  form_set_value($element, $v, $form_state);
  
  return $element;
}


/**
 * Check if element is empty
 */
function scripture_widget_is_empty($element) {
  return
    empty($element['#value']['from_chapter']) &&
    empty($element['#value']['from_verse']) &&
    empty($element['#value']['to_chapter']) &&
    empty($element['#value']['to_verse']);
}


/**
 * React to user changing the translation
 * Update list of books
 *
 * Check this out: https://www.drupal.org/node/279246#comment-2661792 ...
 */
function scripture_ajax_translation_changed($form, $form_state) {
  $array_parents = $form_state['triggering_element']['#array_parents'];
  array_pop($array_parents); // go one level up from the triggering element
  $scripture_verse_picker_element = drupal_array_get_nested_value($form, $array_parents);
  return $scripture_verse_picker_element['book'];
}
