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
 * Form for admin/structure/scripture/subverses
 * Build a form to manage (add, modify, delete) the subverses, making use of single verse picker
 *
 * Multi-step form tutorial:
 * http://www.jaypan.com/tutorial/drupal-7-ajax-driven-multi-step-forms-non-javascript-fallback
 */
function scripture_callback_subverses($form, &$form_state) {
  
  $step = $form_state['step'] = isset($form_state['step']) ? $form_state['step'] : 1;
  $n_steps = 2; // number of steps
  $form_state['storage']['num_new'] = isset($form_state['storage']['num_new']) ? $form_state['storage']['num_new'] : 0;
   
  // a wrapper in which to place the form step
  $wrapper = drupal_html_id("subverses_form_step");
  $form['#prefix'] = "<div id='{$wrapper}'>";
  $form['#suffix'] = "</div>";
  
  // load the default values for the current step
  if (isset($form_state['values']["step_{$step}"])) {
    $default = $form_state['values']["step_{$step}"];
  } elseif (isset($form_state['storage']["step_{$step}"])) {
    $default = $form_state['storage']["step_{$step}"];
  } else {
    $default = NULL;
  }
  
  // using the same callback for all buttons
  $ajax = array(
    'wrapper'  => $wrapper,
    'callback' => 'scripture_callback_subverses_ajax'
  );
  
  // Generate the appropriate form step
  switch($step) {
    case 1:
      $form['step_1'] = array(
        '#type' => 'scripture_verse_picker',
        '#title' => 'Pick a verse to work with',
        '#required' => TRUE,
        '#scripture_settings' => array(
          'require_translation' => TRUE,
          'allow_range'         => FALSE,
          'allow_subverse'      => FALSE,
        ),
        '#default_value' => $default,
        '#tree' => TRUE,
      );
      break;
    case 2:
      
      $s1 = &$form_state['storage']['step_1'];
      
      $subverses = scripture_get_subverses($s1['from_vid'],$s1['translation']);
      
      $subverse_header = array(
        'sid'  => t('Subverse ID'),
        'text' => t('Relevant part of verse text'),
      );
      
      $subverse_rows = array();
      foreach ($subverses as $sid => $subverse) {
        $subverse_rows[$sid] = array(
          'sid' => $sid,
          'text' => $subverse->text,
        );
      }
      
      $form['step_2'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
      );
      $form['step_2']['verse_preview'] = array(
        '#type'   => 'markup',
        '#markup' => drupal_render(scripture_preview($s1)),
      );
      $form['step_2']['current_subverses'] = array(
        '#type'    => 'tableselect',
        '#header'  => $subverse_header,
        '#options' => $subverse_rows,
      );
      $form['step_2']['delete'] = array(
        '#type'   => 'submit',
        '#value'  => t('Delete selected subverses'),
        '#limit_validation_errors' => array(),
        '#submit' => array('scripture_callback_subverses_delete'),
        '#ajax'   => &$ajax,
      );
      $form['step_2']['new'] = array(
        '#type' => 'container',
      );
      $form['step_2']['add'] = array(
        '#type'   => 'submit',
        '#value'  => t('Add ...'),
        '#limit_validation_errors' => array(),
        '#submit' => array('scripture_callback_subverses_add'),
        '#ajax'   => &$ajax,
      );
      
      for ($i=0;$i<$form_state['storage']['num_new'];$i++) {
        $form['step_2']['new']["new_{$i}"] = array(
          '#type' => 'container',
          'sid' => array(
            '#type' => 'textfield',
            '#title' => t('Subverse'),
            '#size' => 1,
            '#maxlength' => 3,
          ),
          'text' => array(
            '#type' => 'textfield',
            '#title' => t('Text'),
            '#maxlength' => 255,
          ),
        );
      }
      
      break;
  }
  
  // Create buttons
  
  $form['buttons'] = array(
    '#type' => 'container',
  );
  
  if ($step != 1) {
    $form['buttons']['prev'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#limit_validation_errors' => array(),
      '#submit' => array('scripture_callback_subverses_back'),
      '#ajax' => &$ajax,
    );
  }
  
  if ($step !== $n_steps) {
    $form['buttons']['next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
      '#ajax' => &$ajax,
    );
  } else {
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#ajax' => &$ajax,
    );
  }
  
  return $form;
}

