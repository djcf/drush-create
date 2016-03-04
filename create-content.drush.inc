<?php
/**
 * Create a taxonomy vocabulary and return the vid.
 */
function drush_createcontent_create_vocabulary($name=null, $machine_name=null) {
  $vocabulary = new stdClass();
  $vocabulary->description = drush_get_option('description', null);

  if (!$name) {
    $name = drush_prompt(dt('Please enter the name for your new vocabulary'));
  }
  if (!$machine_name) {
    $machine_name = drush_prompt(dt('Please enter the machine name for your new vocabulary'));
    $vocabulary->description = drush_prompt(dt('Please enter a description for your new vocabulary'));
  }
  $vocabulary->name = $name;
  $vocabulary->machine_name = $machine_name;  

  taxonomy_vocabulary_save($vocabulary);

  if (drush_get_option('verbose', false)) {
    drush_log("Your new vocabulary named $name has vid #" . $vocabulary->vid, 'ok');
  } elseif (drush_get_option('auto', false)) {
    echo $vocabulary->vid;
  } elseif (!drush_get_option('quiet', false)) {
    drush_log($vocabulary->vid, 'ok');
  }
}

/**
 * Create a taxonomy term and return the tid.
 */
function drush_createcontent_create_term($name=null, $vid=null) {
  $term = new stdClass();
  $term->description = drush_get_option('description', null);

  if (!$name) {
    $name = drush_prompt(dt('Please enter the name for your new term'));
    $term->description = drush_prompt(dt('Please enter a description for your new term'));
  }
  while (!is_numeric($vid)) {
    $vid = drush_prompt(dt('Please enter the ID of the vocabulary your new term will belong to'));
  }

  $term->name = $name;
  $term->vid = $vid;
  $term->parent = drush_get_option('p');

  taxonomy_term_save($term);

  if (drush_get_option('verbose', false)) {
    drush_log("Your new term named $name has tid #" . $term->tid, 'ok');
  }  elseif (drush_get_option('auto', false)) {
    echo $term->tid;
  } elseif (!drush_get_option('quiet', false)) {
    drush_log($term->tid, 'ok');
  }
}

/**
 * Create a taxonomy term and return the tid.
 */
