<?php
$DATE = '$Date: 2001-12-29 05:44:04 $';
$AUTHOR = '$Author: jcore $';

require('utility.php');
doctype();
head("sphere - user registration");

$input = <<<INPUT
        <form action="$SITEROOT/tools/useradd.php" method="post">
        <div>
          <label>username *</label><br />
          <input type="text" name="username" size="33" maxlength="32" /><br />
          <label>password *</label><br />
          <input type="password" name="password" size="13" maxlength="12" /><br />
          <label>verify password *</label><br />
          <input type="password" name="verifypass" size="13" maxlength="12" /><br />
          <label>email address</label><br />
          <input type="text" name="email" size="40" maxlength="255" /><br />
          <label>first name</label><br />
          <input type="text" name="firstname" size="20" maxlength="64" /><br />
          <label>last name</label><br />
          <input type="text" name="lastname" size="20" maxlength="64" /><br />
          <label>birthdate</label><br />
          <label>year</label>
          <input type="text" name="year" size="5" maxlength="4" />
          <label>month</label>
          <input type="text" name="month" size="3" maxlength="2" />
          <label>day</label>
          <input type="text" name="day" size="3" maxlength="2" /><br />
          <label>photo URL (square aspect ratio)</label><br />
          <input type="text" name="photo" size="50" value="http://" maxlength="255" /><br />
          <label>website URL</label><br />
          <input type="text" name="website" size="50" value="http://" maxlength="255" /><br />
          <label>I would like to receive email notifications about new sphere developments</label>
          <input type="checkbox" name="notify" /><br />
          <hr />
          <input type="submit" />
        </div>
        </form>
INPUT;

$notes = <<<NOTES
        All entries are case-sensitive<br /><br />
        Fields marked with an asterisk (*) are required.<br /><br />
        Your email address is necessary if you forget your password and need us to mail it to you.  We will not give your email address out to anyone (view our policy statement <a href="$SITEROOT/policy.php">here</a>), but if you feel uncomfortable giving it to us you may omit it.<br /><br />
        Your password is viewable to anyone who has access to our MySQL database (all members of the sphere sourceforge project).  The password is encrypted with only a simple crypt() (with a known salt) so we suggest that you don't use a valuable password.
NOTES;

echo generatePane("left", generateBox("box", "new user registration", $input));
echo generatePane("right", generateBox("box", "some things to keep in mind", $notes));

tail();
?>
