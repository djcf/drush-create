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

  public static function get_factory($node_type=null) {
    return $node_type ? new AutomatedNodeFactory($node_type) : new InteractiveNodeFactory();
  }

  public static function get_node_wrapper($node_type=null) {
    return NodeFactory::get_factory($node_type)->get_entity();
  }

  function get_field_references() {}

  function get_entity($values=null) {
    if (!$this->entity) {
      if (isset($values['is_new']) && $values['is_new']===false) {
        $this->entity = node_load($values['nid']);
        $this->entity_wrapper = entity_metadata_wrapper('node', $this->entity);
        $cant_update = array('nid', 'is_new', 'uid', 'changed', 'revision_timestamp');
        foreach($values as $key=>$val) {
          #echo "Updating $key (" . gettype($key) . ") with $val (" . gettype($val) . ")";
          if (!in_array($key, $cant_update)) {
            $this->entity_wrapper->$key->set($val);
          }
        }
        # Fix for cant update changed field using entity api:
      } else {
        $this->entity = entity_create('node', $values);
        $this->entity_wrapper = entity_metadata_wrapper('node', $this->entity);
      }
    }
    return $this->entity_wrapper;
  }

  function get_options($node_type) {
    $node_values = array();
    $node_values['nid'] = drush_get_option('nid', null);
    if (!$node_values['nid']) {
      unset($node_values['nid']); // no $nid? $nid not defined: meaning node is new.
    } else {
      while (!is_numeric($node_values['nid'])) {
        $node_values['nid'] = drush_prompt(dt('Please enter a numeric node ID'));
      }
      $node_values['is_new'] = true; // is_new? $nid is defined and node is new: backdate
      if (node_load($node_values['nid'])) {
        $node_values['is_new'] = false; // ! $is_new? $nid is defined and node is not new: update
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

  function set_field_reference($field_name, $value, $is_file = false) {
    #echo PHP_EOL . "setting $field_name to $value". PHP_EOL;
    if ($is_file) {
      if (is_array($value)) {
        $value = $value[0];
      }
      $file = $this->insert_complete_file($value);
      if ($file) {

        $this->get_entity()->$field_name->file->set($file);

        file_usage_add($file, "entity", "node", $this->get_entity()->nid->value());

        foreach (image_styles() as $style => $style_obj) {
          $derivative_uri = image_style_path($style, $file->uri);
          $success        = file_exists($derivative_uri) || image_style_create_derivative($style_obj, $file->uri, $derivative_uri);
          if (!$success) {
            drush_log("Could not create derivate image style $style for $drupalschemeuri in $derivative_uri", "warning");
          }
        }

      } else {
        drush_log("Could not set/update file field $field_name", "warning");
      }
    } else {
      $this->get_entity()->$field_name->set($value);
    }
  }

  function insert_complete_file($filevalues) {
    $filemetamap = explode(":", $filevalues); #"/buildkit/build/CiviCRM/sites/default/files/migrate/$filename";
    $filepath=$filemetamap[0];
    $fileuri=$filemetamap[1];
    $filename = basename($filepath);
    $drupalschemeuri = "public://migrate/$filename";
    #$drupalschemeuri = file_build_uri($filename);
    try {
      $filedata = file_get_contents($filepath);
    } catch (Exception $e) {
      drush_log("Could not open file $filepath", "error");
      return null;
    }
    try {
      $file_obj = file_save_data($filedata, $drupalschemeuri, FILE_EXISTS_REPLACE);
    } catch (Exception $e) {
      drush_log("Could not save file data: $e", "warning");
    }
    if (is_object($file_obj)) {
      try {
        $file_obj->status = FILE_STATUS_PERMANENT;
        $file_obj->type = "image";
        return file_save($file_obj);
      } catch (Exception $e) {
        drush_log("Could not save file metadata into drupal: $e", "warning");
      }
    }
    return null;
  }

  function insert_file_reference($filevalues) {
    $filemetamap = explode(":", $filevalues); #"/buildkit/build/CiviCRM/sites/default/files/migrate/$filename";
    $filepath=$filemetamap[0];
    $fileuri=$filemetamap[1];
    $filename = basename($filepath);

    $file = new stdClass();
    $file->uri = $fileuri;
    $file->filename = basename($filepath);
    $file->filemime = file_get_mimetype($filepath);
    $file->status = FILE_STATUS_PERMANENT;
    $file->display = 1;
    try {
      $f = file_copy($file, "public://$filename");
      $file_obj = file_save($file);
    } catch (Exception $e) {
      drush_log("Could not save file data into drupal", "warning");
    }

    return $file_obj;
  }
}

class AutomatedNodeFactory extends NodeFactory {
  function get_field_references() {
    foreach(drush_get_option_list('vocabularies', array()) as $vocabularystr) {
      $terms = array();
      $vocabulary_map = explode(":", $vocabularystr);
      $field_name = $vocabulary_map[0];
      $is_multi_field = is_array($this->get_entity()->$field_name->raw());
      $vocabulary_name = $vocabulary_map[1];
      foreach(drush_get_option_list($field_name, array()) as $term_identifier) {
        $term_id = is_numeric($term_identifier) ? $term_identifier : drush_createcontent_create_term(trim(urldecode($term_identifier)), $vocabulary_name, true);
        $terms[] = $term_id;
      }
      if(sizeof($terms)>0) {
        if ($is_multi_field) {
          $this->set_field_reference($field_name, $terms);
        } else {
          $this->set_field_reference($field_name, $terms[0]);
        }
      } else {
        drush_log("Could not find any terms for term reference field $field_name in supplied arguments.", "error");
        echo "Make sure a comma-separated list of terms from the taxonomy named '$vocabulary_name' is appended, like this: --$field_name=term1,term2,etc";
      }
    }    

    foreach(drush_get_option_list('fields', array()) as $field_name) {
      #echo "Getting $field_name";
      $fields = array();
      $is_multi_field = is_array($this->get_entity()->$field_name->raw());
      foreach(drush_get_option_list($field_name, array()) as $field_value) {
        $fields[] = $field_value;
      }
      if (sizeof($fields)>0) {
        if ($is_multi_field) {
          $this->set_field_reference($field_name, $fields);
        } else {
          $this->set_field_reference($field_name, $fields[0]);
        }
      }
    }

    foreach(drush_get_option_list('file_references', array()) as $field_name) {
      #echo "*** Getting FILES $field_name ***";
      $file_fields = array();
      $is_multi_field = is_array($this->get_entity()->$field_name->raw());
      foreach(drush_get_option_list($field_name, array()) as $field_value) {
        $file_fields[] = $field_value;
      }
      if (sizeof($file_fields)>0) {
        if ($is_multi_field) {
          $this->set_field_reference($field_name, $file_fields, true);
        } else {
          $this->set_field_reference($field_name, $file_fields[0], true);
        }
      }
    }
  }
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