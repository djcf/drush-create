<?php
class NodeFactory {
  protected $entity = null;
  protected $entity_wrapper = null;
  protected $body = null;

  function __construct($node_type=null) {
    $this->get_entity($this->get_options($node_type));
    if($this->body) {
      $this->get_entity()->body->set($this->body);
    }
    $this->get_field_references();
    return $this->get_entity();
  }

  function get_field_references() {}

  function get_entity($values=null) {
    if (!$this->entity) {
      $this->entity = entity_create('node', $values);
      $this->entity_wrapper = entity_metadata_wrapper('node', $this->entity);
    }
    return $this->entity_wrapper;
  }

  function set_field_reference($field_name, $value) {
    if (!is_array($this->get_entity()->$field_name->raw())) {
      $value = $value[0];
    }
    $this->get_entity()->$field_name->set(make_ints($value));
  }

  function get_options($node_type) {
    $node_values = array();
    $node_values['nid'] = drush_get_option('nid', null);
    if (!$node_values['nid']) {
      unset($node_values['nid']);
      unset($node_values['vid']);
    } else {
      while (!is_numeric($node_values['nid'])) {
        $node_values['nid'] = drush_prompt(dt('Please enter a numeric node ID'));
      }
    }

    $node_values['is_new'] = true;
    if (isset($node_values['nid'])) {
      if (node_load($node_values['nid'])) {
        $node_values['is_new'] = false;
        if (!(drush_get_option('auto', false) || drush_get_option('quiet', false))) {
          drush_log("A node with nID #" . $node_values['nid'] . " already exists in the database.");
          if (!$node_type && !user_prompt('Would you like to overwrite/update the existing node with new data?')) {
            drush_log("Exiting...");
            exit(0);
          }
        }
      }
    }

    $node_values['type'] = $node_type ? $node_type : drush_prompt(dt('Please enter the new node\'s content type'));

    $node_values['uid'] = drush_get_option('uid', 1);  
    while (!is_numeric($node_values['uid'])) {
      $node_values['uid'] = drush_prompt(dt('Please enter the uID of the user who created the new node'));
    }

    return $node_values;
  }
}

class AutomatedNodeFactory extends NodeFactory {
  function get_options($node_type) {
    $node_values = parent::get_options($node_type);
    $node_values['created'] = drush_get_option('created', time());
    $node_values['changed'] = drush_get_option('changed', $node_values['created']);
    $node_values['revision_timestamp'] = drush_get_option('revision_timestamp', $node_values['changed']);

    if (drush_get_option('language', null)) {
      $node_values['language'] = drush_get_option('language', null);
    }

    $node_values['comment'] = drush_get_option('comments', 2);
    $node_values['status'] = drush_get_option('status', 1);
    $node_values['promote'] = drush_get_option('promoted', 0);
    $node_values['sticky'] = drush_get_option('sticky', 0);
    $node_values['title'] = drush_get_option('title', '');

    $this->body = $this->get_body();
    return $node_values;
  }

  function get_body() {
    $body = array();
    $body['value'] = drush_get_option('body', null);
    if ($body['value']) {
      $body['summary'] = drush_get_option('summary', '');
      $body['format'] = drush_get_option('input_format', 'filtered_html');
      $body['safe_value'] = strip_tags($body['value'], "<p>");
      $body['safe_summary'] = strip_tags($body['summary'], "<p>");
      return $body;
    }
    return null;
  }
  
  function get_field_references() {
    foreach(drush_get_option_list('vocabularies', array()) as $vocabularystr) {
      $terms = array();
      $vocabulary_map = explode(":", $vocabularystr);
      $field_name = $vocabulary_map[0];
      $vocabulary_name = $vocabulary_map[1];
      foreach(drush_get_option_list($field_name, array()) as $tid) {
        $term_id = is_numeric($tid) ? $tid : drush_createcontent_create_term(trim(urldecode($tid)), $vocabulary_name, true);
        $terms[] = $term_id;
      }
      $this->set_field_reference($field_name, $terms);
    }    

    foreach(drush_get_option_list('fields', array()) as $field_name) {
      $fields = array();
      foreach(drush_get_option_list($field_name, array()) as $field_value) {
        $fields[] = $field_value;
      }
      $this->set_field_reference($field_name, $fields);
    }
  }
}

