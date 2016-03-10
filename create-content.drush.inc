<?php
require_once('utils.php');
require_once('NodeFactory.php');

/**
 * Create a taxonomy vocabulary and return the vid.
 */
function drush_createcontent_create_vocabulary($name=null, $machine_name=null, $automate=false) {
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

  if (!$automate) {
    $automate = drush_get_option('auto', false);
  }

  taxonomy_vocabulary_save($vocabulary);

  if (drush_get_option('verbose', false)) {
    drush_log("Your new vocabulary named $name has vid #" . $vocabulary->vid, 'ok');
  } elseif ($automate) {
    echo $vocabulary->vid;
  } elseif (!drush_get_option('quiet', false)) {
    drush_log($vocabulary->vid, 'ok');
  }

}

/**
 * Create a taxonomy term and return the tid.
 */
function drush_createcontent_create_node($node_type=null) {
  $node = NodeFactory::get_node_wrapper($node_type);
  $node->save();

  $actioned = $node->is_new->value() ? "created" : "updated";
  if (drush_get_option('verbose', false)) {
    drush_log("Your node named " . $node->title->value() . " with nID #" . $node->nid->value() . " has been $actioned with the following properties", 'ok');
    #print_r($node);
  } elseif (drush_get_option('auto', false)) {
    echo $node->nid->value();
  } elseif (!drush_get_option('quiet', false)) {
    drush_log($actioned . " node nID #" . $node->nid->value(), 'ok');
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
  if (!$vid) {
    $vid = drush_prompt(dt('Please enter the ID of the vocabulary your new term will belong to or its unique machine name'));
  }

  do {
    if (is_numeric($vid)) {
      $vocabulary = taxonomy_vocabulary_load($vid);
    } else {
      $vocabulary = taxonomy_vocabulary_machine_name_load($vid);
    }
    if (!$vocabulary) {
      echo "There is no vocabulary with that name or that ID '$vid' in the system. ";
      $vid = drush_prompt(dt('Please enter a valid vocabulary ID or a valid vocabulary machine name.'));
    }
  } while (!$vocabulary);

  $term->name = $name;
  $term->vid = $vocabulary->vid;
  $term->parent = drush_get_option('parent');

  $existing_term_id = term_exists($term->name, $vocabulary)->tid;

  if(!$existing_term_id) {
    taxonomy_term_save($term);

    if (drush_get_option('verbose', false)) {
      drush_log("Your new term named $name has tid #" . $term->tid, 'ok');
    }  elseif (drush_get_option('auto', false)) {
      echo $term->tid;
    } elseif (!drush_get_option('quiet', false)) {
      drush_log($term->tid, 'ok');
    }
    return $term->tid;
  } else {
    if (drush_get_option('auto', false)) {
      echo $existing_term_id;
    } elseif (!drush_get_option('quiet', false)) {
        drush_log("A term named " . $term->name . " already exists in vocabulary " . $vocabulary->name . ". It has tID #$existing_term_id.", 'ok');
    }
    return $existing_term_id;
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
      'auto' => 'Prints only the tiD when run',
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
      'auto' => 'Prints only the vID when run',
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
      'type' => 'The content-type of the new node',
    ),
    'options' => array(
      'uid' => 'The user ID of the user who created the node. Defaults to 1.',
      'title' => 'The title of the new node',
      'language' => 'The language for your new node or LANGUAGE_NONE if unspecified',
      'comments' => 'Whether comments should be enabled or not. 0 for disabled, 1 for read-only and 2 for read/write which is the default option.',
      'status' => 'Whether the node is published or still a draft. 0 for draft, 1 for published.',
      'sticky' => 'Whether the node is promoted to the top of lists. 0 for not stickied, 1 for always appears at the top of lists.',
      'promote' => 'Whether the node is promoted to the front page or not; 0 for unpromoted (default) or 1 for promoted.',
      'body' => 'Contents of the body field for the node',
      'input_format' => 'Input format for the body of the node',
      'summary' => 'Summary for the body of the node',
      'quiet' => 'Don\'t make any noice if successful',
      'nid' => 'You can specify the node ID before it is created. However the Node ID must not already exist.',
      'created' => 'Specify the time the node was created as a unix timestamp. Defaults to now()',
      'changed' => 'Specify the time the node was last updated as a unix timestamp. Defaults to created.',
      'vocabularies' => 'Specify a comma-separated list of vocabularies for the new node',
      '$vocabulary_machine_name' => 'Specify a comma-separated list of term names or term IDs associated with the $vocabulary_machine_name',
      'references' => 'Specify a comma-separated list of entity reference field names for the new node',
      '$reference_node_id' => 'Specify a comma-separated list of node IDs associated with the entity $reference',
      'revision_timestamp' => 'Specify the last revision. Defaults to changed.',
      'verbose'=> 'Pretty-print the new node\'s details',
      'auto' => 'Prints only the nID when run',
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

