<?php
$SITEROOT = "";
$DOCUMENTROOT = "/home/groups/s/sp/sphere/htdocs";

require("$DOCUMENTROOT/mysql.php");

/* keep icon filenames to less than 64 characters in length */
$ICONS = array(
'article' => 'article.png',
'announcement' => 'announcement.png',
'game' => 'game.png',
'outside' => 'outside.png',
'project' => 'project.png'
);

function fatal_error ($brief, $message) {
  die("fatal error '$brief': $message");
}

function doctype () {
  echo <<<HTML
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "DTD/xhtml1-strict.dtd">
HTML;
}

function head ($title) {
  global $SITEROOT;
  global $sphereusername;
  global $spherepassword;
  if ($sphereusername != NULL)
    $message = "welcome $sphereusername (<a href=\"$SITEROOT/tools/userlogout.php\">logout</a>)";
  else
    $message = "you are not logged in (<a href=\"$SITEROOT/login.php\">login</a> or <a href=\"$SITEROOT/register.php\">register</a>)";
  echo <<<HTML
<html>
<head>
  <title>$title</title>
  <link rel="stylesheet" type="text/css" title="graphics" href="$SITEROOT/graphics.css" />
</head>
<body>
  <table class="main" cellspacing="0" cellpadding="0">
  <tr>
    <td class="menu">
      <table class="menu" cellspacing="0" cellpadding="0">
      <tr>
        <td class="menuleft">
          <a href="$SITEROOT/">news</a>
          <a href="$SITEROOT/about.php">about</a>
          <a href="http://sourceforge.net/project/showfiles.php?group_id=1551">download</a>
          <a href="$SITEROOT/games/">games</a>
          <a class="last" href="http://aegisknight.org/forum/list.php?f=4/">forum</a>
        </td>
        <td class="menucenter">
          <img src="$SITEROOT/images/title.png" alt="sphere.sourceforge.net logo" /><br />
          $message
        </td>
        <td class="menuright">
          <a href="">reserved</a>
          <a href="">reserved</a>
          <a href="">reserved</a>
          <a href="http://sourceforge.net/projects/sphere">sourceforge</a>
          <a class="last" href="">reserved</a>
        </td>
      </tr>
      </table>
    </td>
  </tr>
  <tr>
    <td class="body">
HTML;
}

/* $DATE and $AUTHOR should be set to the respective CVS keyword expansions before calling tail() */
function tail () {
  global $DATE;
  global $AUTHOR;
  $moddate = substr($DATE, 7, 10);
  $modtime = substr($DATE, 18, 8);
  $modauthor = substr($AUTHOR, 9, -2);
  echo <<<HTML
    </td>
  </tr>
  <tr>
    <td class="tail">
      <div class="references">
        <a href="http://validator.w3.org/check/referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0!" height="31" width="88" /></a>
        <a href="http://jigsaw.w3.org/css-validator/check/referer"><img style="border:0;width:88px;height:31px" src="http://jigsaw.w3.org/css-validator/images/vcss" alt="Valid CSS!" /></a>
        <a href="http://sourceforge.net"><img src="http://sourceforge.net/sflogo.php?group_id=873&amp;type=1" alt="sourceforge logo" /></a>
      </div>
      <p class="notes">
        Last Modified on $moddate at $modtime by $modauthor
      </p>
    </td>
  </tr>
  </table>
</body>
</html>
HTML;
}

function generateBox ($type, $title, $body) {
  return <<<BOX

      <table class="$type">
      <tr>
        <td class="head">$title</td>
      </tr>
      <tr>
        <td class="body">
$body
        </td>
      </tr>
      </table>

BOX;
}

function generatePane ($side, $body) {
  return <<<PANE
      <table class="$side" cellpadding="0">
      <tr>
        <td>
$body
        </td>
      </table>
PANE;
}

/* if sphereusername and spherepassword cookies are set we need to make sure that they're valid */
if ($sphereusername != NULL or $spherepassword != NULL) {
  if ($sphereusername == NULL) {
    fatal_error("sphereusername cookie not set", "The spherepassword cookie is set, but the sphereusername cookie is not.  Please delete the spherepassword cookie and attempt to login again.");
  }
  if ($spherepassword == NULL) {
    fatal_error("spherepassword cookie not set", "The sphereusername cookie is set, but the spherepassword cookie is not.  Please delete the sphereusername cookie and attempt to login again.");
  }
  $conn = new MySQLConnection;
  if (!$conn->verify($sphereusername, $spherepassword)) {
    fatal_error("invalid password", "The sphereusername and spherepassword cookies are set, but they are not consistent with our database.  Please clear the sphereusername and spherepassword cookies and attempt to login again.");
  }
}

?>
