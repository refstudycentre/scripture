<?php

/*
 * USER PAGE CALLBACKS
 */

/**
 * Page callback for scripture/*
 * List all nodes associated with the given verse range
 */
function scripture_callback_node_listing($param) {
  $content = array();

  $range = scripture_decode_range($param);
  $range['translation'] = variable_get('scripture_default_translation', NULL);
  // TODO: check if range is valid before calling preview
  
  $content['scripture'] = scripture_preview($range);
  
  return $content;
}
