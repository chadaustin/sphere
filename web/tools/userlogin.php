<?php
$DATE = '$Date: 2001-12-29 03:16:56 $';
$AUTHOR = '$Author: jcore $';

require '../utility.php';

function failure ($reasons) {
  doctype();
  head("sphere - user login");
  echo "<p>Login has failed for the following reason";
  if (count($reasons) > 1)
    echo "s";
  echo "</p>\n";
  foreach ($reasons as $reason)
    echo "<li>$reason</li>\n";
  echo "<ul>\n";
  echo <<<HTML
</ul>
<p>Please go <a href="../login.php">back</a> and try again.</p>
HTML;
  tail();
}

$failed = array();
if ($username == NULL)
  $failed[] = "missing username";
if ($password == NULL)
  $failed[] = "missing password";
if (count($failed) > 0)
  failure($failed);
else {
  $conn = new MySQLConnection;
  $cookie = $conn->validateUser($username, $password);
  if (count($cookie) == 2) {
    /* set login cookies for 1 day */
    $time = time() + 86400;
    setcookie("sphereusername", $cookie['username'], $time, "/");
    setcookie("spherepassword", $cookie['password'], $time, "/");
    header("Code: 302");
    header("Message: Found");
    header("Location: $SITEROOT/");
  } else {
    failure(array("$cookie"));
  }
  $conn->cleanup();
}
?>
