<?php
$DATE = '$Date: 2001-12-29 03:16:56 $';
$AUTHOR = '$Author: jcore $';

require('../utility.php');

function accessDenied () {
  echo <<<HTML
<p>You do not have access to this area.</p>
HTML;
}

doctype();
head("sphere - news articles admin");
$conn = new MySQLConnection;
$user = $conn->getUser($sphereusername);
if ($user) {
  if ($user->level & 1) { /* news bit */
    if ($articleicon and $articletitle and $articletext) {
      $result = $conn->addNewsArticle($user->username, $articleicon, $articletitle, $articletext);
      if ($result)
        echo "<p>Error: $result</p>";
      else
        echo "<p>News article submitted successfully</p>";
    } else {
      $icon_options = "";
      foreach ($ICONS as $icon_name => $dummy) {
        $icon_options = $icon_options . "            <option>$icon_name</option>\n";
      }
      $input = <<<INPUT
        <form action="addnews.php" method="post">
        <div>
          <label for="articleicon">icon</label><br />
          <select name="articleicon">
$icon_options          </select><br />
          <label for="articletitle">title</label><br />
          <input class="full" type="text" name="articletitle" maxlength="255" /><br />
          <label for="articletext">text</label><br />
          <textarea class="full" name="articletext" rows="15"></textarea><br />
          <hr>
          <input type="submit" />
        </div>
        </form>
INPUT;
      echo generateBox("box", "news details", $input);
    }
  } else
    accessDenied();
} else
  accessDenied();
  
tail();
?>
