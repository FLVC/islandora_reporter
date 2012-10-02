<?php
/**
 * @file
 *   Holds helper function for islandora_reporter module.
 * @todo
 *   optimize number of queries
 */

/**
 * This function wraps the print module's print_insert_link function.
 * It lets on specify the visible text of the link.
 *
 * @param string $path
 *   The path to create the printer friendly link for.
 * @param string $label
 *   The text to show for the link
 *
 * @return markup
 *   The button
 */
function islandora_reporter_print_insert_link($path, $label = NULL) {
  $print_button = print_insert_link($path);

  // Modify the text if provided.
  if (!is_null($label)) {
    $print_button_DOMDoc = DOMDocument::loadHTML($print_button);
    $anchors = $print_button_DOMDoc->getElementsByTagName('a');
    $anchors->item(0)->nodeValue = $label;
    // Make sure to return the parent span.
    $print_button = $print_button_DOMDoc->saveXML($anchors->item(0)->parentNode);
  }

  return $print_button;
}


/**
 * This function will return all queries from the database.
 *
 * @param string $content_model
 *   If supplied the results will be limited to
 *   those related to the content model.
 *
 * @return array
 *   $queries all the queries in the database.
 *   $queries[] = array('query_index' => 'query_index',
 *     'query_name' => 'query_name')
 */
function islandora_reporter_get_all_queries($content_model = NULL) {
  $queries = array();

  if (isset($content_model)) {
    $DB_query = "SELECT {islandora_reporter_queries}.query_index, query_name FROM {islandora_reporter_queries}
    INNER JOIN {islandora_reporter_queries_to_CM} ON {islandora_reporter_queries}.query_index={islandora_reporter_queries_to_CM}.query_index
    WHERE content_model='$content_model'";
  }
  else {
    $DB_query = 'SELECT query_index, query_name FROM {islandora_reporter_queries}';
  }


  $resource = db_query($DB_query);

  while ($row = db_fetch_array($resource)) {
    $queries[] = $row;
  }

  return $queries;
}

/**
 * This function will return all the info
 * about a query stored in the database.
 *
 * @param integer $query_index
 *   The index of the query to retrun.
 *
 * @return array
 *   all info in the database about the query
 *   $query_info('name' => '',
 *               'query'=> '',)
 *               'returns' => array(),)
 *               'pregs' => array(),)
 *               'content_models' => array(),)
 */
function islandora_reporter_get_full_query_info($query_index) {
  $query_info = array();

  $get_query_results = islandora_reporter_get_query($query_index);
  $get_pregs_results = islandora_reporter_get_pregs_and_labels($query_index);
  $get_returns_results = islandora_reporter_get_returns_and_labels($query_index);
  $get_CMs_results = islandora_reporter_get_CMs_and_labels($query_index);

  $query_info['name'] = $get_query_results['query_name'];
  $query_info['query'] = $get_query_results['query'];
  $query_info['pregs'] = $get_pregs_results;
  $query_info['returns'] = $get_returns_results;
  $query_info['content_models'] = $get_CMs_results;

  return $query_info;
}

/**
 * This function will set all the info
 * about a query stored in the database.
 *
 * @param array $query_info
 *   all info in the database about the query
 *   $query_info('name' => '',
 *               'query'=> '',)
 *               'returns' => array(),)
 *               'pregs' => array(),)
 *               'content_models' => array(),)
 *
 * @param integer $query_index
 *   The unique key for the query.
 */
function islandora_reporter_add_full_query_info($query_info, $query_index = NULL) {

  $query_index = islandora_reporter_add_query($query_info['query'], $query_info['name'], $query_index);

  // Remove other related database entries.
  islandora_reporter_remove_returns_and_labels($query_index);
  islandora_reporter_remove_pregs_and_labels($query_index);
  islandora_reporter_remove_CMs_and_labels($query_index);

  if (isset($query_info['returns'])) {
    islandora_reporter_add_returns_and_labels($query_index, $query_info['returns']);
  }
  if ($query_info['pregs']) {
    islandora_reporter_add_pregs_and_labels($query_index, $query_info['pregs']);
  }
  if ($query_info['content_models']) {
    islandora_reporter_add_CMs_and_labels($query_index, $query_info['content_models']);
  }

}

/**
 * This function will return a query stored in the database
 *
 * @param integer $query_index
 *   The index of the query to retrun.
 *
 * @return string
 *   $query the query from the database
 *   array(
 *   'query' =>'query',
 *   'query_name' => 'query_name')
 */
function islandora_reporter_get_query($query_index) {
  $query = array();

  $DB_query = "SELECT query, query_name FROM {islandora_reporter_queries} WHERE query_index='$query_index'";
  $resource = db_query($DB_query);

  while ($row = db_fetch_array($resource)) {
    $query[] = $row;
  }

  // Only one result.
  return $query[0];
}

/**
 * This function will add a query to the database.
 *
 * @param string $query
 *   The query to store in the database.
 * @param string $query_name
 *   The human label name for the query.
 * @param integer $query_index
 *   The query index if this is an update.
 *
 * @return integer
 *   The query_index that the new query is at.
 */
function islandora_reporter_add_query($query, $query_name, $query_index = NULL) {
  $update = NULL;

  $object = array('query' => $query, 'query_name' => $query_name);
  if (isset($query_index)) {
    $update = 'query_index';
    $object['query_index'] = (string) $query_index;
    drupal_write_record('islandora_reporter_queries', $object, 'query_index');
  }
  else {
    drupal_write_record('islandora_reporter_queries', $object);
  }

  return $object['query_index'];
}

