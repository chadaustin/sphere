<?php
require('../utility.php');

/* set cookie one day in the past */
global $sphereusername;
global $spherepassword;

if ($sphereusername != NULL and $sphereusername != NULL) {
  $time = time() - 86400;
  setcookie("sphereusername", $sphereusername, $time, "/");
  setcookie("spherepassword", $spherepassword, $time, "/");
}
header("Code: 302");
header("Message: Found");
header("Location: $SITEROOT/");
?>
