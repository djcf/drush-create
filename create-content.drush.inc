<?php
/**
 * Create a taxonomy vocabulary and return the vid.
 */
function drush_create_vocabulary($name=null, $machine_name=null) {
  if (!$name) {
    $name = drush_choice(dt('Please enter the name for your new vocabulary: '));
  }
  if (!$machine_name) {
    $machine_name = drush_choice(dt('Please enter the machine name for your new vocabulary: '));
  }
  $vocabulary = new stdClass();
  $vocabulary->name = $name;
  $vocabulary->machine_name = $machine_name;
  
  $vocabulary->description = drush_get_option('description', null);

  taxonomy_vocabulary_save($vocabulary);

  if (drush_get_option('verbose', false) {
    dt("Your new vocabulary named $name has vid #" . $vocabulary->vid);
  } elseif (!drush_get_option('quiet', false)) {
    dt($vocabulary->vid);
  }
}

/**
 * Create a taxonomy term and return the tid.
 */
function drush_create_term($name=null, $vid=null) {
  if (!$name) {
    $name = drush_choice(dt('Please enter the name for your new term: '));
  }
  if (!$vid) {
    while (!is_numeric($vid)) {
      $vid = drush_choice(dt('Please enter the ID of the vocabulary your new term will belong to: '));
    }
  }

  $term = new stdClass();
  $term->name = $name;
  $term->vid = $vid;
  
  $term->description = drush_get_option('description', null);
  $term->parent = drush_get_option('p');

  taxonomy_term_save($term);

  if (drush_get_option('verbose', false) {
    dt("Your new term named $name has tid #" . $term->tid);
  } elseif (!drush_get_option('quiet', false)) {
    dt($term->tid);
  }
}

/**
 * Create a taxonomy term and return the tid.
 */
function drush_create_node($title=null, $node_type=null, $uid=null) {
  $node = new stdClass();

  if (!$uid) {
    $interactive = true;
    $name = drush_choice(dt('Please enter the name of your new node: '));
    $node_type = drush_choice(dt('Please enter the content type for your new node: '));
    while (!is_numeric($uid)) {
      $vid = drush_choice(dt('Please enter the uID of the user who created the new node: '));
    }
    $boolean_options = array('No', 'Yes');
    $node->language = drush_choice(dt('Enter the node language or <enter> for LANGUAGE_NONE: '));
    $node->comment = drush_choice(array('Comments disabled', 'Read-only', 'Read/write'), dt('Are comments on the node enabled?: '));
    $node->status = drush_choice($boolean_options, dt('Is the node published or still a draft?: '));
    $node->promote = drush_choice($boolean_options, dt('Is the node promoted to the front page?: '));

    $user_prompt = "y"
    while ($user_prompt == "y")) {
      $user_prompt = drush_choice(dt('Would you like to enter a new term reference (taxonomy) field? You will need to know the term ID and the field machine name. (Enter \'y\' if so:)'));
      if ($user_prompt == "y") {
        $field_name = drush_choice(dt('Enter the field\'s machine name (e.g. \'field_machine_name\')'));
        $tid = null;
        while (!is_numeric($tid)) {
          $tid = drush_choice(dt('Enter the term ID: '));
        }
        $node->$field_name[$node->language][]['tid'] = $tid
      }
    }

    $user_prompt = "y"
    while ($user_prompt == "y")) {
      $user_prompt = drush_choice(dt('Would you like to enter a new entity reference (node) field? You will need to know the node ID, it\'s content type, and the machine name of the reference field. (Enter \'y\' if so:)'));
      if ($user_prompt == "y") {
        $target_id = null;
        while (!is_numeric($target_id)) {
          $target_id = drush_choice(dt('Enter the target\'s nID: '));
        }
        $field_name = drush_choice(dt('Enter the entity reference\'s field machine name (e.g. \'field_machine_name\')'));
        $target_type = drush_choice(dt('Enter the target\'s content target type (e.g. \'node\' is default, other possible values are \'user\' and \'taxonomy_term\': '));    
        $node->$field_name[$node->language][] = array(
          'target_id' => $target_id,
          'target_type' => $target_type
        );
      }
    }

  } else {
    $interactive = false;
  }

  $node->title = $title;
  $node->uid = $uid;
  $node->type = $node_type;

  node_object_prepare($node);

  if (!$interactive) {
    $node->language = drush_get_option('description', LANGUAGE_NONE);
    $node->comment = drush_get_option('comments', 2);
    $node->status = drush_get_option('status', 1);
    $node->promote = drush_get_option('promote', 0);
  }

  $node = node_submit($node); // prepare for saving
  node_save($node);
  
  if (drush_get_option('verbose', false) {
    dt("Your new node named $title has been created with nID #" . $node->nid);
  } elseif (!drush_get_option('quiet', false)) {
    dt($node->nid);
  }
}

/**
 * Implements hook_drush_command().
 */
function drush_create_content_command() {
  $items['create-term'] = array(
    'description' => 'Create a new taxonomy term given a name for the new term and a vocabulary ID. Returns the tID of the new term.',
    'aliases' => array('ctt'),
    'arguments' => array(
      'name' => 'The name of the new taxonomy term',
      'vid' => 'The vocabulary ID for the new term'
    ),
    'options' => array(
      'description' => 'A description for the new term',
      'parent_id' => 'The term ID for the parent of this new term',
      'quiet' => 'Don\'t make any noice if successful',
      'verbose'=> 'Pretty-print the new term ID'
    ),
    'examples' => array(
      'drush ctt' => 'Create the new term interactively.',
      'drush ctt newterm 1 --parent_id=10 --description="My new term"' => 'Create a new term with custom data',
    ),
  );

  $items['create-vocabulary'] = array(
    'description' => 'Create a new taxonomy vocabulary given a name for the new vocabulary and a machine name. Returns the vID of the new vocabulary.',
    'aliases' => array('ctv'),
    'arguments' => array(
      'name' => 'The name of the new vocabulary',
      'machine_name' => 'The machine identifier for the new vocabulary.'
    ),
    'options' => array(
      'description' => 'A description for the new vocabulary',
      'quiet' => 'Don\'t make any noice if successful',
      'verbose'=> 'Pretty-print the new term ID'
    ),
    'examples' => array(
      'drush ctv' => 'Create the new vocabulary interactively.',
      'drush ctv mynewvocab my_new_vocab --description="My new vocabulary"' => 'Create a new vocabulary with custom data',
    )
  );

  $items['create-node'] = array(
    'description' => 'Create a new content node given a name for the new vocabulary and a machine name. Returns the nID of the new node.',
    'aliases' => array('cnn'),
    'arguments' => array(
      'name' => 'The name of the new node',
      'uid' => 'The user who created the node.'
    ),
    'options' => array(
      'description' => 'A description for the new vocabulary',
      'quiet' => 'Don\'t make any noice if successful',
      'verbose'=> 'Pretty-print the new term ID'
    ),
    'examples' => array(
      'drush ctv' => 'Create the new node interactively.',
      'drush ctv mynewvocab my_new_vocab --description="My new vocabulary"' => 'Create a new vocabulary with custom data',
    )
  );

  return $items;
}

