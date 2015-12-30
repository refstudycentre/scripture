<?php

// Field API docs: https://api.drupal.org/api/drupal/modules%21field%21field.module/group/field/7
// Bird's eye view of creating a custom field type: http://clikfocus.com/blog/how-set-custom-field-type-using-drupal-7-fields-api

/*
 * FIELD TYPE
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/group/field_types/7
 */

/**
 * Implements hook_field_info().
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/function/hook_field_info/7
 */
function scripture_field_info() {
  $fields = array();
  
  $fields['scripture'] = array(
    'label' => t('Scripture reference'),
    'description' => t('A field that references a range of bible verses from a specific translation'),
    'default_widget' => 'scripture_picker',
    'default_formatter' => 'scripture_formatter_plain'
  );
  
  return $fields;
}

/**
 * Implements hook_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors).
 * https://api.drupal.org/api/drupal/modules!field!field.api.php/function/hook_field_validate/7
 * Validate this module's field data.
 * If there are validation problems, add to the $errors array (passed by reference). There is no return value.
 *
 */
function scripture_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  // required fields are already individually checked by FAPI
  // logical checks are now done on the element-level
  // checking for verse existence is now also done on verse picker element level
}

/**
 * Implements hook_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items).
 * https://api.drupal.org/api/drupal/modules!field!field.api.php/function/hook_field_presave/7
 * Define custom presave behavior for this module's field types.
 * Make changes or additions to field values by altering the $items parameter by reference. There is no return value.
 */
function scripture_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {

  foreach ($items as $delta => &$item) {
    $v = &$item['verse'];
    
    // flatten for database
    $item['translation'] = $v['translation'];
    $item['sid'] = $v['sid'];
    $item['from_vid'] = $v['from_vid'];
    $item['to_vid'] = $v['to_vid'];
  
  }
  
}

/*
 * FIELD FORMATTER
 * https://api.drupal.org/api/drupal/modules!field!field.api.php/group/field_formatter/7
 */

define('SCRIPTURE_SHOW_TEXT_NO', 0x0001);
define('SCRIPTURE_SHOW_TEXT_TRUNC', 0x0002);
define('SCRIPTURE_SHOW_TEXT_ALL', 0x0003);

/**
 * Implements hook_field_formatter_info()
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/function/hook_field_formatter_info/7
 */
function scripture_field_formatter_info() {
  $formatters = array();
  
  $formatters['scripture_formatter_plain'] = array(
    'label' => t('Plain-text scripture reference'),
    'description' => t('A plain-text formatter for scripture references'),
    'field types' => array(
      'scripture'
    ),
    'settings' => array(
      'show_text' => SCRIPTURE_SHOW_TEXT_NO,
      'show_ref' => 1
    )
  );
  
  return $formatters;
}

/**
 * Implements hook_field_formatter_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items, $displays)
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/function/hook_field_formatter_prepare_view/7
 * load multiple verses, single verse, or subverse from database, based on settings
 */
function scripture_field_formatter_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items, $displays) {
  
  foreach ($items as $eid => &$i) {
    $s = &$displays[$eid]['settings'];
    
    foreach ($i as $delta => &$val) {
      $val['translation'] = NULL; // This will cause the default translation to be loaded later
      scripture_prepare($val,$s);
    }
  
  }

}

/**
 * Implements hook_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display)
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/function/hook_field_formatter_view/7
 * build a renderable array to display the field value
 * Good reading on renderable arrays:
 * - https://www.drupal.org/node/930760
 * - http://cocoate.com/ddbook/scary-render-array
 * - http://drupal.stackexchange.com/questions/11438/how-to-nest-elements-in-a-render-array
 */
function scripture_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {

  $contents = array();
  $s = $display['settings'];
  
  foreach ($items as $delta => $i) {
    $contents[$delta] = scripture_view($i,$s);
  }
  
  return $contents;
}


/**
 * Implements hook_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state).
 * https://api.drupal.org/api/drupal/modules!field_ui!field_ui.api.php/function/hook_field_formatter_settings_form/7
 * Specify the form elements for a formatter's settings.
 */
