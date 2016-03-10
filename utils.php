<?php
function make_ints($val) {
    if (is_array($value)) {
      return array_map($value);
    }
	return is_numeric($value) ? intval($value) : $value;
}

// Does the term exist already? Don't create it again.
function term_exists($term_name, $vocabulary) {
  foreach(taxonomy_get_tree($vocabulary->vid) as $term) {
    if ($term->name == $term_name) {
      return $term;
    }
  }
  return false;
}

function user_prompt($msg) {
  user_prompt() = drush_prompt(dt($msg . " (Enter 'y' if so)"));
  if (user_prompt()=="y")
    return true;
  return false;
}
