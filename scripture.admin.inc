<?php

/*
 * ADMIN PAGE CALLBACKS
 * Three different strategies for page callbacks: http://www.drupalcoder.com/blog/how-to-define-page-callbacks-that-you-dont-want-to-render-using-page-template-in-drupal.html
 * Build the callbacks using renderable arrays. Good reading on renderable arrays:
 * - https://www.drupal.org/node/930760
 * - http://cocoate.com/ddbook/scary-render-array
 * - http://drupal.stackexchange.com/questions/11438/how-to-nest-elements-in-a-render-array
 */

/**
 * Page callback for admin/structure/scripture[/status]
 * summarize what is going on in the Scripture db tables
 */
function scripture_callback_status() {
  $content = array();
  
  $translations = scripture_get_translations();
  
  $translation_rows = array();
  foreach ($translations as $id => $translation) {
    
    $books = scripture_get_books($id);
    
    $translation_rows[] = array(
      $translation->name,
      count($books),
    );
    
  }
  
  $content['summary'] = array(
    '#prefix' => '<h2>' . t('Summary') . '</h2>',
    '#theme' => 'table',
    '#rows' => array(
      array(
        t('Number of translations'),
        count($translations)
      ),
    )
  );
  
  $content['translations'] = array(
    '#prefix' => '<h2>' . t('Translations') . '</h2>',
    '#theme' => 'table',
    '#header' => array(
      t('Translation'),
      t('Number of books'),
    ),
    '#rows' => $translation_rows
  );
  
  $content['TODO'] = array(
    '#prefix' => '<h2>TODO</h2>',
    '#markup' => 'TODO: display any other required statistics here',
  );
  
  return $content;
}

/**
 * Page callback for admin/structure/scripture/translations
 */
function scripture_callback_translations() {
  $form = array();
  
  $translations = scripture_get_translations();
  foreach ($translations as $abbr => &$t) {
    $t = "{$t->abbr} / {$t->lang} / {$t->name}";
  }
  
  $form['scripture_default_translation'] = array(
    '#type' => 'select',
    '#title' => t('Default translation'),
    '#options' => $translations,
    '#description' => t('Choose the default translation to use when none is specified.'),
    '#required' => true,
    '#default_value' => array(
      variable_get('scripture_default_translation', NULL)
    )
  );
  
  return system_settings_form($form);
}

/**
 * Page callback for admin/structure/scripture/translations
 */
function scripture_callback_queries() {
  $form = array();

  $form['scripture_match_quality_weight_query'] = array(
    '#type' => 'textfield',
    '#size' => 3,
    '#title' => t('Query match quality weight'),
    '#description' => t('The relative importance of search results covering the entire query.'),
    '#required' => true,
    '#default_value' => 1,
  );

  $form['scripture_match_quality_weight_entity'] = array(
    '#type' => 'textfield',
    '#size' => 3,
    '#title' => t('Entity match quality weight'),
    '#description' => t('The relative importance of the query covering the entire range of verses in the search results.'),
    '#required' => true,
    '#default_value' => 1,
  );

  return system_settings_form($form);
}
