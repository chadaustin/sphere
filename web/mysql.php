<?php
require("$DOCUMENTROOT/passwords.inc.php");

class MySQLConnection {
  var $HOST = "localhost";
  var $USER = "jcore";
  var $DATABASE = "sphere";

  var $connection;

  function MySQLConnection () {
    global $MYSQL_PASSWORD;
    $this->connection = mysql_connect($this->HOST, $this->USER, $MYSQL_PASSWORD) or die("could not connect to MySQL server on {$this->HOST}");
    mysql_select_db($this->DATABASE) or die("could not select database {$this->DATABASE}");
  }

  function cleanup () {
    mysql_close($this->connection);
  }

  function query ($string) {
    $result = mysql_query($this->string, $this->connection);
    return($result);
  }

  function addUser ($username, $password, $email, $firstname, $lastname, $year, $month, $day, $photo, $website, $notify) {
    mysql_query("CREATE TABLE IF NOT EXISTS users (username VARCHAR(32), password VARCHAR(32), email VARCHAR(255), firstname VARCHAR(64), lastname VARCHAR(64), birthdate DATE, photo VARCHAR(255), website VARCHAR(255), notify TINYINT, level TINYINT, created DATETIME)", $this->connection);
    $password = crypt($password, "jc");
    if ($notify)
      $notify = 1;
    $result = mysql_query("INSERT INTO users VALUES ('$username', '$password', '$email', '$firstname', '$lastname', '$year-$month-$day', '$photo', '$website', '$notify', '0', NOW())");
    $error = mysql_error();
    if ($error != '') {
      echo "<p>MySQL error: $error</p>\n";
      return FALSE;
    } else
      return TRUE;
  }

  function getUser ($username) {
    $result = mysql_query("SELECT * FROM users WHERE username = '$username'", $this->connection);
    if (mysql_error() != '')
      return FALSE;
    if (mysql_num_rows($result) == 1)
      return (mysql_fetch_object($result));
    else
      return FALSE;
  }

  /* $password is not encrypted */
  function validateUser ($username, $password) {
    $result = mysql_query("SELECT username, password FROM users WHERE username = '$username'", $this->connection);
    if (mysql_error() != '')
      return mysql_error();
    if (mysql_num_rows($result) != 1)
      return "user '$username' not found";
    $user = mysql_fetch_object($result);
    $password = crypt($password, "jc");
    if ($password == $user->password) {
      return array('username' => $user->username, 'password' => $user->password);
    } else
      return "incorrect password for '$username'";
  }

  /* $password is encrypted */
  function verify ($username, $password) {
    $result = mysql_query("SELECT username, password FROM users WHERE username = '$username'", $this->connection);
    if (mysql_error() != '' or mysql_num_rows($result) != 1)
      return FALSE;
    $user = mysql_fetch_object($result);
    if ($password != $user->password)
      return FALSE;
    return TRUE;
  }

  function addNewsArticle ($author, $icon, $title, $text) {
    mysql_query("CREATE TABLE IF NOT EXISTS news (date DATETIME, author VARCHAR(32), icon VARCHAR(64), title VARCHAR(255), text TEXT)");
    $result = mysql_query("INSERT INTO news VALUES (NOW(), '$author', '$icon', '$title', '$text')");
    if (mysql_error() != '')
      return mysql_error();
    return FALSE;
  }

  function getNewsArticles ($startdate, $enddate, $maxarticles) {
    $result = mysql_query("SELECT * FROM news WHERE date BETWEEN '$startdate' AND '$enddate' ORDER BY date DESC LIMIT $maxarticles");
    $articles = array();
    while ($row = mysql_fetch_object($result)) {
      $articles[] = $row;
    }
    return $articles;
  }
}
?>
