<?
$__DATABASE = "sphere";


function start(){
  include("/home/groups/s/sp/sphere/htdocs/doctype.inc.php"); 
  echo '
  <html>
    <head>
      <title>sphere.sourceforge.net</title>
      <link rel="stylesheet" type="text/css" href="http://sphere.sourceforge.net/utility.css" />
    </head>
    <body>
      <table cellpadding="0" cellspacing="0" class="main">
        <tr>
          <td class="topbar">

            <table cellpadding="0" cellspacing="0" class="topmenu">
              <tr>
                <td class="menuitems">
                  <a class="headleft"  href="http://sphere.sourceforge.net/index.php">news</a>
                  <a class="headleft"  href="http://sphere.sourceforge.net/index.php?U_ACT=articles">articles</a>
                  <a class="headleft"  href="http://sphere.sourceforge.net/dev.php?U_ACT=current">download</a>
                  <a class="headleft"  href="http://sphere.sourceforge.net/dev.php?U_ACT=log">dev log</a>
                  <a class="headleft"  style="border-bottom: none" href="http://sphere.sourceforge.net/index.php?U_ACT=forums">forums</a>
                </td>

                <td class="menulogo">
                  <img src="http://sphere.sourceforge.net/images/title.png" alt="sphere.sourceforge.net logo" /><br />
                  <form id="login" method="post" action="http://sphere.sourceforge.net/account.php?U_ACT=login">
                    <div>
                      <input type="text" name="login_name" size="8" />
                      <a href="javascript:document.getElementById(\'login\').submit()"> login</a> or <a href="http://sphere.sourceforge.net/account.php?U_ACT=register">register</a> (<a href="http://sphere.sourceforge.net/account.php?U_ACT=forgot">forgot password?</a>)
                    </div>
                  </form>
                </td>
                
                <td class="menuitems">
                  <a class="headright" href="http://sphere.sourceforge.net/projects.php?U_ACT=list">projects</a>
                  <a class="headright" href="http://sphere.sourceforge.net/dev.php?U_ACT=about">about sphere</a>
                  <a class="headright" href="http://sphere.sourceforge.net/links.php">links</a>
                  <a class="headright" href="http://sphere.sourceforge.net/index.php?U_ACT=map">site map</a>
                  <a class="headright" style="border-bottom: none" href="http://sphere.sourceforge.net/contact.php">contact</a>
                </td>
              </tr>
            </table>

          </td>
        </tr>
      <tr>
        <td class="body">
          THIS SITE IS CURRENTLY UNDERGOING CONSTRUCTION. IT WILL MOST LIKELY HAVE A BILLION ERRORS.';
}


function conclude(){
  echo '
          /td>
	</tr>
      </table>
    </body>
  </html>';
}

function news_item($_ID, $_TOPIC, $_DATE, $_AID, $_ICON, $_BODY){
  $AEMAIL = GetCellFromUID($_AID, 'EMAIL');
  $AUTHOR = GetCellFromUID($_AID, 'NAME');

	echo "
          <table class=\"item\">
            <tr>
              <td class=\"itemhead\">
                <img src=\"images/icons/$_ICON.png\" style=\"vertical-align: middle\" alt=\"[ $_ICON ]\" /> $_TOPIC (<a href=\"http://sphere.sourceforge.net/index.php?U_ACT=reply&amp;rid=$_ID\">entire thread</a>)
              </td>
            </tr>
            <tr>
              <td class=\"itembody\">
                $_BODY
                <br />
                <br />
                <div class=\"itembodyauthor\"> - <a href=\"mailto:$AEMAIL\">$AUTHOR</a> ($_DATE) </div>
              </td>
            </tr>
          </table>";
}

function important_links(){
  echo '
    <br />
    <div class="linkbar">
      <a href="http://validator.w3.org/check/referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0!" height="31" width="88" /></a>
      <a href="http://jigsaw.w3.org/css-validator/check/referer"><img style="border:0;width:88px;height:31px" src="http://jigsaw.w3.org/css-validator/images/vcss" alt="Valid CSS!" /></a>
      <a href="http://sourceforge.net"><img src="http://sourceforge.net/sflogo.php?group_id=873&amp;type=1" alt="sourceforge logo" /></a>
      <br /><span class="smalltext">please note, if you have a user account and are logged in, that this page may not verify with css depending on your theme. in your user preferences, when changing themes, it will be noted if a theme is css valid or not. we encourage you to use <a href="http://w3.org/style/css/">correct css</a> when creating a stylehseet.<br />
      also, this site most likely has display problems in 4.0 browsers. Use a 6.0 browser and, preferrably, use a browser based off of mozilla :)</span><br />
      <a href="http://kmeleon.org">get k-meleon</a> | <a href="http://mozilla.org">get mozilla</a>
    </div>';
}

function mconnect(){ //What can I say? I'm lazy.
  mysql_connect("mysql.sourceforge.net", "sphere", "crothosu") or die("could not connect to the mysql database. contact malis@wolf-phantom.net");
}

function GetCellFromUID($_ID, $_CELL){
  global $__DATABASE;
  mconnect();
  $QUERY = "SELECT * FROM ACCOUNTS
            WHERE ID=$_ID";
  $RESULT = mysql_db_query($__DATABASE, $QUERY);
  while($R = mysql_fetch_array($RESULT)){
    $RETURN = $R[$_CELL];
  }
  return($RETURN);
}

function GETUIDFromName($_NAME){
  global $__DATABASE;
  mconnect();
  $QUERY = "SELECT * FROM ACCOUNTS
            WHERE NAME = \"$_NAME\"";
  $RESULT = mysql_db_query($__DATABASE, $QUERY);
  if($R = mysql_fetch_array($RESULT)){
    $RETURN = $R['ID'];
  }
  return($RETURN);
}

?>