/**
 * This function will delete a query from the database.
 * It will also remove any related information in the database.
 *
 * @param integer $query_index
 *   The index of the query to delete.
 */
function islandora_reporter_remove_query($query_index) {

  // Remove the query from the database.
  $query = "DELETE FROM {islandora_reporter_queries} WHERE query_index='$query_index'";
  db_query($query);

  // Remove other related database entries.
  islandora_reporter_remove_returns_and_labels($query_index);
  islandora_reporter_remove_pregs_and_labels($query_index);

}

/**
 * This function will retrieve return value to label mappings
 * from the database for a particular query.
 *
 * @param integer $query_index
 *   The identifier of the query to get the mapping for.
 *
 * @return array
 *   $mapping The mapping of query returns to labels. array('return'=>'label')
 */
function islandora_reporter_get_returns_and_labels($query_index) {
  $mapping = array();

  $DB_query = "SELECT return_string, return_label FROM {islandora_reporter_query_returns_to_label} WHERE query_index='$query_index'";
  $resource = db_query($DB_query);

  while ($row = db_fetch_array($resource)) {
    $mapping[$row['return_string']] = $row['return_label'];
  }

  return $mapping;

}

/**
 * This function will add the returns and labels mapping.
 *
 * @param integer $query_index
 *   The identifier of the query.
 * @param array $mapping
 *   The mapping of query returns to labels. array('return'=>'label')
 */
function islandora_reporter_add_returns_and_labels($query_index, $mapping) {
  foreach ($mapping as $return_string => $return_label) {
    $object = array(
      'return_string' => $return_string,
      'return_label' => $return_label,
      'query_index' => $query_index,
    );
    drupal_write_record('islandora_reporter_query_returns_to_label', $object);
  }
}

/**
 *  This function will remove entries in the
 *  islandora_reporter_query_returns_to_label table that are related to a query.
 *
 * @param integer $query_index
 *   The identifier of the query.
 */
function islandora_reporter_remove_returns_and_labels($query_index) {
  // Remove the mapping from the database.
  $query = "DELETE FROM {islandora_reporter_query_returns_to_label} WHERE query_index='$query_index'";
  db_query($query);
}

/**
 * This function will retrieve the preg strings
 * and labels for them from the database.
 *
 * @param integer $query_index
 *   The identifier of the query.
 *
 * @return array
 *   $mapping The mapping of pregs to labels. array('preg'=>'label')
 */
function islandora_reporter_get_pregs_and_labels($query_index) {
  $mapping = array();

  $DB_query = "SELECT preg, preg_label FROM {islandora_reporter_query_preg_to_label} WHERE query_index='$query_index'";
  $resource = db_query($DB_query);

  while ($row = db_fetch_array($resource)) {
    $mapping[$row['preg']] = $row['preg_label'];
  }

  return $mapping;
}

/**
 * This function will add the pregs and labels mapping.
 *
 * @param integer $query_index
 *   The identifier of the query.
 * @param array $mapping
 *   The mapping of query pregs to labels. array('preg'=>'label')
 */
function islandora_reporter_add_pregs_and_labels($query_index, $mapping) {
  foreach ($mapping as $preg => $preg_label) {
    $object = array(
      'preg' => $preg,
      'preg_label' => $preg_label,
      'query_index' => $query_index,
    );
    drupal_write_record('islandora_reporter_query_preg_to_label', $object);
  }
}

/**
 *  This function will remove entries in the
 *  islandora_reporter_query_preg_to_label table that are related to a query.
 *
 * @param integer $query_index
 *   The identifier of the query.
 */
function islandora_reporter_remove_pregs_and_labels($query_index) {
  // Remove the mapping from the database.
  $query = "DELETE FROM {islandora_reporter_query_preg_to_label} WHERE query_index='$query_index'";
  db_query($query);
}

/**
 * This function will retrieve the content model strings
 * and labels for them from the database.
 *
 * @param integer $query_index
 *   The identifier of the query.
 *
 * @return array
 *   $mapping The mapping of content_models to labels
 *   array('content_model'=>'label')
 */
function islandora_reporter_get_CMs_and_labels($query_index) {
  $mapping = array();

  $DB_query = "SELECT content_model, content_model_label FROM {islandora_reporter_queries_to_CM} WHERE query_index='$query_index'";
  $resource = db_query($DB_query);

  while ($row = db_fetch_array($resource)) {
    $mapping[$row['content_model']] = $row['content_model_label'];
  }

  return $mapping;
}

/**
 * This function will add the content models and labels mapping.
 *
 * @param integer $query_index
 *   The identifier of the query.
 * @param array $mapping
 *   The mapping of content models to labels. array('preg'=>'label')
 */
function islandora_reporter_add_CMs_and_labels($query_index, $mapping) {
  foreach ($mapping as $content_model => $label) {
    $object = array(
      'content_model' => $content_model,
      'content_model_label' => $label,
      'query_index' => $query_index,
    );
    drupal_write_record('islandora_reporter_queries_to_CM', $object);
  }
}

/**
 *  This function will remove entries in the
 *  islandora_reporter_queries_to_CM table that are related to a query.
 *
 * @param integer $query_index
 *   The identifier of the query.
 */
function islandora_reporter_remove_CMs_and_labels($query_index) {
  // Remove the mapping from the database.
  $query = "DELETE FROM {islandora_reporter_queries_to_CM} WHERE query_index='$query_index'";
  db_query($query);
}
