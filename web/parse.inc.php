<?php

class data_object {
  var $name;
  var $type;
  var $data;
}



function parse_data_file ($file, $parse = false, $debug = false) {
  $fp = fopen ($file, "rb");
  if (!$fp) {
    echo ("$file cannot be opened for read");
    return false;
  }

  $data = fread($fp, filesize($file));
  if ($debug) echo ($data);

  fclose ($fp);

  $blah = parse_data ($data, $parse);
  return $blah;
}



function parse_data ($data, $parse = false) {
  $count = strpos (" " . $data, $parse);
  if (!$count) return false;

  if ($parse) $data = split($parse, $data);
  else $data = split("\n", $data);

  $theArray = array();
  $x = 0;

  for ($i = 0; $i < count($data); ++$i) {
    $data[$i] = trim ($data[$i]);

    if (strlen($data[$i])) {
      $count = strpos (" " . $data[$i], "\r");
      $count2 = strpos(" " . $data[$i], "\n");
      $blah = ($count > $count2) ? $count - 2: $count2 - 1;
      if ($blah) {
        if ($parse) $blah2 = strpos(substr($data[$i], 0, $blah), " ");
        else $blah2 = 0;
        $name = substr($data[$i], 0, ($blah2) ? $blah2 : $blah);
        $stuff = substr($data[$i], ($blah2) ? $blah2 + 1 : $blah + 2, strlen($data[$i]));
        if ($parse) {
          $theArray[$x] = new data_object;
          $theArray[$x]->name = trim ($name);
          $theArray[$x]->data = $stuff;
          $theArray[$x]->type = $parse;
        }
        else {
          $theArray[$x] = new data_object;
          $theArray[$x]->name = "<null>";
          $theArray[$x]->data = trim ($name);
          $theArray[$x]->type = "<undefined>";
        }
        ++$x;
      }
    }
  }
  return $theArray;
}



function find_data ($data, $label, $info = false) {
  $info = strtolower($info);
  $label = strtolower($label);
  $result = array();

  $x = 0;
  for ($i = 0; $i < count($data); ++$i) {
    $okay = false;

    if (!$label || $label == strtolower($data[$i]->name)) {
      if ($info) {
        if (strpos(" " . strtolower($data[$i]->data), $info)) $okay = true;
      }
      elseif ($label == strtolower($data[$i]->name)) $okay = true;
    }

    if ($okay) {
      $result[$x] = $data[$i];
      ++$x;
    }
  }

  return (count($result)) ? $result : false;
}

?>
