<?php
$DATE = '$Date: 2001-12-29 03:16:56 $';
$AUTHOR = '$Author: jcore $';

require('../utility.php');

doctype();
head("sphere - user registration");

function failure ($reasons) {
  echo "<p>Registration has failed for the following reason";
  if (count($reasons) > 1)
    echo "s";
  echo "</p>\n";
  foreach ($reasons as $reason)
    echo "<li>$reason</li>\n";
  echo "<ul>\n";
  echo <<<HTML
</ul>
<p>Please go <a href="../register.php">back</a> and try again.</p>
HTML;
}

/* validate the input */
$failed = array();
$illegal = "\s'";
if ($username == NULL)
  $failed[] = "missing username";
if (preg_match("/\s/", $username))
  $failed[] = "username contains whitespace";
if (strlen($username) > 32)
  $failed[] = "username is too long";
if ($password == NULL)
  $failed[] = "missing password";
if (preg_match("/\s/", $password))
  $failed[] = "password contains whitespace";
if (strlen($password) > 12)
  $failed[] = "password is too long";
if ($verifypass == NULL)
  $failed[] = "missing password verification";
if ($password != $verifypass)
  $failed[] = "passwords do not match";
if (preg_match("/\s/", $email))
  $failed[] = "email address contains whitespace";
if (strlen($email) > 255)
  $failed[] = "email address is too long";
if (strlen($firstname) > 64)
  $failed[] = "first name is too long";
if (strlen($lastname) > 64)
  $failed[] = "last name is too long";
if ($year != NULL and $year < 1900 or $year > 2002)
  $failed[] = "birthdate year is out of range (1900-2002)";
if ($month != NULL and $month < 1 or $month > 12)
  $failed[] = "birthdate month is out of range (1-12)";
if ($day != NULL and $day < 1 or $day > 31)
  $failed[] = "birthdate day is out of range (1-31)";
if ($photo == "http://")
  $photo = NULL;
if (preg_match("/[$illegal]/", $photo))
  $failed[] = "photo URL contains illegal characters";
if (strlen($photo) > 255)
  $failed[] = "photo URL is too long";
if ($website == "http://")
  $website = NULL;
if (preg_match("/[$illegal]/", $website))
  $failed[] = "website URL contains illegal characters";
if (strlen($website) > 255)
  $failed[] = "website URL is too long";
$firstname = trim($firstname);
$lastname = trim($lastname);

if (count($failed) > 0) {
  failure($failed);
} else {
  $conn = new MySQLConnection;
  if ($conn->getUser($username)) {
    failure(array("username is already in use"));
  }
  else {
    if ($conn->addUser($username, $password, $email, $firstname, $lastname, $year, $month, $day, $photo, $website, $notify))
      echo <<<HTML
<p>Congratulations!  You are now a registered member of sphere.sourceforge.net.</p>
<p>You may proceed to <a href="../login.php">login</a>.</p>
HTML;
    else
      failure(array("MySQL failure"));
  }
  $conn->cleanup();
}

tail();
?>
