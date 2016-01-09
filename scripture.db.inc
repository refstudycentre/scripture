<?php

/**
 * Get all known translations
 * @return array:
 *   An array like follows:
 *   array(
 *     id => object(
 *       abbr => 'translation abbreviation',
 *       name => 'translation name',
 *       lang => 'langcode',
 *     ),
 *     ...,
 *   )
 */
function scripture_get_translations() {
  return db_select('lw_translations', 't')->fields('t')->execute()->fetchAllAssoc('abbr');
}

/**
 * Get the info for a translation based on translation id
 */
function scripture_get_translation($abbr) {
  return db_select('lw_translations', 't')->fields('t')->condition('abbr',$abbr)->execute()->fetchAssoc();
}

/**
 * Retrieve verse from the db based on vid
 */
function scripture_get_verse($vid, $translation = NULL) {

  // Try to load the default translation if it is not specified
  if (empty($translation)) {
    $translation = variable_get('scripture_default_translation', NULL);
  }

  // Check if a translation is specified
  if (empty($translation)) {

    drupal_set_message("The default translation is not specified. Don't know which text to load from database.", 'error', FALSE);
    return FALSE;

  } else {
    return db_select('lw_verses', 'v')
      ->fields('v')
      ->condition('vid', $vid)
      ->condition('translation', $translation)
      ->range(0, 1)
      ->execute()
      ->fetchAssoc();
  }

}

/**
 * Retrieve all verses in a range from the db based on vid
 */
function scripture_get_verses_between($from_vid, $to_vid, $translation = NULL) {

  // Try to load the default translation if it is not specified
  if (empty($translation)) {
    $translation = variable_get('scripture_default_translation', NULL);
  }

  // Check if a translation is specified
  if (empty($translation)) {

    drupal_set_message("The default translation is not specified. Don't know which text to load from database.", 'error', FALSE);
    return FALSE;

  } else {

    return db_select('lw_verses', 'v')
      ->fields('v')
      ->condition('translation', $translation)
      ->condition('vid', array($from_vid, $to_vid), 'BETWEEN')
      ->orderBy('vid', 'ASC')// ordering by versenum does not work, because there are alphanumeric verses in some translations
      ->execute()
      ->fetchAllAssoc('vid');

  }

}

/**
 * Retrieve matching vid from the db
 */
function scripture_get_vid($translation, $booknum, $chapternum, $versenum) {
  if (empty($translation))
    $translation = variable_get('scripture_default_translation', NULL);
  
  // If translation is set, this function can be used to check if that verse exists in a specific translation.
  return db_select('lw_verses', 'v')
    ->fields('v', array('vid'))
    ->condition('translation', $translation)
    ->condition('booknum', $booknum)
    ->condition('chapternum', $chapternum)
    ->condition('versenum', $versenum)
    ->range(0, 1)
    ->execute()
    ->fetchField();
}

/**
 * Get all book names, keyed by booknumber, for a specific translation
 */
function scripture_get_books($translation = NULL) {
  if (empty($translation))
    $translation = variable_get('scripture_default_translation', NULL);
  
  if (!empty($translation))
    return db_select('lw_books', 'b')->fields('b')->condition('translation', $translation)->execute()->fetchAllAssoc('booknum');
  else
    return array();
}

/**
 * Get book which contains given verse
 */
function scripture_get_book($verse) {
  if (empty($verse['translation']))
    $translation = variable_get('scripture_default_translation', NULL);
  else
    $translation = $verse['translation'];
  
  if (!empty($translation))
    return db_select('lw_books', 'b')->fields('b')->condition('booknum', $verse['booknum'])->condition('translation', $translation)->range(0, 1)->execute()->fetchAssoc();
  else
    return array();
}


/**
 * Get nodes which reference the given verse range
 */
function scripture_get_nodes($from_vid, $to_vid = NULL) {

  if (empty($to_vid)) {

    // single verse
    $to_vid = $from_vid;

  } else {

    // verse range
    if ($to_vid < $from_vid) {

      // error: negative verse range...
      return FALSE;

    }

  }

  // TODO: we need some way to quickly inspect all fields of type "Scripture reference"

//    $query = new EntityFieldQuery();
//    $query
//      ->entityCondition('entity_type', 'node')
//      ->propertyConfition('status', NODE_PUBLISHED)
//      ->fieldConfition('')

}

