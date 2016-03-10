<?php
function make_ints($val) {
  if (is_array($val)) {
    return array_map('intval', $val);
  }
  return is_numeric($val) ? intval($val) : $val;
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
  return (drush_prompt(dt($msg . " (Enter 'y' if so)")) == "y");
}
