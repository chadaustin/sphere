<?
$__DATABASE = "sphere";


function start(){
  include("/home/groups/s/sp/sphere/htdocs/doctype.inc.php"); 
  echo '
  <html>
    <head>
      <title>sphere.sourceforge.net</title>
      <link rel="stylesheet" type="text/css" href="http://sphere.sourceforge.net/malis-test/utility.css" />
    </head>
    <body>
      <table cellpadding="0" cellspacing="0" class="main">
        <tr>
          <td class="topbar">

            <table cellpadding="0" cellspacing="0" class="topmenu">
              <tr>
                <td class="menuitems">
                  <a class="headleft"  href="http://sphere.sourceforge.net/malis-test/index.php">news</a>
                  <a class="headleft"  href="http://sphere.sourceforge.net/malis-test/index.php?U_ACT=articles" style="text-decoration: line-through">articles</a>
                  <a class="headleft"  href="http://sphere.sourceforge.net/malis-test/dev.php?U_ACT=current">download</a>
                  <a class="headleft"  href="http://sphere.sourceforge.net/malis-test/dev.php?U_ACT=log" style="text-decoration: line-through">dev log</a>
                  <a class="headleft"  style="border-bottom: none" href="http://aegisknight.org/forum">forums *</a>
                </td>

                <td class="menulogo">
                  <img src="http://sphere.sourceforge.net/images/title.png" alt="sphere.sourceforge.net logo" />
                  <form id="login" method="post" action="http://sphere.sourceforge.net/malis-test/account.php?U_ACT=login">
                    <div>
                      <input type="text" name="login_name" size="8" />
                      <a href="javascript:document.getElementById(\'login\').submit()"> login</a> or <a href="http://sphere.sourceforge.net/malis-test/account.php?U_ACT=register">register</a> (<a href="http://sphere.sourceforge.net/malis-test/account.php?U_ACT=forgot">forgot password?</a>)
                    </div>
                  </form>
                </td>
                
                <td class="menuitems">
                  <a class="headright" href="http://sphere.sourceforge.net/malis-test/projects.php?U_ACT=list" style="text-decoration: line-through">projects</a>
                  <a class="headright" href="http://sphere.sourceforge.net/malis-test/dev.php?U_ACT=about">about sphere</a>
                  <a class="headright" href="http://sphere.sourceforge.net/malis-test/links.php">links</a>
                  <a class="headright" href="http://sphere.sourceforge.net/malis-test/index.php?U_ACT=map">site map</a>
                  <a class="headright" style="border-bottom: none" href="http://sphere.sourceforge.net/malis-test/contact.php">contact</a>
                </td>
              </tr>
            </table>

          </td>
        </tr>
      <tr>
        <td class="body">
          currently under development. line-through means the page is not available yet. (* the forums are the old forums on <a href="http://aegisknight.org">aegisknight.org</a> until the forum code for sphere.sf.net is completed.)
          <table class="bodybody">
            <tr>
              <td class="bodybody">';
}


function conclude(){
  echo '
              </td>
              <td class="sidebar">
              
                <table class="sidebaritem">
                  <tr>
                    <td class="sidebaritemhead">
                      news navigation
                    </td>
                  </tr>
                  <tr>
                    <td class="sidebaritembody">
                      20.10.01 - Final Example<br />
                      19.10.01 - More examples<br />
                      18.10.01 - Example News<br />
                      17.10.01 - Examples<br />
                      16.10.01 - Example<br />
                      <br />
                      <div style="text-align: right; width: 100%">older ...</div>
                    </td>
                  </tr>
                </table>
                
                <table class="sidebaritem">
                  <tr>
                    <td class="sidebaritemhead">
                      latest additions
                    </td>
                  </tr>
                  <tr>
                    <td class="sidebaritembody">
                      20.10.01 - Games - Some dumb game<br />
                      19.10.01 - Articles - Some dumb article<br />
                      <br />
                      <div style="text-align: right; width: 100%">complete ...</div>
                    </td>
                  </tr>
                </table>
                
                <table class="sidebaritem">
                  <tr>
                    <td class="sidebaritemhead">
                      my account
                    </td>
                  </tr>
                  <tr>
                    <td class="sidebaritembody">
                      currently, since i don\'t have access to php and don\'t really wanna waste my time making an independent file-driven database myself, i don\'t have a user system. but, when it is implemented, if you\'re logged in, here you\'ll be able to edit all sorts of things like which panels on the side you want, possibly what theme you want the site in, if you want to set up a project and what projects you are part of etc. more on this to come hopefully.
                    </td>
                  </tr>
                </table>
                
              </td>
            </tr>
          </table>
          </td>
        </tr>
      </table>
    </body>
  </html>';
}

function gen_item( $_HEAD, $_BODY ){
  echo "
    <table class=\"item\">
      <tr>
        <td class=\"itemhead\">
          $_HEAD
        </td>
      </tr>
      <tr>
        <td class=\"itembody\">
          $_BODY
          <br />
        </td>
      </tr>
    </table>";
}

function news_item( $_ID, $_TOPIC, $_DATE, $_AID, $_ICON, $_BODY ){
  $AEMAIL = GetCellFromUID( $_AID, 'EMAIL' );
  $AUTHOR = GetCellFromUID( $_AID, 'NAME' );
  if( $_ICON == 1 ) $_ICON = "announcement";
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
      also, this site most likely has display problems in 4.0 browsers. Use a 6.0 browser and, preferrably, use a browser besides Microsoft Internet Explorer 6.0(-) for Windows because it does not allow for alpha blending in PNGs.</span><br />
      <a href="http://kmeleon.org">get k-meleon</a> | <a href="http://mozilla.org">get mozilla</a>
    </div>';
}

function mconnect(){ //What can I say? I'm lazy.
  mysql_connect("mysql.sourceforge.net", "sphere", "crothosu") or die("could not connect to the mysql database. contact malis@wolf-phantom.net");
}

function GetCellFromUID( $_ID, $_CELL ){
  global $__DATABASE;
  mconnect();
  $QUERY = "SELECT * FROM ACCOUNTS
            WHERE ID=$_ID";
  $RESULT = mysql_db_query( $__DATABASE, $QUERY );
  while( $R = mysql_fetch_array( $RESULT ) ){
    $RETURN = $R[$_CELL];
  }
  return( $RETURN );
}

function GETUIDFromName( $_NAME ){
  global $__DATABASE;
  mconnect();
  $QUERY = "SELECT * FROM ACCOUNTS
            WHERE NAME = \"$_NAME\"";
  $RESULT = mysql_db_query( $__DATABASE, $QUERY );
  if( $R = mysql_fetch_array( $RESULT ) ){
    $RETURN = $R['ID'];
  }
  return( $RETURN );
}

?>
