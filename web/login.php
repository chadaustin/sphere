<?php
$DATE = '$Date: 2001-12-29 05:44:04 $';
$AUTHOR = '$Author: jcore $';

require('utility.php');

doctype();
head("sphere - user login");
if ($sphereusername) {
  echo "<p>You are already logged in</p>";
} else {
  $input = <<<INPUT
        <form action="$SITEROOT/tools/userlogin.php" method="post">
        <div>
          <label>username</label><br />
          <input type="text" name="username" size="33" maxlength="32" /><br />
          <label>password</label><br />
          <input type="password" name="password" size="13" maxlength="12" /><br />
          <hr />
          <input type="submit" />
        </div>
        </form>
INPUT;

  $notes = <<<NOTES
        Remember that your username and password are case-sensitive.<br /><br />
        Your password will be transmitted in plaintext.<br /><br />
        You must have cookies enabled for this site.  We will only set one cookie for your username and one for your password (view our policy <a href="policy.php">here</a>).
NOTES;

  echo generatePane("left", generateBox("box", "login information", $input));
  echo generatePane("right", generateBox("box", "notes", $notes));
}

tail();
?>