function scripture_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $s = $instance['display'][$view_mode]['settings'];
  return array(
    'show_ref' => array(
      '#type' => 'checkbox',
      '#title' => t('Show verse reference'),
      '#required' => false,
      '#default_value' => $s['show_ref']
    ),
    'show_text' => array(
      '#type' => 'select',
      '#title' => t('Show verse text?'),
      '#options' => array(
        SCRIPTURE_SHOW_TEXT_NO => "No",
        SCRIPTURE_SHOW_TEXT_TRUNC => "First and last few words",
        SCRIPTURE_SHOW_TEXT_ALL => "Entire text"
      ),
      '#required' => true,
      '#default_value' => $s['show_text']
    )
  );
}

/**
 * Implements hook_field_formatter_settings_summary($field, $instance, $view_mode).
 * https://api.drupal.org/api/drupal/modules!field_ui!field_ui.api.php/function/hook_field_formatter_settings_summary/7
 * Return a short summary for the current formatter settings of an instance.
 */
function scripture_field_formatter_settings_summary($field, $instance, $view_mode) {
  $s = $instance['display'][$view_mode]['settings'];
  if ($s['show_ref']) {
    $show_ref = "Show verse reference";
  } else {
    $show_ref = "Do not show verse reference";
  }
  switch ($s['show_text']) {
    case SCRIPTURE_SHOW_TEXT_NO:
      $show_text = "Do not show verse text";
      break;
    case SCRIPTURE_SHOW_TEXT_TRUNC:
      $show_text = "Show first and last few words of text range";
      break;
    case SCRIPTURE_SHOW_TEXT_ALL:
      $show_text = "Show the entire text";
      break;
  }
  return "{$show_ref}<br/>{$show_text}";
}

/*
 * FIELD WIDGET
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/group/field_widget/7
 */

/**
 * Implements hook_field_widget_info().
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/function/hook_field_widget_info/7
 */
function scripture_field_widget_info() {
  $widgets = array();
  
  $widgets['scripture_picker'] = array(
    'label' => t('Scripture picker'),
    'description' => t('A compound form element for editing scripture references'),
    'field types' => array(
      'scripture'
    ),
    'behaviours' => array(
      'multiple values' => FIELD_BEHAVIOR_DEFAULT, // the entire widget accepts one entry
      'default_value' => FIELD_BEHAVIOR_NONE
    ) // the widget can have no default value

  );
  
  return $widgets;
}

/**
 * Implements hook_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element).
 * https://api.drupal.org/api/drupal/modules%21field%21field.api.php/function/hook_field_widget_form/7
 * Create the complex form element that constitutes the widget, using basic form elements.
 */
function scripture_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  
  $element['#type'] = 'fieldset'; // just to show the title and put a nice frame around it
  
  $element['verse'] = array(
    '#type' => 'scripture_verse_picker',
  );
  
  if (!empty($items[0])) {
    $translation = ($items[0]['translation'] > 0) ? $items[0]['translation'] : NULL;
    $from_verse = scripture_get_verse($items[0]['from_vid'], $translation);
    $to_verse = scripture_get_verse($items[0]['to_vid'], $translation);
    
    $element['verse']['#default_value'] = array(
      'translation'  => $translation,
      'book'         => $from_verse['booknum'],
      'from_chapter' => $from_verse['chapternum'],
      'from_verse'   => $from_verse['versenum'],
      'to_chapter'   => $to_verse['chapternum'],
      'to_verse'     => $to_verse['versenum'],
      'sid'          => $items[0]['sid'],
    );
  }
  
  return $element;
  
}

/**
 * Implements hook_field_is_empty($item, $field).
 * https://api.drupal.org/api/drupal/modules!field!field.api.php/function/hook_field_is_empty/7
 * Define what constitutes an empty item for a field type.
 */
function scripture_field_is_empty($item, $field) {
  return empty($item['verse']['book']) || empty($item['verse']['from_chapter']) || empty($item['verse']['from_verse']);
}