/**
 * Callback for clicking the "back" button
 */
function scripture_callback_subverses_back($form, &$form_state) {
  
  // reset the form, but retain only step 1 data
  $form_state['storage'] = array(
    'step_1' => $form_state['storage']['step_1'],
  );
  
  // move one step back
  $form_state['step']--;
  $form_state['rebuild'] = TRUE;
}

/**
 * Callback for clicking the "delete" button
 */
function scripture_callback_subverses_delete($form, &$form_state) {
  
  $vid = $form_state['storage']['step_1']['from_vid'];
  $translation = $form_state['storage']['step_1']['translation'];
  $selected = $form['step_2']['current_subverses']['#value'];
  
  $n = scripture_delete_subverses($vid,$translation,$selected);
  
  if ($n > 0) {
    drupal_set_message(t('%n subverses were deleted from the database.', array('%n'=>$n)), 'status');
  } else {
    drupal_set_message(t('An error occured. Nothing was deleted ... hopefully.'), 'error');
  }
  
  $form_state['rebuild'] = TRUE;
  
}

/**
 * Callback for clicking the "add" button
 * Add a row for filling in new subverses.
 */
function scripture_callback_subverses_add($form, &$form_state) {
  $form_state['storage']['num_new']++;
  $form_state['rebuild'] = TRUE;
}


/**
 * Callback for clicking any AJAX button
 */
function scripture_callback_subverses_ajax($form, &$form_state) {
  return $form;
}

/**
 * Valdiate each step of the form
 */
function scripture_callback_subverses_validate($form, &$form_state) {
  switch($form_state['step'])
  {
    case "1":
      break;
  
    case "2":
      
      // if there are any new subverse rows
      if (isset($form_state['values']['step_2']['new'])) {
        $current_subverses = array_keys($form_state['values']['step_2']['current_subverses']);
        
        // loop through them
        foreach ($form_state['values']['step_2']['new'] as $key => $val) {
          
          // if both are empty
          if (empty($val['sid']) && empty($val['text'])) {
            
            // remove empty rows
            unset($form_state['values']['step_2']['new'][$key]);
            
          } else {
            
            // throw errors for incomplete entries
            if (empty($val['sid'])) {
              form_error($form['step_2']['new'][$key]['sid'],t('Subverse ID omitted.'));
            } elseif (empty($val['text'])) {
              form_error($form['step_2']['new'][$key]['text'],t('Text omitted.'));
            }
            
          }
          
          if (in_array($val['sid'], $current_subverses)) {
            form_error($form['step_2']['new'][$key]['sid'],t('Subverse already exists.'));
          }
          
        }
      }
      
      break;
  
  }
}

/**
 * Submit the form when clicking "next" or "submit" buttons
 */
function scripture_callback_subverses_submit($form, &$form_state) {

  $step = &$form_state['step'];
  $form_state['storage']['step_' . $step] = $form_state['values']['step_' . $step];
  
  if (isset($form_state['values']['next']) && $form_state['values']['op'] == $form_state['values']['next']) {
    $step++; // next step
  } elseif (isset($form_state['values']['submit']) && $form_state['values']['op'] == $form_state['values']['submit']) {
    
    // if there are new and valid entries, add them to the database
    if (!empty($form_state['storage']['step_2']['new'])) {
      $vid = $form_state['storage']['step_1']['from_vid'];
      $translation = $form_state['storage']['step_1']['translation'];
      $new = &$form_state['storage']['step_2']['new'];
      
      scripture_save_subverses($vid, $translation, $new);
    }

    // stay on step 2 to review the result of the database insert
    $step = 2;
    
    // reset the form, but retain only step 1 data
    $form_state['storage'] = array(
      'step_1' => $form_state['storage']['step_1'],
    );
  }
  
  $form_state['rebuild'] = TRUE;
}