class InteractiveNodeFactory extends NodeFactory {
  function get_options($node_type) {
      $node_values = parent::get_options($node_type);

      $node_values['title'] = drush_prompt(dt('Please enter the title of the node'));
      $node_values['type'] = drush_prompt(dt('Please enter the content type for your new node'));
      $node_values['uid'] = drush_prompt(dt('Please enter the UID of the user who created the node'));
      $node_values['language'] = drush_prompt(dt('Enter the node language or <enter> for LANGUAGE_NONE'));

      // Unfortunately drush interprets 0 as cancel so we have to present every 0-possible choice as indexed from 1
      $boolean_options = array(1=> 'No', 'Yes');
      $trichoice = array(1=>'Comments disabled', 'Read-only', 'Read/write');

      $node_values['comment'] = drush_choice($trichoice, dt('Are comments on the node enabled?'));
      $node_values['status'] = drush_choice($boolean_options, dt('Is the node published or still a draft?'));
      $node_values['promote'] = drush_choice($boolean_options, dt('Is the node promoted to the front page?'));
      $node_values['sticky'] = drush_choice($boolean_options, dt('Is the node stickied at the top of lists?'));

      // Now we shift the index down, back to 0.
      foreach (array('comment', 'status', 'promote', 'sticky') as $x) {
        $node_values[$x]--;
      }

      $this->body = $this->get_body();

      return $node_values;
  }

  function get_body() {
    if (user_prompt('Would you like to enter a body for the new node?') == "y") {
      $body = array();
      $body['value'] = drush_prompt(dt('Enter the text for the body of the new node'));
      $body['format'] = drush_prompt(dt('What format is the body in? (e.g. \'filtered_html\', \'full_html\', etc.)'));
      $body['safe_value'] = strip_tags($body['value'], "<p>");
      $body['summary'] = $body['safe_summary'] = null;
      if(user_prompt('Would you like to enter a summary for the body of the new node?')) {
        $body['summary'] = drush_prompt(dt('Enter the text for the summary for the body of the new node'));
        $body['safe_summary'] = strip_tags($body['summary'], "<p>");
      }
      return $body;
    }
    return null;
  }

  function get_field_references() {
    while (user_prompt("'Would you like to enter a new field? You will need to know the field machine name. You can also set existing entity references this way if you know the entity ID.")) {
      $field_name = drush_prompt(dt('Enter the field\'s machine name (e.g. \'field_machine_name\')'));
      $field_values = array();
      do {
        $field_values[] = drush_prompt(dt("Enter the value for $field_name."));
      } while (user_prompt("Would you like to enter another value for $field_name?"));
      $this->set_field_reference($field_name, $field_values);
    }

    while (user_prompt("Would you like to enter a new term reference (taxonomy) field? Enter either the term ID or its title. New taxonomy terms will be created if nessessary.")) {
      $field_name = drush_prompt(dt('Enter the term reference field\'s machine name (e.g. \'field_machine_name\')'));
      $field_values = array();
      $vid = null;
      do {
        $term = drush_prompt(dt("Enter a tID or term name for $field_name."));
        if(!is_numeric($term)) {
          if(!$vid) {
            $vid = drush_prompt(dt("Enter a vocabulary vID or its machine name that $term belongs or will belong to"));
          }
          $term = drush_createcontent_create_term($term, $vid);
        }
        $field_values[] = $term;
      } while (user_prompt("Would you like to enter another term for $field_name?"));
      $this->set_field_reference($field_name, $field_values);
    }
  }
}