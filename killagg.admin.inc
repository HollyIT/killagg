<?php

function killagg_settings() {
  $form['tools'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tools'
  );
  $directory = 'public://';
  $is_writable = is_dir($directory) && is_writable($directory);
  $disabled = !$is_writable;
  $disabled_message = '';
  if (!$is_writable) {
    $disabled_message = ' ' . t('<strong class="error">Set up the <a href="!file-system">public files directory</a> to make these optimizations available.</strong>', array('!file-system' => url('admin/config/media/file-system')));
  }
  $form['tools']['preprocess_css'] = array(
    '#type' => 'checkbox',
    '#title' => t('Aggregate and compress CSS files.'),
    '#description' => 'If this is not enabled then the disable css settings will have no affect',
    '#default_value' => intval(variable_get('preprocess_css', 0) && $is_writable),
    '#disabled' => $disabled,
  );
  $form['tools']['preprocess_js'] = array(
    '#type' => 'checkbox',
    '#title' => t('Aggregate JavaScript files.'),
    '#description' => 'If this is not enabled then the disable js settings will have no affect',
    '#default_value' => intval(variable_get('preprocess_js', 0) && $is_writable),
    '#disabled' => $disabled,
  );
  $form['tools']['buttons'] = array();
  $form['tools']['buttons']['delete_js'] = array(
    '#type' => 'submit',
    '#value' => 'Clear JS Cache',
    '#submit' => array('killagg_clear_js_cache')
  );
  $form['tools']['buttons']['delete_css'] = array(
    '#type' => 'submit',
    '#value' => 'Clear CSS Cache',
    '#submit' => array('killagg_clear_css_cache')
  );
  $form['tools']['buttons']['delete_both'] = array(
    '#type' => 'submit',
    '#value' => 'Clear JS & CSS Cache',
    '#submit' => array('killagg_clear_both_cache')
  );
  $files = array();
  $files['modules'] = _killagg_get_files('module');
  $files['themes'] = _killagg_get_files('theme');
  $files['misc_css'] = _killagg_misc_files('css');
  $files['misc_js'] = _killagg_misc_files('js');
  $files['libraries'] = _killagg_library_paths();

  $types = _killagg_types();
  $defaults = variable_get('killagg_kill_items', array());

  foreach ($types as $type => $typeName) {
    $form[$type] = array(
      '#type' => 'fieldset',
      '#title' => $typeName,
      '#tree' => TRUE,
      '#theme' => 'killagg_admin_table',
      '#header_title' => $typeName,
    );
    foreach ($files[$type] as $key => $name) {
      $form[$type][$key]['name'] = array('#markup' => $name);
      if ($type != 'misc_js') {
        $form[$type][$key]['disable_css'] = array(
          '#type' => 'checkbox',
          '#default_value' => isset($defaults['css'][$key]['disabled']) ? $defaults['css'][$key]['disabled'] : 0
        );
      }
      if ($type != 'misc_css') {
        $form[$type][$key]['disable_js'] = array(
          '#type' => 'checkbox',
          '#default_value' => isset($defaults['js'][$key]['disabled']) ? $defaults['js'][$key]['disabled'] : 0
        );
      }
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit'
  );

  return $form;
}

function killagg_clear_js_cache(&$form, &$form_state) {
  drupal_clear_js_cache();
  drupal_set_message('Javascript cache cleared');
}

function killagg_clear_css_cache(&$form, &$form_state) {
  drupal_clear_css_cache();
  drupal_set_message('CSS cache cleared');
}

function killagg_clear_both_cache(&$form, &$form_state) {
  killagg_clear_js_cache($form, $form_state);
  killagg_clear_css_cache($form, $form_state);
}

function killagg_settings_submit(&$form, &$form_state) {

  variable_set('preprocess_css', $form_state['values']['preprocess_css']);
  variable_set('preprocess_js', $form_state['values']['preprocess_js']);
  $types = _killagg_types();
  $settings = array();
  foreach (array_keys($types) as $type) {

    foreach ($form_state['values'][$type] as $name => $item) {
      if (isset($item['disable_css']) && $item['disable_css']) {
        $settings['css'][$name] = array('disabled' => TRUE, 'type' => $type);
      }
      if (isset($item['disable_js']) && $item['disable_js']) {
        $settings['js'][$name] = array('disabled' => TRUE, 'type' => $type);
      }
    }
  }
  variable_set('killagg_kill_items', $settings);
  drupal_set_message('The configuration has been saved');
}

function theme_killagg_admin_table(&$vars) {
  $element = $vars['element'];
  $header = NULL;
  $rows = array();
  foreach (element_children($element) as $item) {
    $row = array();
    if (!$header) {
      $header = array($element['#header_title']);
      if (isset($element[$item]['disable_css'])) {
        $header[] = 'Disable CSS';
      }
      if (isset($element[$item]['disable_js'])) {
        $header[] = 'Disable JS';
      }
    }
    $row[] = drupal_render($element[$item]['name']);
    if (isset($element[$item]['disable_css'])) {
      $row[] = drupal_render($element[$item]['disable_css']);
    }
    if (isset($element[$item]['disable_js'])) {
      $row[] = drupal_render($element[$item]['disable_js']);
    }
    $rows[] = $row;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

function _killagg_misc_files($type) {
  $files = file_scan_directory('misc', '/\.' . $type . '$/');
  $items = array();
  foreach ($files as $file) {
    $items[$file->uri] = $file->filename;
  }
  return $items;
}

function _killagg_library_paths() {
  $base = 'sites/all/libraries';
  $dir = scandir($base);
  $items = array();
  foreach ($dir as $d) {

    if (substr($d, 0, 1) != '.' && is_dir($base . '/' . $d)) {
      $items[$base . '/' . $d] = $d;
    }
  }
  return $items;
}

function _killagg_types() {
  return array(
    'modules' => 'Modules',
    'themes' => 'Themes',
    'misc_css' => 'Misc CSS',
    'misc_js' => 'Misc JS',
    'libraries' => 'Libraries'
  );
}