function drush_createcontent_create_node($title=null, $node_type=null, $uid=null) {
  $node = new stdClass();    
  if (!$uid) {
    $interactive = true;
    $name = drush_prompt(dt('Please enter the name of your new node'));
    $node_type = drush_prompt(dt('Please enter the content type for your new node'));
    $node->language = drush_prompt(dt('Enter the node language or <enter> for LANGUAGE_NONE'));

    // Unfortunately drush interprets 0 as cancel so we have to present every 0-possible choice as indexed from 1
    $boolean_options = array(1=> 'No', 'Yes');
    $trichoice = array(1=>'Comments disabled', 'Read-only', 'Read/write');

    $node->comment = drush_choice($trichoice, dt('Are comments on the node enabled?'));
    $node->status = drush_choice($boolean_options, dt('Is the node published or still a draft?'));
    $node->promote = drush_choice($boolean_options, dt('Is the node promoted to the front page?'));
    $node->sticky = drush_choice($boolean_options, dt('Is the node stickied at the top of lists?'));

    // See l65. Now we shift the index down, back to 0.
    foreach (array('comment', 'status', 'promote', 'sticky') as $x) {
      $node->$x--;
    }

    $user_prompt = drush_prompt(dt('Would you like to enter a body for the new node? Enter \'y\' if so'));
    if ($user_prompt == "y") {
      $value = drush_prompt(dt('Enter the text for the body of the new node'));
      $format = drush_prompt(dt('What format is the body in? (e.g. \'filtered_html\', \'full_html\', etc.)'));
      $safe_value = "<p>$value</p>";
      $summary = $safe_summary = null;
      $user_prompt = drush_prompt(dt('Would you like to enter a summary for the body of the new node? Enter \'y\' if so'));
      if ($user_prompt == "y") {
        $summary = drush_prompt(dt('Enter the text for the summary for the body of the new node'));
        $safe_summary = "<p>$safe_summary</p>";
      }
      $node->body[$node->language][0] = compact($value, $summary, $format, $safe_value, $safe_summary);
    }

    $user_prompt = drush_prompt(dt('Would you like to enter a new field? You will need to know the field machine name and the names of any properties it expects. (Enter \'y\' if so.)'));
    while ($user_prompt == "y") {
      $field_name = drush_prompt(dt('Enter the field\'s machine name (e.g. \'field_machine_name\')'));
      $field = array();
      do {
        $property = drush_prompt(dt("Enter the name for a property of '$field_name', e.g.'value\', 'summary\', 'format', etc."));
        $value = drush_prompt(dt("Enter the value for the property '$property' of '$field_name'"));
        $field[$property] = $value;
        $user_prompt = drush_prompt(dt('Would you like to enter another property and value for $field_name?'));
      } while ($user_prompt=='y');
      array_push($node->$field_name[$node->language], $field);
      $user_prompt = drush_prompt(dt('Would you like to enter another field?'));
    };

    $user_prompt = drush_prompt(dt('Would you like to enter a new term reference (taxonomy) field? You will need to know the term ID and the field machine name. (Enter \'y\' if so)'));
    while ($user_prompt == "y") {
      $field_name = drush_prompt(dt('Enter the field\'s machine name (e.g. \'field_machine_name\')'));
      do {
        $tid = drush_prompt(dt('Enter the term ID'));
      } while (!is_numeric($tid));
      array_push($node->$field_name[$node->language], array('tid' =>$tid));

      $user_prompt = drush_prompt(dt('Would you like to enter another term reference field?'));
    }

    $user_prompt = drush_prompt(dt('Would you like to enter a new entity reference (node) field? You will need to know the node ID, it\'s content type, and the machine name of the reference field. (Enter \'y\' if so)'));
    while ($user_prompt == "y") {
        do {
          $target_id = drush_prompt(dt('Enter the target\'s nID'));
        } while (!is_numeric($target_id));
        $field_name = drush_prompt(dt('Enter the entity reference\'s field machine name (e.g. \'field_machine_name\')'));
        $target_type = drush_prompt(dt('Enter the target\'s content target type (e.g. \'node\' is default, other possible values are \'user\' and \'taxonomy_term\''));    
        array_push($node->$field_name[$node->language], array(
          'target_id' => $target_id,
          'target_type' => $target_type
        ));

      $user_prompt = drush_prompt(dt('Would you like to enter another entity reference field?'));
    }
  } else {
    $interactive = false;
  }
  while (!is_numeric($uid)) {
    $vid = drush_prompt(dt('Please enter the uID of the user who created the new node'));
  }


  $node->title = $title;
  $node->uid = $uid;
  $node->type = $node_type;

  node_object_prepare($node);

  if (!$interactive) {
    $node->language = drush_get_option('language', LANGUAGE_NONE);
    $node->comment = drush_get_option('comments', 2);
    $node->status = drush_get_option('status', 1);
    $node->promote = drush_get_option('promoted', 0);
    $node->sticky = drush_get_option('sticky', 0);
    $value = drush_get_option('body', null);
    if ($value) {
      $summary = drush_get_option('summary', '');
      $format = drush_get_option('input_format', 'filtered_html');
      $safe_value = "<p>$value</p>";
      $node->body[$node->language][0] = compact($value, $summary, $format, $safe_value);
    }
  }

  $node = node_submit($node); // prepare for saving
  node_save($node);
  
  if (drush_get_option('verbose', false)) {
    drush_log("Your new node named $title has been created with nID #" . $node->nid . ' and the following properties', 'ok');
    foreach (array($node) as $key=>$value) {
      dt(" *    $key$value");
    }
  } elseif (drush_get_option('auto', false)) {
    echo $node->nid;
  } elseif (!drush_get_option('quiet', false)) {
    drush_log($node->nid, 'ok');
  }
}

/**
 * Implements hook_drush_command().
 */
function createcontent_drush_command() {
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
      'language' => 'The language for your new node or LANGUAGE_NONE if unspecified',
      'comments' => 'Whether comments should be enabled or not0 for disabled, 1 for read-only and 2 for read/write which is the default option.',
      'status' => 'Whether the node is published or still a draft0 for draft, 1 for published.',
      'sticky' => 'Whether the node is promoted to the top of lists0 for not stickied, 1 for always appears at the top of lists.',
      'promote' => 'Whether the node is promoted to the front page or not; 0 for unpromoted (default) or 1 for promoted.',
      'body' => 'Contents of the body field for the node',
      'input_format' => 'Input format for the body of the node',
      'summary' => 'Summary for the body of the node',
      'quiet' => 'Don\'t make any noice if successful',
      'verbose'=> 'Pretty-print the new node\'s details'
    ),
    'examples' => array(
      'drush cnn' => 'Create the new node interactively.',
      'drush cnn newnode 100 \
        --language=LANGUAGE_NONE \
        --comments=2 \
        --status=1 \
        --sticky=0 \
        --promote=0 \
        --body="Hello, world" \
        --input_format="filtered_html", \
        --verbose' => 'Create a new node with custom data',
    )
  );

  return $items;
}

