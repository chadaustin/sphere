<?php
$DATE = '$Date: 2001-12-29 03:16:56 $';
$AUTHOR = '$Author: jcore $';

require('utility.php');

doctype();
head("sphere - news");

$conn = new MySQLConnection;
/* get articles from the last 10 days, or a maximum of 10 articles */
$startdate = date("Y-m-d H:i:s", time() - (60 * 60 * 24 * 10));
$enddate = date("Y-m-d H:i:s", time());
$articles = $conn->getNewsArticles($startdate, $enddate, 10);
$left = "";
$counter = 0;
foreach ($articles as $article) {
  $counter++;
  $head = <<<HEAD
<a name="$counter" />
<img src="$SITEROOT/images/icons/{$ICONS[$article->icon]}" alt="{$article->icon}" />
<b>{$article->date}</b>: {$article->title}
HEAD;
  $author = $article->author;
  $body = <<<BODY
{$article->text}
<div class="sig">- <a href="tools/showuser.php?username=$author">$author</a></div>
BODY;
  $left .= generateBox("news", $head, $body);
}

$right = "";
$counter = 0;
$body = "";
foreach ($articles as $article) {
  $counter++;
  $body .= <<<BODY
<a href="#$counter">{$article->title}</a><br />
BODY;
}
$right .= generateBox("box", "latest news", $body);

if ($sphereusername) {
  $user = $conn->getUser($sphereusername);
  $youraccount = <<<TEXT
Hello {$user->username}.  Your user level is {$user->level}.
TEXT;
  if ($user->level & 1)
    $youraccount .= '<br />You may <a href="admin/addnews.php">add a new news article</a>.';
}
else {
  $youraccount = <<<TEXT
You can register for a free account on sphere.sourceforge.net.  The account system doesn't do anything useful yet unless you have an elevated user level.
TEXT;
}
$youraccount .= "<br />Please report any bugs in the new user system to jcore.";
$right .= generateBox("box", "your account", $youraccount);

echo generatePane("left", $left);
echo generatePane("right", $right);

tail();
?>
